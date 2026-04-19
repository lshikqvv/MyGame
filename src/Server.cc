#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "connection/Socket.h"
#include "game/Deck.h"
#include "game/Hand.h"

using namespace std;
using namespace connection;
using namespace deck;
using namespace hand;

static const int PORT = 8080;
static const string GAME_POKER = "POKER";

namespace {
int create_listener(int port)
{
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listener, 4) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    return listener;
}

vector<int> parse_exchange_positions(const string& line)
{
    vector<int> positions;
    if (line.rfind("EXCHANGE", 0) != 0) return positions;

    string payload = line.substr(string("EXCHANGE").size());
    for (char& c : payload) {
        if (c == ',' || c == '|') c = ' ';
    }

    stringstream ss(payload);
    int n = 0;
    set<int> unique;
    while (ss >> n) {
        if (n == 0) {
            unique.clear();
            break;
        }
        if (n >= 1 && n <= 5) unique.insert(n);
    }
    positions.assign(unique.begin(), unique.end());
    return positions;
}

bool parse_rematch_yes(const string& line)
{
    return line == "REMATCH YES";
}

void send_hand(Socket& player, Hand& h)
{
    player.request("HAND " + h.toCodeString());
}

string result_code_for_player(int cmp, bool is_player1)
{
    if (cmp == 0) return "DRAW";
    if (is_player1) return (cmp > 0) ? "WIN" : "LOSE";
    return (cmp < 0) ? "WIN" : "LOSE";
}

void send_result_detail(Socket& target, Hand& self, Hand& opp, const string& result_code)
{
    target.request(
        "RESULT_DETAIL RESULT=" + result_code +
        " SELF_ROLE=" + self.categoryName() +
        " OPP_ROLE=" + opp.categoryName() +
        " SELF_KEY=" + self.tiebreakString() +
        " OPP_KEY=" + opp.tiebreakString()
    );
}

bool handshake_game_selection(Socket& player, int player_no)
{
    player.request("HELLO " + to_string(player_no));
    player.request("GAME_OPTIONS " + GAME_POKER);
    string choice = player.response();
    if (choice != "GAME " + GAME_POKER) {
        player.request("SESSION_END");
        return false;
    }
    return true;
}
}

int main()
{
    int listener = create_listener(PORT);
    cout << "Server started on port " << PORT << endl;
    cout << "Waiting for players..." << endl;

    sockaddr_in client_addr {};
    socklen_t client_len = sizeof(client_addr);

    Socket player1;
    Socket player2;
    player1.sockfd = accept(listener, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (player1.sockfd < 0) {
        perror("accept player1 failed");
        exit(EXIT_FAILURE);
    }
    cout << "Player 1 connected" << endl;

    player2.sockfd = accept(listener, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (player2.sockfd < 0) {
        perror("accept player2 failed");
        exit(EXIT_FAILURE);
    }
    cout << "Player 2 connected" << endl;

    if (!handshake_game_selection(player1, 1) || !handshake_game_selection(player2, 2)) {
        player1.finish();
        player2.finish();
        close(listener);
        return 0;
    }

    bool continue_session = true;
    while (continue_session) {
        Deck d;
        d.init();
        d.shuffle();

        Hand h1;
        Hand h2;
        h1.make(d);
        h2.make(d);
        h1.sort();
        h2.sort();

        send_hand(player1, h1);
        send_hand(player2, h2);

        player1.request("REQUEST_EXCHANGE");
        player2.request("INFO Opponent is exchanging cards.");
        h1.exchange(d, parse_exchange_positions(player1.response()));
        h1.sort();
        send_hand(player1, h1);

        player2.request("REQUEST_EXCHANGE");
        player1.request("INFO Opponent is exchanging cards.");
        h2.exchange(d, parse_exchange_positions(player2.response()));
        h2.sort();
        send_hand(player2, h2);

        int cmp = h1.compareTo(h2);
        string r1 = result_code_for_player(cmp, true);
        string r2 = result_code_for_player(cmp, false);

        player1.request("RESULT " + r1);
        player2.request("RESULT " + r2);
        send_result_detail(player1, h1, h2, r1);
        send_result_detail(player2, h2, h1, r2);

        player1.request("REMATCH_PROMPT");
        player2.request("REMATCH_PROMPT");
        bool p1_yes = parse_rematch_yes(player1.response());
        bool p2_yes = parse_rematch_yes(player2.response());
        if (p1_yes && p2_yes) {
            player1.request("ROUND_NEXT");
            player2.request("ROUND_NEXT");
            continue;
        }

        player1.request("SESSION_END");
        player2.request("SESSION_END");
        continue_session = false;
    }

    player1.finish();
    player2.finish();
    close(listener);
    return 0;
}
