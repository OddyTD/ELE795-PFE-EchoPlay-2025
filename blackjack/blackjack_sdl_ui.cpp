// blackjack_sdl_ui.cpp
// UI SDL2 + SDL_ttf pour le moteur Blackjack (gestes tactiles)
// - Fonctionne sur Raspberry Pi OS et Ubuntu.
// - Gestes :
//      * Double tap  -> HIT
//      * Swipe (←/→) -> STAND
//      * Long press  -> DOUBLE DOWN (seulement en 1ère décision d'une main à 2 cartes)
// - Fallback souris/clavier (PC) :
//      * Clic double rapide = Hit, glisser horizontal = Stand, maintenir clic >600ms = Double
//      * Touches: H=Hit, S=Stand, D=Double, Q/Esc=Quitter
//
// Par simplicité, ce fichier inclut le moteur pour ne pas gérer d'en-tête séparé.
// Placez "blackjack_engine.cpp" à côté et laissez l'inclusion ci-dessous active.
// N'compilez PAS blackjack_engine.cpp séparément si vous laissez l'#include actif.
// Si vous préférez une compilation séparée, commentez l'#include ci-dessous et
// compilez: g++ ... blackjack_engine.cpp blackjack_sdl_ui.cpp ...

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <array>

// --- WebSocketpp + Asio ---
#define ASIO_STANDALONE
#include <asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <memory>

// Inclure le moteur
#include "blackjack_engine.cpp"

// --- Réglages UI  ---
// Durée d'affichage du résultat de fin de manche (en millisecondes)
static int UI_ROUND_PAUSE_MS = 5000;   // ↑ pour afficher plus longtemps le récap
// Géométrie des cartes et recouvrement
static int UI_CARD_W = 70;             // largeur carte
static int UI_CARD_H = 100;            // hauteur carte
static int UI_CARD_STEP = 32;          // pas horizontal entre cartes (↑ = moins de recouvrement)

// Vitesse de révélation des cartes du croupier (ms par carte pendant l'animation de fin)
static int UI_DEALER_REVEAL_STEP_MS = 600;
// État global pour l'animation de révélation
static uint64_t g_lastRoundSerialRendered = 0;
static uint64_t g_revealStartTicks = 0;

// Couleurs de table (vert feutre)
static SDL_Color UI_TABLE_COLOR      = { 11, 110, 59, 255 }; // vert casino
static SDL_Color UI_TABLE_ZONE_TOP   = {  0,   0,  0,  30 }; // ombrage léger
static SDL_Color UI_TABLE_ZONE_BOTTOM= {  0,   0,  0,  60 }; // ombrage plus marqué

// Port WebSocket
static unsigned short WS_PORT = 8765;

// Réglages des images de cartes
static const char* UI_CARDS_DIR = "./PlayingCards/51x71"; // changez vers 42x58, 71x71, 71x98 ou 128x178

// Atlas de 52 textures (card_0.jpg .. card_51.jpg)
static std::array<SDL_Texture*, 52> g_cardTex{};
static bool g_cardTexLoaded = false;

static int rankToIndex(Rank r) {
    // Ordre des fichiers: A,2,3,4,5,6,7,8,9,10,J,Q,K
    if (r == Rank::Ace) return 0;
    return static_cast<int>(r) - 1; // 2->1 ... 10->9, J->10, Q->11, K->12
}

static int suitToPackIndex(Suit s) {
    // Ordre des dossiers selon l'utilisateur: Trèfle (Clubs), Coeur (Hearts), Pique (Spades), Carreaux (Diamonds)
    switch (s) {
        case Suit::Clubs:    return 0;
        case Suit::Hearts:   return 1;
        case Suit::Spades:   return 2;
        case Suit::Diamonds: return 3;
    }
    return 0;
}

static int cardToImageIndex(const Card& c) {
    int ri = rankToIndex(c.rank); // 0..12
    int si = suitToPackIndex(c.suit); // 0..3
    return si * 13 + ri; // 0..51
}

