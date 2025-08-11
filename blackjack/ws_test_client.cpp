// ws_test_client.cpp
// Client WebSocket (localhost) pour envoyer des actions au serveur Blackjack
// Dépendances: websocketpp (header-only) + standalone Asio (header-only)
// Build (Ubuntu / Raspberry Pi OS):
//   sudo apt install -y g++ libwebsocketpp-dev libasio-dev
//   g++ -std=c++17 -O2 ws_test_client.cpp -o ws_test_client -lpthread
// Usage:
//   ./ws_test_client --host localhost --port 8765 --path /
//   Commandes: hit | stand | double | pret | rejouer | help | quit

#define ASIO_STANDALONE
#include <asio.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <sstream>
#include <cctype>

using client = websocketpp::client<websocketpp::config::asio_client>;

static std::string trim_lower(std::string s) {
    size_t a = 0; while (a < s.size() && std::isspace((unsigned char)s[a])) ++a;
    size_t b = s.size(); while (b>a && std::isspace((unsigned char)s[b-1])) --b;
    s = s.substr(a, b-a);
    for (auto &c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

static const char* HELP_TXT =
"Commandes:\n"
"  hit / h       -> {\"action\":\"tirer_carte\"}\n"
"  stand / s     -> {\"action\":\"pret\"} (ou stand, selon serveur)\n"
"  double / d    -> {\"action\":\"double\"}\n"
"  pret / p      -> {\"action\":\"pret\"}\n"
"  rejouer / r   -> {\"action\":\"rejouer\"}\n"
"  help          -> cette aide\n"
"  quit / exit   -> fermer la connexion\n";

int main(int argc, char** argv) {
    std::string host = "localhost";
    std::string path = "/";
    int port = 8765;
    // Permettre de choisir le mot pour Stand côté serveur ("pret" par défaut)
    std::string stand_word = "pret"; // changez en "stand" si nécessaire

    for (int i=1; i<argc; ++i) {
        std::string a = argv[i];
        if (a == "--help" || a == "-h") {
            std::cout << "Usage: " << argv[0] << " [--host H] [--port P] [--path /ws] [--stand-word pret|stand]\n";
            std::cout << HELP_TXT;
            return 0;
        } else if (a == "--host" && i+1 < argc) { host = argv[++i]; }
        else if (a == "--port" && i+1 < argc) { port = std::atoi(argv[++i]); }
        else if (a == "--path" && i+1 < argc) { path = argv[++i]; }
        else if (a == "--stand-word" && i+1 < argc) { stand_word = argv[++i]; }
    }

    std::stringstream uri; uri << "ws://" << host << ":" << port << path;
    std::cout << "[Connex] " << uri.str() << "\n";

    client c;
    c.clear_access_channels(websocketpp::log::alevel::all);
    c.init_asio();

    std::atomic<bool> connected{false};
    std::atomic<bool> closed{false};
    websocketpp::connection_hdl hdl;
    std::mutex hmut;

    c.set_open_handler([&](websocketpp::connection_hdl h){
        {
            std::lock_guard<std::mutex> lk(hmut); hdl = h;
        }
        connected.store(true);
        std::cout << "[OK] Connecté" << std::endl;
        std::cout << HELP_TXT;
    });

    c.set_fail_handler([&](websocketpp::connection_hdl){
        std::cout << "[ERR] Echec de connexion" << std::endl;
        closed.store(true);
    });

    c.set_close_handler([&](websocketpp::connection_hdl){
        std::cout << "[Info] Fermé par le serveur" << std::endl;
        closed.store(true);
    });

    c.set_message_handler([&](websocketpp::connection_hdl, client::message_ptr msg){
        auto op = msg->get_opcode();
        if (op == websocketpp::frame::opcode::text) {
            std::cout << "<-- " << msg->get_payload() << std::endl;
        } else {
            std::cout << "<-- [binaire] " << msg->get_payload().size() << " octets" << std::endl;
        }
    });

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri.str(), ec);
    if (ec) {
        std::cerr << "[ERR] get_connection: " << ec.message() << "\n";
        return 1;
    }

    c.connect(con);

    // Thread entrée utilisateur
    std::thread tin([&]{
        std::string line;
        while (!closed.load()) {
            if (!std::getline(std::cin, line)) break;
            std::string cmd = trim_lower(line);
            if (cmd == "" ) continue;
            if (cmd == "quit" || cmd == "exit") {
                std::lock_guard<std::mutex> lk(hmut);
                if (connected.load()) c.close(hdl, websocketpp::close::status::normal, "bye");
                break;
            }
            std::string json;
            if (cmd == "h" || cmd == "hit") json = "{\"action\":\"tirer_carte\"}";
            else if (cmd == "s" || cmd == "stand") json = std::string("{\"action\":\"") + stand_word + "\"}";
            else if (cmd == "d" || cmd == "double") json = "{\"action\":\"double\"}";
            else if (cmd == "p" || cmd == "pret") json = "{\"action\":\"pret\"}";
            else if (cmd == "r" || cmd == "rejouer") json = "{\"action\":\"rejouer\"}";
            else if (cmd == "help") { std::cout << HELP_TXT; continue; }
            else { std::cout << "[!] Commande inconnue. Tapez 'help'.\n"; continue; }

            if (!connected.load()) { std::cout << "[!] Pas connecté.\n"; continue; }
            websocketpp::lib::error_code se;
            {
                std::lock_guard<std::mutex> lk(hmut);
                c.send(hdl, json, websocketpp::frame::opcode::text, se);
            }
            if (se) { std::cerr << "[ERR] send: " << se.message() << "\n"; }
            else { std::cout << "--> " << json << "\n"; }
        }
    });

    c.run(); // boucle Asio

    closed.store(true);
    if (tin.joinable()) tin.join();
    return 0;
}
