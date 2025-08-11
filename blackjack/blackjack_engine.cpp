// blackjack_engine.cpp
// Moteur Blackjack minimal pour ≤4 joueurs, sans IHM.
// - Règles configurables : nb de paquets, nb de manches avant reshuffle, seuil du croupier,
//   hit sur soft 17, ratio de paiement du Blackjack.
// - Boucle de jeu découplée de l'I/O : fournissez une fonction de décision par joueur.
// - Prêt pour intégrer une UI (console, GUI, réseau) sans changer la logique.
// C++17

#include <algorithm>
#include <array>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// ================================ Cartes & Main ================================

enum class Suit { Clubs, Diamonds, Hearts, Spades };

enum class Rank {
    Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten,
    Jack = 11, Queen = 12, King = 13, Ace = 14
};

struct Card {
    Rank rank;
    Suit suit;

    static int faceValueNonAce(Rank r) {
        int v = static_cast<int>(r);
        if (r == Rank::Jack || r == Rank::Queen || r == Rank::King) return 10;
        if (r == Rank::Ace) return 1; // traité à part pour soft/hard
        return v; // 2..10
    }

    std::string toString() const {
        static const char* suitCh[] = {"C","D","H","S"};
        std::string r;
        switch (rank) {
            case Rank::Ace:   r = "A"; break;
            case Rank::King:  r = "K"; break;
            case Rank::Queen: r = "Q"; break;
            case Rank::Jack:  r = "J"; break;
            case Rank::Ten:   r = "10"; break;
            case Rank::Nine:  r = "9"; break;
            case Rank::Eight: r = "8"; break;
            case Rank::Seven: r = "7"; break;
            case Rank::Six:   r = "6"; break;
            case Rank::Five:  r = "5"; break;
            case Rank::Four:  r = "4"; break;
            case Rank::Three: r = "3"; break;
            case Rank::Two:   r = "2"; break;
        }
        return r + suitCh[static_cast<int>(suit)];
    }
};

struct Hand {
    std::vector<Card> cards;

    void add(const Card& c) { cards.push_back(c); }

    // Calcule la meilleure valeur (<=21 si possible). Retourne {total, soft?}
    std::pair<int,bool> bestTotal() const {
        int total = 0; int aces = 0;
        for (auto &c : cards) {
            if (c.rank == Rank::Ace) aces++;
            else total += Card::faceValueNonAce(c.rank);
        }
        // Ajouter les As : 1 chacun, puis +10 si ça tient (soft)
        total += aces; // tous à 1
        bool soft = false;
        if (aces > 0 && total + 10 <= 21) { total += 10; soft = true; }
        return {total, soft};
    }

    int total() const { return bestTotal().first; }
    bool isSoft() const { return bestTotal().second; }
    bool isBlackjack() const { return cards.size() == 2 && total() == 21; }
    bool isBust() const { return total() > 21; }
};

// ================================ Shoe / Sabot ================================

class Shoe {
public:
    explicit Shoe(int numDecks = 6, uint64_t seed = std::random_device{}())
        : m_numDecks(numDecks), m_rng(seed) {
        refillAndShuffle();
    }

    Card draw() {
        if (m_index >= m_cards.size()) refillAndShuffle();
        return m_cards[m_index++];
    }

    void refillAndShuffle() {
        m_cards.clear(); m_cards.reserve(52 * m_numDecks);
        for (int d = 0; d < m_numDecks; ++d) {
            for (int s = 0; s < 4; ++s) {
                for (int r = 2; r <= 14; ++r) {
                    if (r == 11 || r == 12 || r == 13 || r == 14 || (r >=2 && r <=10)) {
                        m_cards.push_back(Card{static_cast<Rank>(r), static_cast<Suit>(s)});
                    }
                }
            }
        }
        std::shuffle(m_cards.begin(), m_cards.end(), m_rng);
        m_index = 0;
    }

    size_t remaining() const { return m_cards.size() - m_index; }

    void setNumDecks(int n) { m_numDecks = std::max(1, n); refillAndShuffle(); }
private:
    int m_numDecks;
    std::vector<Card> m_cards;
    size_t m_index = 0;
    std::mt19937_64 m_rng;
};

// ================================ Règles & Actions ================================

struct GameConfig {
    int numDecks = 6;                // paquets dans le sabot
    int roundsBeforeShuffle = 6;     // nb de manches avant reshuffle (pénétration simple)
    int dealerHitThreshold = 16;     // le croupier pioche tant que total <= threshold
    bool dealerHitsSoft17 = false;   // si true, le croupier pioche aussi sur soft 17
    double blackjackPayout = 1.5;    // 3:2 = 1.5 ; 6:5 = 1.2
};

enum class PlayerAction { Hit, Stand, DoubleDown };

struct DecisionContext {
    const Hand& hand;
    Card dealerUpcard;
    int playerIndex;
    int roundNumber;
};

using DecisionFn = std::function<PlayerAction(const DecisionContext&)>;

// ================================ Joueurs & Résultats ================================