static bool loadCardTextures(SDL_Renderer* r) {
    if (g_cardTexLoaded) return true;
    bool ok = true;
    for (int i = 0; i < 52; ++i) {
        std::string path = std::string(UI_CARDS_DIR) + "/card_" + std::to_string(i) + ".jpg";
        SDL_Surface* s = IMG_Load(path.c_str());
        if (!s) { std::cerr << "IMG_Load échoue: " << path << " => " << IMG_GetError() << "\n"; ok = false; continue; }
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        SDL_FreeSurface(s);
        if (!t) { std::cerr << "CreateTexture échoue pour: " << path << " => " << SDL_GetError() << "\n"; ok = false; continue; }
        g_cardTex[i] = t;
    }
    g_cardTexLoaded = ok; // même si partiel, on marque true pour éviter reload
    return ok;
}

static void unloadCardTextures() {
    for (auto &t : g_cardTex) { if (t) { SDL_DestroyTexture(t); t=nullptr; } }
    g_cardTexLoaded = false;
}


// ============================= Utilitaires rendu texte =============================

struct FontPack {
    TTF_Font* main = nullptr;
};

static TTF_Font* loadAnyFont(int px) {
    const char* candidates[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    };
    for (auto p : candidates) {
        TTF_Font* f = TTF_OpenFont(p, px);
        if (f) return f;
    }
    return nullptr;
}

static void drawText(SDL_Renderer* r, TTF_Font* f, const std::string& txt, int x, int y) {
    if (!f) return;
    SDL_Color col{255,255,255,255};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, txt.c_str(), col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst{ x, y, surf->w, surf->h };
    SDL_FreeSurface(surf);
    if (tex) { SDL_RenderCopy(r, tex, nullptr, &dst); SDL_DestroyTexture(tex); }
}

// Calcule la meilleure valeur blackjack (<=21 si possible) pour une main affichée
static std::pair<int,bool> uiBestTotal(const std::vector<Card>& cards) {
    int total = 0; int aces = 0;
    for (const auto& c : cards) {
        if (c.rank == Rank::Ace) aces++;
        else total += Card::faceValueNonAce(c.rank);
    }
    total += aces; // tous les As comptés 1
    bool soft = false;
    if (aces > 0 && total + 10 <= 21) { total += 10; soft = true; }
    return {total, soft};
}

// ============================= État partagé UI <-> Moteur =============================

struct DecisionContextCopy {
    std::vector<Card> handCards;
    Card dealerUp;
    int playerIndex = 0;
    int roundNumber = 0;
};

struct Bridge {
    std::mutex m;
    std::condition_variable cv;

    // Contexte en attente côté UI quand le moteur veut une décision
    std::optional<DecisionContextCopy> pendingCtx;

    // Action choisie par l'utilisateur (ou la souris/clavier)
    std::optional<PlayerAction> chosen;

    // Dernier résultat de manche complet pour affichage
    std::optional<RoundResult> lastRound;

    // Stats persistantes par joueur
    struct PlayerStats { int win=0, loss=0, push=0; };
    std::vector<PlayerStats> stats;

    // Compteur de manches terminées (pour déclencher l'anim de révélation)
    std::atomic<uint64_t> roundSerial{0};

    // Pour fermeture propre
    std::atomic<bool> quit{false};
};

// ============================= WebSocket Server =============================
#include <set>

class WsHub {
public:
    using server = websocketpp::server<websocketpp::config::asio>;

    WsHub() : running(false), bridge(nullptr) {}

    void start(unsigned short port, Bridge* b) {
        bridge = b;
        ws.clear_access_channels(websocketpp::log::alevel::all);
        ws.init_asio();
        ws.set_open_handler([this](websocketpp::connection_hdl hdl){
            std::lock_guard<std::mutex> lk(m);
            conns.insert(hdl);
        });
        ws.set_close_handler([this](websocketpp::connection_hdl hdl){
            std::lock_guard<std::mutex> lk(m);
            conns.erase(hdl);
        });
        ws.set_message_handler([this](websocketpp::connection_hdl hdl, server::message_ptr msg){
            handle_message(hdl, msg->get_payload());
        });
        ws.listen(WS_PORT);
        ws.start_accept();
        running.store(true);
        th = std::thread([this]{ ws.run(); });
    }

