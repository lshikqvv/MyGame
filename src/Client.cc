#include "connection/Socket.h"
#include <vector>
#include <sstream>
// #include "window_app/Window_app.h"

using namespace std;
// using namespace window_app;
using namespace connection;

static const int PORT = 8080;
static const int MAX_CLNT = 2;

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
    if (!item.empty()) {
            elems.push_back(item);
        }
    }
    return elems;
}

int main(int argc, char *argv[])
{
    // auto app = Gtk::Application::create(argc, argv);
    // WindowApp window_app(1280, 960);
    // app->run(window_app);

    while(true)
    {
        string data;
        cin >> data;

        Socket sock;
        cout << "Greetings." << endl;
        sock.create("127.0.0.1", PORT);
        sock.request(data);

        // Receive and display the hand
        string hand = sock.response();
        vector<string> cards = split(hand, ' ');
        cout << "Your hand is: ";
        for (const string& card : cards) {
            cout << card << " ";
        }
        cout << endl;

        // Ask for cards to exchange
        cout << "Enter the numbers of the cards to exchange (1-5), separated by spaces. Enter 0 if you don't want to exchange any cards: ";
        string cards_to_exchange;
        cin >> cards_to_exchange;
        sock.request(cards_to_exchange);

        // Receive and display the new hand
        string new_hand = sock.response();
        cout << "Your new hand is: " << new_hand << endl;

        // Receive and display the result
        string result = sock.response();
        cout << "Result: " << result << endl;

        sock.finish();
    }

    return 0;
}
