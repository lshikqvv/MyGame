#include "connection/Socket.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace connection;

static const int PORT = 8080;

namespace {
vector<string> split(const string& s, char delim)
{
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        if (!item.empty()) elems.push_back(item);
    }
    return elems;
}

void show_hand(const string& payload)
{
    vector<string> cards = split(payload, '|');
    cout << "Your hand:" << endl;
    for (size_t i = 0; i < cards.size(); ++i) {
        cout << "  " << (i + 1) << ": " << cards[i] << endl;
    }
}
}

int main()
{
    Socket sock;
    sock.create("127.0.0.1", PORT);
    cout << "Connected to server." << endl;

    bool running = true;
    while (running) {
        string line = sock.response();
        if (line.empty()) break;

        if (line.rfind("HELLO ", 0) == 0) {
            cout << "Assigned as Player " << line.substr(6) << endl;
            continue;
        }
        if (line.rfind("GAME_OPTIONS ", 0) == 0) {
            cout << "Selected game: POKER" << endl;
            sock.request("GAME POKER");
            continue;
        }
        if (line.rfind("HAND ", 0) == 0) {
            show_hand(line.substr(5));
            continue;
        }
        if (line == "REQUEST_EXCHANGE") {
            cout << "Exchange card positions (1-5, separated by spaces, or 0): ";
            string input;
            getline(cin, input);
            if (input.empty()) input = "0";
            sock.request("EXCHANGE " + input);
            continue;
        }
        if (line.rfind("INFO ", 0) == 0) {
            cout << line.substr(5) << endl;
            continue;
        }
        if (line.rfind("RESULT ", 0) == 0) {
            cout << "Game result: " << line.substr(7) << endl;
            continue;
        }
        if (line.rfind("RESULT_DETAIL ", 0) == 0) {
            cout << "Detail: " << line.substr(14) << endl;
            continue;
        }
        if (line == "REMATCH_PROMPT") {
            cout << "Play again? (y/n): ";
            string input;
            getline(cin, input);
            if (!input.empty() && (input[0] == 'y' || input[0] == 'Y')) {
                sock.request("REMATCH YES");
            } else {
                sock.request("REMATCH NO");
            }
            continue;
        }
        if (line == "ROUND_NEXT") {
            cout << "Next round started." << endl;
            continue;
        }
        if (line == "SESSION_END") {
            cout << "Session ended." << endl;
            running = false;
            continue;
        }
        cout << "Server: " << line << endl;
    }

    sock.finish();
    return 0;
}