    void stop() {
        if (!running.exchange(false)) return;
        websocketpp::lib::error_code ec;
        ws.stop_listening();
        {
            std::lock_guard<std::mutex> lk(m);
            for (auto const& c : conns) {
                ws.close(c, websocketpp::close::status::normal, "shutdown", ec);
            }
            conns.clear();
        }
        ws.stop();
        if (th.joinable()) th.join();
    }

    void broadcast(const std::string& text) {
        std::lock_guard<std::mutex> lk(m);
        for (auto const& c : conns) {
            websocketpp::lib::error_code se;
            ws.send(c, text, websocketpp::frame::opcode::text, se);
        }
    }

private:

    void handle_message(websocketpp::connection_hdl, const std::string& payload) {
        auto act = parse_action(payload);
        if (act.empty() || !bridge) return;

        std::unique_lock<std::mutex> lk(bridge->m);
        if (!bridge->pendingCtx) return; // rien à décider

        if (act == "tirer_carte") {
            bridge->chosen = PlayerAction::Hit;
        } else if (act == "pret" || act == "stand") {
            bridge->chosen = PlayerAction::Stand;
        } else if (act == "double") {
            if (bridge->pendingCtx->handCards.size() == 2)
                bridge->chosen = PlayerAction::DoubleDown;
            else
                return; // ignore si non autorisé
        } else {
            return;
        }
        bridge->cv.notify_all();
    }

    static std::string parse_action(const std::string& s) {
        // 1) texte brut
        std::string t; t.reserve(s.size());
        for (char c : s) t.push_back((char)std::tolower((unsigned char)c));
        if (t == "tirer_carte" || t == "pret" || t == "stand" || t == "double" || t == "rejouer") return t;

        // 2) JSON minimal: "action":"xxx"
        auto p = t.find("\"action\"");
        if (p == std::string::npos) return {};
        p = t.find(':', p);
        if (p == std::string::npos) return {};
        p = t.find('"', p);
        if (p == std::string::npos) return {};
        auto q = t.find('"', p+1);
        if (q == std::string::npos) return {};
        return t.substr(p+1, q-(p+1));
    }

    server ws;
    std::thread th;
    std::mutex m;
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> conns;
    std::atomic<bool> running;
    
    public:
    Bridge* bridge; // après, car on a besoin du type complet pour accéder à ses membres
};

// Global
static std::unique_ptr<WsHub> g_ws;

// Helpers JSON pour diffuser des cartes (indices 0..51)
static std::string json_array_from_cards(const std::vector<Card>& cards) {
    std::string out = "[";
    for (size_t i=0;i<cards.size();++i) {
        if (i) out += ",";
        out += std::to_string(cardToImageIndex(cards[i]));
    }
    out += "]";
    return out;
}


static DecisionContextCopy copyCtx(const DecisionContext& ctx) {

    DecisionContextCopy c;
    c.playerIndex = ctx.playerIndex;
    c.roundNumber = ctx.roundNumber;
    c.dealerUp = ctx.dealerUpcard;
    c.handCards = ctx.hand.cards; // copie
    return c;
}