struct Outcome {
    double deltaChips = 0.0; // gain/perte sur la manche
    bool blackjack = false;
    bool bust = false;
    int total = 0;
};

struct PlayerRoundResult {
    Outcome outcome;
    Hand hand;
    double bet = 0.0; // prend en compte DoubleDown (x2)
};

struct RoundResult {
    std::vector<PlayerRoundResult> players;
    Hand dealer;
    bool dealerBlackjack = false;
    bool shuffledThisRound = false;
};

struct Player {
    std::string name;
    DecisionFn decide;
    double stack = 1000.0;
    double baseBet = 10.0;
    bool active = true; // permet de "désactiver" un siège
};

// ================================ Moteur Blackjack ================================

class BlackjackEngine {
public:
    explicit BlackjackEngine(GameConfig cfg, uint64_t seed = std::random_device{}())
        : m_cfg(cfg), m_shoe(cfg.numDecks, seed), m_rng(seed) {}

    bool addPlayer(const std::string& name, DecisionFn fn, double stack = 1000.0, double baseBet = 10.0) {
        if (m_players.size() >= 4) return false; // max 4 joueurs
        m_players.push_back(Player{name, std::move(fn), stack, baseBet, true});
        return true;
    }

    const std::vector<Player>& players() const { return m_players; }

    RoundResult playOneRound() {
        RoundResult rr; rr.shuffledThisRound = maybeShuffle();
        ++m_round;
        // Distribuer
        std::vector<Hand> hands(m_players.size());
        Hand dealer;
        // Initial bets
        std::vector<double> bet(m_players.size(), 0.0);

        for (int i = 0; i < 2; ++i) {
            for (size_t p = 0; p < m_players.size(); ++p) if (m_players[p].active) {
                hands[p].add(m_shoe.draw());
            }
            dealer.add(m_shoe.draw());
        }

        Card dealerUp = dealer.cards.front();
        bool dealerBJ = dealer.isBlackjack();
        rr.dealerBlackjack = dealerBJ;

        // Décisions joueurs
        for (size_t p = 0; p < m_players.size(); ++p) {
            PlayerRoundResult prr; prr.hand = hands[p];
            if (!m_players[p].active) { rr.players.push_back(std::move(prr)); continue; }

            double b = m_players[p].baseBet;
            bet[p] = b; // peut doubler plus tard

            bool firstDecision = true;
            if (!dealerBJ) {
                // Blackjack joueur naturel
                if (prr.hand.isBlackjack()) {
                    prr.outcome.blackjack = true;
                } else {
                    // Boucle d'action
                    while (true) {
                        DecisionContext ctx{prr.hand, dealerUp, static_cast<int>(p), m_round};
                        PlayerAction a = m_players[p].decide ? m_players[p].decide(ctx) : PlayerAction::Stand;
                        if (a == PlayerAction::Stand) break;
                        if (a == PlayerAction::DoubleDown && firstDecision) {
                            bet[p] += b; // double la mise (total = 2*b)
                            prr.hand.add(m_shoe.draw());
                            break; // puis stand d'office
                        }
                        // Sinon HIT
                        prr.hand.add(m_shoe.draw());
                        if (prr.hand.isBust()) break;
                        firstDecision = false; // après un hit, doubleDown n'est plus autorisé
                        firstDecision = false; // (doublons volontairement explicite)
                    }
                }
            }
            rr.players.push_back(std::move(prr));
        }

        // Jeu du croupier
        if (!dealerBJ) {
            auto needHit = [&](const Hand& h) {
                auto [tot, soft] = h.bestTotal();
                if (tot < m_cfg.dealerHitThreshold) return true;        // < 16 typiquement
                if (tot == m_cfg.dealerHitThreshold) return true;       // <= threshold
                if (tot == 17 && m_cfg.dealerHitsSoft17 && soft) return true; // H17
                return false;
            };
            while (needHit(dealer)) dealer.add(m_shoe.draw());
        }

        rr.dealer = dealer;

        // Résolution
        for (size_t p = 0; p < m_players.size(); ++p) {
            if (!m_players[p].active) continue;
            auto &prr = rr.players[p];
            double b = bet[p]; if (b <= 0.0) b = m_players[p].baseBet;
            double delta = 0.0;

            if (dealerBJ) {
                if (prr.hand.isBlackjack()) delta = 0.0; // push
                else delta = -b;
            } else if (prr.outcome.blackjack) {
                delta = m_cfg.blackjackPayout * b; // ex: 1.5 * b
            } else if (prr.hand.isBust()) {
                prr.outcome.bust = true; delta = -b;
            } else if (dealer.isBust()) {
                delta = +b;
            } else {
                int pt = prr.hand.total();
                int dt = dealer.total();
                if (pt > dt) delta = +b; else if (pt < dt) delta = -b; else delta = 0.0;
            }

            prr.bet = b;
            prr.outcome.total = prr.hand.total();
            prr.outcome.deltaChips = delta;
            m_players[p].stack += delta;
        }

        return rr;
    }