// DecisionFn bloquante : attend un geste depuis l'UI
static PlayerAction humanDecision(Bridge* bridge, const DecisionContext& ctx) {
    std::unique_lock<std::mutex> lk(bridge->m);
    bridge->pendingCtx = copyCtx(ctx);
    bridge->chosen.reset();
    bridge->cv.notify_all();

    // Broadcast main initiale au microcontrôleur
    if (g_ws) {
        auto c = copyCtx(ctx);
        std::string payload = std::string("{\"action\":\"main_initiale\",")
            + "\"round\":" + std::to_string(c.roundNumber) + ","
            + "\"dealer_up\":" + std::to_string(cardToImageIndex(c.dealerUp)) + ","
            + "\"cartes\":" + json_array_from_cards(c.handCards)
            + "}";
        g_ws->broadcast(payload);
}

    // Attendre l'utilisateur
    while (!bridge->quit.load()) {
        if (bridge->chosen.has_value()) {
            PlayerAction act = *bridge->chosen;
            bridge->pendingCtx.reset();
            bridge->chosen.reset();
            return act;
        }
        bridge->cv.wait_for(lk, std::chrono::milliseconds(50));
    }
    // Si on quitte, considérer Stand par défaut
    return PlayerAction::Stand;
}

// ============================= Détection de gestes =============================

struct TapTracker {
    bool down = false;
    float downX = 0, downY = 0;
    uint64_t downTime = 0; // ms

    // pour double tap
    uint64_t lastTapTime = 0;
    float lastTapX = 0, lastTapY = 0;
};

static float dist2(float x1,float y1,float x2,float y2){ float dx=x1-x2, dy=y1-y2; return dx*dx+dy*dy; }

struct GestureResult { enum Type { None, DoubleTap, Swipe, LongPress } type=None; float dx=0, dy=0; };

static GestureResult analyzeGesture(TapTracker& t, float upX, float upY, uint64_t upTime) {
    GestureResult gr; gr.type = GestureResult::None;
    const uint64_t dt = upTime - t.downTime;
    const float d2 = dist2(upX, upY, t.downX, t.downY);

    // Paramètres (px et ms)
    const float TAP_RADIUS2 = 30.0f*30.0f;
    const uint64_t TAP_MAX = 250;
    const uint64_t DOUBLE_TAP_MAX = 350;
    const float SWIPE_MIN = 80.0f; // pixels
    const uint64_t LONG_PRESS_MIN = 600;

    // Long press (immobile et assez long)
    if (dt >= LONG_PRESS_MIN && d2 < TAP_RADIUS2) {
        gr.type = GestureResult::LongPress; return gr;
    }

    // Swipe horizontal si déplacement suffisant et rapide
    float dx = upX - t.downX; float dy = upY - t.downY;
    if (std::abs(dx) >= SWIPE_MIN && std::abs(dx) > std::abs(dy)*1.2f && dt <= 600) {
        gr.type = GestureResult::Swipe; gr.dx = dx; gr.dy = dy; return gr;
    }

    // Tap
    if (dt <= TAP_MAX && d2 < TAP_RADIUS2) {
        // Double tap ?
        if (t.lastTapTime && (upTime - t.lastTapTime) <= DOUBLE_TAP_MAX &&
            dist2(upX, upY, t.lastTapX, t.lastTapY) < (TAP_RADIUS2*1.5f)) {
            gr.type = GestureResult::DoubleTap;
        }
        // update last tap
        t.lastTapTime = upTime; t.lastTapX = upX; t.lastTapY = upY;
    }
    return gr;
}

// ============================= Rendu simple =============================

static void drawCard(SDL_Renderer* r, TTF_Font* f, const Card& c,
                     int x, int y, int w = UI_CARD_W, int h = UI_CARD_H) {
    const int idx = cardToImageIndex(c);
    if (idx >= 0 && idx < 52 && g_cardTex[idx]) {
        SDL_Rect dst{ x, y, w, h };
        SDL_RenderCopy(r, g_cardTex[idx], nullptr, &dst);
        // Optionnel: cadre léger
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_RenderDrawRect(r, &dst);
        return;
    }
    // Fallback (si image manquante): rectangle + code carte
    SDL_Rect rect{ x, y, w, h };
    SDL_SetRenderDrawColor(r, 20, 120, 20, 255);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderDrawRect(r, &rect);
    if (f) {
        drawText(r, f, c.toString(), x + 8, y + 8);
    }
}

static void drawHandN(SDL_Renderer* r, TTF_Font* f, const std::vector<Card>& cards, int x, int y, int n) {
    int offset = UI_CARD_STEP; int w = UI_CARD_W; int h = UI_CARD_H; int i=0;
    n = std::max(0, std::min((int)cards.size(), n));
    for (int k=0; k<n; ++k) {
        const auto &c = cards[k];
        drawCard(r, f, c, x + i*offset, y, w, h); ++i;
    }
}

static void drawHand(SDL_Renderer* r, TTF_Font* f, const std::vector<Card>& cards, int x, int y) {
    drawHandN(r, f, cards, x, y, (int)cards.size());
}


static void fillRect(SDL_Renderer* r, int x,int y,int w,int h, SDL_Color col){
    SDL_Rect rc{ x,y,w,h }; SDL_SetRenderDrawColor(r, col.r,col.g,col.b,col.a); SDL_RenderFillRect(r, &rc);
}

// ============================= Thread moteur =============================

struct EngineThreadCfg {
    GameConfig cfg;
    int numHuman = 1; // 1 humain par défaut
    int numAI = 0;    // joueurs IA supplémentaires
    uint64_t seed = 42;
};

static void engineThread(Bridge* bridge, EngineThreadCfg etcfg) {
    BlackjackEngine engine(etcfg.cfg, etcfg.seed);

    // Ajout joueurs
    // Joueur 0 humain
    engine.addPlayer("Vous", [bridge](const DecisionContext& ctx){ return humanDecision(bridge, ctx); }, 500.0, 10.0);
    for (int i = 0; i < etcfg.numAI && (i+1) < 4; ++i) {
        engine.addPlayer(std::string("IA ")+std::to_string(i+1), basicStrategy, 500.0, 10.0);
        std::lock_guard<std::mutex> lk(bridge->m);
        bridge->stats.assign(engine.players().size(), {});
    }

    while (!bridge->quit.load()) {
        RoundResult rr = engine.playOneRound();
        {
            std::lock_guard<std::mutex> lk(bridge->m);
            bridge->lastRound = rr;

            if (bridge->stats.size() != rr.players.size())
                    bridge->stats.assign(rr.players.size(), {});

                for (size_t i = 0; i < rr.players.size(); ++i) {
                    double d = rr.players[i].outcome.deltaChips;
                    if (d > 0) bridge->stats[i].win++;
                    else if (d < 0) bridge->stats[i].loss++;
                    else bridge->stats[i].push++;
            }

            bridge->roundSerial++;

            // Broadcast fin de partie pour le joueur 0 (humain)
            if (g_ws && !rr.players.empty()) {
                const auto &pr = rr.players[0];
                int dealerTot = rr.dealer.total();
                int playerTot = pr.hand.total();
                std::string outcome = (pr.outcome.blackjack ? "Blackjack" :
                                    (pr.outcome.deltaChips > 0 ? "Victoire" :
                                        (pr.outcome.deltaChips < 0 ? "Defaite" : "Egalite")));
                std::string payload = std::string("{\"action\":\"fin_partie\",")
                    + "\"dealer_total\":" + std::to_string(dealerTot) + ","
                    + "\"player_total\":" + std::to_string(playerTot) + ","
                    + "\"delta\":" + std::to_string((int)std::round(pr.outcome.deltaChips)) + ","
                    + "\"resultat\":\"" + outcome + "\","
                    + "\"dealer_cartes\":" + json_array_from_cards(rr.dealer.cards) + ","
                    + "\"player_cartes\":" + json_array_from_cards(pr.hand.cards)
                    + "}";
                g_ws->broadcast(payload);
            }

        }
        bridge->cv.notify_all();
        // Petite pause d'1.2s pour laisser lire le résultat
        // Pause paramétrable pour laisser lire le résultat, tout en restant interruptible
        int remaining = UI_ROUND_PAUSE_MS;
        while (remaining > 0 && !bridge->quit.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            remaining -= 50;
        }
    }
}