    const GameConfig& config() const { return m_cfg; }
    void setConfig(const GameConfig& cfg) {
        m_cfg = cfg; m_shoe.setNumDecks(cfg.numDecks); m_round = 0; m_sinceShuffle = 0;
    }

private:
    bool maybeShuffle() {
        bool doShuffle = false;
        if (m_sinceShuffle >= m_cfg.roundsBeforeShuffle) doShuffle = true;
        // Optionnel : si peu de cartes restantes, reshuffle aussi
        size_t minNeeded = (m_players.size() + 1) * 8; // marge simple
        if (m_shoe.remaining() < minNeeded) doShuffle = true;
        if (doShuffle) { m_shoe.refillAndShuffle(); m_sinceShuffle = 0; }
        else { ++m_sinceShuffle; }
        return doShuffle;
    }

    GameConfig m_cfg;
    Shoe m_shoe;
    std::mt19937_64 m_rng;
    std::vector<Player> m_players;
    int m_round = 0;
    int m_sinceShuffle = 0;
};

// ================================ Stratégies exemples (IA de placeholder) ================================

// Basique :
// - Double sur 11 contre 2..10
// - Sinon Hit < 12
// - 12..16 : Stand si le croupier montre 2..6, sinon Hit
// - 17+ : Stand
static PlayerAction basicStrategy(const DecisionContext& ctx) {
    auto [tot, soft] = ctx.hand.bestTotal();
    int up = Card::faceValueNonAce(ctx.dealerUpcard.rank);

    if (ctx.hand.cards.size() == 2 && !soft && tot == 11 && up >= 2 && up <= 10) {
        return PlayerAction::DoubleDown;
    }
    if (tot <= 11) return PlayerAction::Hit;
    if (tot >= 17) return PlayerAction::Stand;
    // 12..16
    if (up >= 2 && up <= 6) return PlayerAction::Stand; // dealer weak
    return PlayerAction::Hit;
}

// Toujours stand à 17+ sinon hit (très simple)
static PlayerAction simple17(const DecisionContext& ctx) {
    int t = ctx.hand.total();
    return (t >= 17) ? PlayerAction::Stand : PlayerAction::Hit;
}

// ================================ Démo minimale (aucune IHM) ================================

#ifdef BLACKJACK_DEMO
int main() {
    GameConfig cfg;
    cfg.numDecks = 6;
    cfg.roundsBeforeShuffle = 8;
    cfg.dealerHitThreshold = 16;    // croupier tire jusqu'à 16 inclus
    cfg.dealerHitsSoft17 = false;   // S17 (ne tire pas sur soft 17)
    cfg.blackjackPayout = 1.5;      // 3:2

    BlackjackEngine engine(cfg, /*seed*/ 42);

    engine.addPlayer("Alice", basicStrategy, 500.0, 10.0);
    engine.addPlayer("Bob", simple17, 500.0, 10.0);

    // Joue 1 manche et affiche un résumé minimal (pour vérif)
    RoundResult rr = engine.playOneRound();

    std::cout << (rr.shuffledThisRound ? "[SHUFFLE]\n" : "") ;
    std::cout << "Dealer: ";
    for (auto &c : rr.dealer.cards) std::cout << c.toString() << ' ';
    std::cout << "=> " << rr.dealer.total() << (rr.dealer.isBust()?" (BUST)":"")
              << (rr.dealerBlackjack?" (BJ)":"") << "\n";

    for (size_t i = 0; i < rr.players.size(); ++i) {
        std::cout << "P"<<i<<": ";
        for (auto &c : rr.players[i].hand.cards) std::cout << c.toString() << ' ';
        std::cout << "=> " << rr.players[i].hand.total();
        if (rr.players[i].outcome.blackjack) std::cout << " (BJ)";
        if (rr.players[i].hand.isBust()) std::cout << " (BUST)";
        std::cout << ", bet=" << rr.players[i].bet
                  << ", delta=" << rr.players[i].outcome.deltaChips
                  << "\n";
    }
    return 0;
}
#endif

/*
Compilation (Linux/Mac/WSL) :
    g++ -std=c++17 -O2 blackjack_engine.cpp -o blackjack

Démo (activer le main ci-dessus) :
    g++ -std=c++17 -O2 -DBLACKJACK_DEMO blackjack_engine.cpp -o blackjack_demo
    ./blackjack_demo

Intégration UI :
    - Conservez BlackjackEngine, Player, DecisionFn, RoundResult.
    - Dans votre UI, instanciez l'engine avec GameConfig.
    - Ajoutez ≤4 joueurs avec une DecisionFn qui lit les actions GUI.
    - Appelez playOneRound(); puis utilisez RoundResult pour peindre l'état.

Extensions faciles :
    - Split / plusieurs mains : transformer PlayerRoundResult en vector<HandResult>.
    - Surrenders, insurance : ajouter des PlayerAction et la logique de payout.
    - Cut-card / pénétration par % au lieu de roundsBeforeShuffle.
*/