// ============================= Programme principal =============================

int main(int argc, char** argv) {
    bool fullscreen = false;
    
    int aiPlayers = 0; // 0..3

    for (int i=1;i<argc;++i){
        std::string a = argv[i];
        if (a == "--fullscreen") fullscreen = true;
        else if (a.rfind("--ai=",0)==0) aiPlayers = std::max(0,std::min(3, std::atoi(a.c_str()+5)));
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << "\n"; return 1;
    }

    if (TTF_Init() != 0) { std::cerr << "TTF_Init error: " << TTF_GetError() << "\n"; }

    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG; // JPG pour vos cartes

    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        std::cerr << "IMG_Init error: " << IMG_GetError() << "\n";
    }

    SDL_Window* win = SDL_CreateWindow(
        "Blackjack Touch",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 600,
        SDL_WINDOW_SHOWN | (fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
    );
    if (!win) { std::cerr << "CreateWindow error: " << SDL_GetError() << "\n"; return 1; }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) { ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE); }
    if (!ren) { std::cerr << "CreateRenderer error: " << SDL_GetError() << "\n"; return 1; }
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    FontPack fonts; fonts.main = loadAnyFont(22);

    // Charger les images de cartes
    if (!loadCardTextures(ren)) {
        std::cerr << "Cartes: certaines textures n'ont pas pu être chargées depuis " << UI_CARDS_DIR << "\n";
    }

    // Config blackjack
    EngineThreadCfg etcfg; // valeurs par défaut
    etcfg.numAI = aiPlayers; // seats IA
    etcfg.cfg.numDecks = 4;            // param modifiable
    etcfg.cfg.roundsBeforeShuffle = 8; // param modifiable
    etcfg.cfg.dealerHitThreshold = 16; // ≤16 tire
    etcfg.cfg.dealerHitsSoft17 = false;
    etcfg.cfg.blackjackPayout = 1.5;

    Bridge bridge;

    g_ws = std::make_unique<WsHub>();
    g_ws->start(WS_PORT, &bridge);

    std::thread th(engineThread, &bridge, etcfg);

    TapTracker tap;

    auto shouldAllowDouble = [&](const std::optional<DecisionContextCopy>& pctx){
        if (!pctx) return false; return pctx->handCards.size() == 2; };

    bool running = true;
    while (running && !bridge.quit.load()) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { running = false; bridge.quit = true; }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q) { running = false; bridge.quit = true; }
                else if (e.key.keysym.sym == SDLK_h) { std::lock_guard<std::mutex> lk(bridge.m); bridge.chosen = PlayerAction::Hit; bridge.cv.notify_all(); }
                else if (e.key.keysym.sym == SDLK_s) { std::lock_guard<std::mutex> lk(bridge.m); bridge.chosen = PlayerAction::Stand; bridge.cv.notify_all(); }
                else if (e.key.keysym.sym == SDLK_d) { std::lock_guard<std::mutex> lk(bridge.m); bridge.chosen = PlayerAction::DoubleDown; bridge.cv.notify_all(); }
            }
            else if (e.type == SDL_FINGERDOWN) {
                tap.down = true; tap.downX = e.tfinger.x * 1024.0f; tap.downY = e.tfinger.y * 600.0f; tap.downTime = SDL_GetTicks64();
            }
            else if (e.type == SDL_FINGERUP) {
                float upX = e.tfinger.x * 1024.0f; float upY = e.tfinger.y * 600.0f;
                uint64_t upT = SDL_GetTicks64();
                GestureResult gr = analyzeGesture(tap, upX, upY, upT);
                if (gr.type == GestureResult::DoubleTap) { std::lock_guard<std::mutex> lk(bridge.m); bridge.chosen = PlayerAction::Hit; bridge.cv.notify_all(); }
                else if (gr.type == GestureResult::Swipe) { std::lock_guard<std::mutex> lk(bridge.m); bridge.chosen = PlayerAction::Stand; bridge.cv.notify_all(); }
                else if (gr.type == GestureResult::LongPress) { if (shouldAllowDouble(bridge.pendingCtx)) { std::lock_guard<std::mutex> lk(bridge.m); bridge.chosen = PlayerAction::DoubleDown; bridge.cv.notify_all(); } }
                tap.down = false;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                tap.down = true; tap.downX = (float)e.button.x; tap.downY = (float)e.button.y; tap.downTime = SDL_GetTicks64();
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                float upX = (float)e.button.x, upY = (float)e.button.y; uint64_t upT = SDL_GetTicks64();
                GestureResult gr = analyzeGesture(tap, upX, upY, upT);
                if (gr.type == GestureResult::DoubleTap) { std::lock_guard<std::mutex> lk(bridge.m); bridge.chosen = PlayerAction::Hit; bridge.cv.notify_all(); }
                else if (gr.type == GestureResult::Swipe) { std::lock_guard<std::mutex> lk(bridge.m); bridge.chosen = PlayerAction::Stand; bridge.cv.notify_all(); }
                else if (gr.type == GestureResult::LongPress) { if (shouldAllowDouble(bridge.pendingCtx)) { std::lock_guard<std::mutex> lk(bridge.m); bridge.chosen = PlayerAction::DoubleDown; bridge.cv.notify_all(); } }
                tap.down = false;
            }
        }

        // Rendu
        int W=0,H=0; SDL_GetRendererOutputSize(ren,&W,&H); if (W==0) {W=1024;H=600;}
        SDL_SetRenderDrawColor(ren, UI_TABLE_COLOR.r, UI_TABLE_COLOR.g, UI_TABLE_COLOR.b, UI_TABLE_COLOR.a); SDL_RenderClear(ren);

        // Zones
        SDL_Color dark{0,0,0,100};
        fillRect(ren, 0, 0, W, H/2, UI_TABLE_ZONE_TOP); // zone croupier
        fillRect(ren, 0, H/2, W, H/2, UI_TABLE_ZONE_BOTTOM); // zone joueur

        // Afficher contexte courant si on attend une décision
        {
            std::lock_guard<std::mutex> lk(bridge.m);
            if (bridge.pendingCtx) {
                auto& c = *bridge.pendingCtx;
                drawText(ren, fonts.main, "Votre tour — Round " + std::to_string(c.roundNumber), 20, H/2 + 10);
                drawText(ren, fonts.main, "Croupier montre: " + c.dealerUp.toString(), 20, 20);
                // main joueur
                drawHand(ren, fonts.main, c.handCards, 20, H/2 + 50);
                // Afficher le total joueur (soft/hard) au moment de la décision
                {
                    auto bt = uiBestTotal(c.handCards);
                    std::string tot = "Total: " + std::to_string(bt.first) + (bt.second ? " (soft)" : " (hard)");
                    drawText(ren, fonts.main, tot, 20, H/2 + 50 + UI_CARD_H + 8);
                }
                if (!bridge.stats.empty()) {
                    const auto &st = bridge.stats[0];
                    std::string stat = "W:" + std::to_string(st.win)
                                    + " L:" + std::to_string(st.loss)
                                    + " P:" + std::to_string(st.push);
                    drawText(ren, fonts.main, stat, 20, H/2 + 50 + UI_CARD_H + 8 + 24);
                }
                drawText(ren, fonts.main, "Gestes: Double tap = HIT, Swipe = STAND, Long press = DOUBLE", 20, H - 40);
                if (!shouldAllowDouble(bridge.pendingCtx)) drawText(ren, fonts.main, "(Double down indisponible)", 20, H - 20);
            } else if (bridge.lastRound) {
                // Affiche un bref résumé de la dernière manche
                const auto& rr = *bridge.lastRound;
                int y = 20;
                drawText(ren, fonts.main, std::string("[Résultat] Dealer ") + (rr.dealerBlackjack?"(BJ)":""), 20, y); y+=10;
                // Révéler progressivement la main du croupier et afficher son total courant
                uint64_t currentSerial = bridge.roundSerial.load();
                if (currentSerial != g_lastRoundSerialRendered) {
                    g_revealStartTicks = SDL_GetTicks64();
                    g_lastRoundSerialRendered = currentSerial;
                }
                uint64_t elapsed = SDL_GetTicks64() - g_revealStartTicks;
                int dCount = (int)rr.dealer.cards.size();
                int showCount = std::max(1, std::min(dCount, (int)(1 + elapsed / UI_DEALER_REVEAL_STEP_MS)));

                drawHandN(ren, fonts.main, rr.dealer.cards, 20, 40, showCount);
                {
                    std::vector<Card> dsub(rr.dealer.cards.begin(), rr.dealer.cards.begin() + showCount);
                    auto dTot = uiBestTotal(dsub);
                    std::string dlabel = std::string("Total croupier: ") + std::to_string(dTot.first) + (dTot.second ? " (soft)" : " (hard)");
                    drawText(ren, fonts.main, dlabel, 20, 40 + UI_CARD_H + 8);
                }

                int py = H/2 + 10;
                for (size_t i=0;i<rr.players.size();++i){
                    const auto& pr = rr.players[i];
                    std::string line = std::string("P")+std::to_string((int)i)+": ";
                    line += (pr.outcome.blackjack?"BJ ":"");
                    line += (pr.hand.isBust()?"BUST ":"");
                    line += "Δ=" + std::to_string((int)std::round(pr.outcome.deltaChips));
                    if (i < bridge.stats.size()) {
                        const auto &st = bridge.stats[i];
                        line += std::string("  W:") + std::to_string(st.win)
                            +  " L:" + std::to_string(st.loss)
                            +  " P:" + std::to_string(st.push);
                    }
                    drawText(ren, fonts.main, line, 20, py);
                    drawHand(ren, fonts.main, pr.hand.cards, 20, py+20);
                    py += 140;
                    if (py > H-140) break;
                }
            } else {
                drawText(ren, fonts.main, "En attente du moteur...", 20, H/2);
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    bridge.quit = true; bridge.cv.notify_all();
    if (th.joinable()) th.join();

    if (fonts.main) TTF_CloseFont(fonts.main);
    unloadCardTextures();
    SDL_DestroyRenderer(ren); SDL_DestroyWindow(win);
    IMG_Quit(); TTF_Quit(); SDL_Quit();
    if (g_ws) { g_ws->stop(); g_ws.reset(); }
    return 0;
}

/*
BUILD — Ubuntu:
    sudo apt update
    sudo apt install g++ libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev fonts-dejavu-core
    g++ -std=c++17 -O2 blackjack_sdl_ui.cpp -o blackjack_touch -lSDL2 -lSDL2_ttf -lSDL2_image -pthread

BUILD — Raspberry Pi Zero 2W (Raspberry Pi OS Bookworm/Bullseye):
    sudo apt update
    sudo apt install g++ libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev fonts-dejavu-core
    g++ -std=c++17 -O2 blackjack_sdl_ui.cpp -o blackjack_touch -lSDL2 -lSDL2_ttf -lSDL2_image -pthread

Exécution en plein écran (Pi tactile):
    ./blackjack_touch --fullscreen

Joueurs IA supplémentaires (pour tester le tour de plusieurs joueurs):
    ./blackjack_touch --ai=2

Notes:
- Gesture mapping :
    * Double tap (ou double clic) = HIT
    * Swipe horizontal (gauche/droite) = STAND
    * Long press (>600ms, peu de mouvement) = DOUBLE (seulement main à 2 cartes)
- Cette UI fait tourner le moteur dans un thread séparé et bloque les décisions via un pont (mutex+cv),
  ce qui laisse l'event loop SDL fluide.
- Pour une UI plus riche (sprites de cartes, animations), conservez ce schéma de "Bridge" et remplacez le rendu.
*/
