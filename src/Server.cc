#include <pthread.h>     // pthread_create

#include "connection/Socket.h"
#include "game/Deck.h"
#include "game/Hand.h"

using namespace std;
using namespace connection;
using namespace deck;
using namespace hand;

static const int PORT = 8080;

void *handler(void *arg)
{
    Deck deck;
    deck.init();
    deck.shuffle();

    Hand hand1;
    Hand hand2;

    while(true) {
        Socket sock1, sock2;

        hand1.make(deck);
        hand2.make(deck);

        hand1.sort();
        hand2.sort();

        hand1.show();
        hand2.show();

        string hand1Str = hand1.toString();
        string hand2Str = hand2.toString();

        cout << "Hand1: " << hand1Str << endl;
        cout << "Hand2: " << hand2Str << endl;

        sock1.request(hand1Str);
        sock2.request(hand2Str);

        sock1.request("Your turn");
        hand1.exchange(deck);
        hand1.sort();
        hand1.show();
        sock1.request(hand1.toString());

        sock2.request("Opponent's turn");
        hand2.exchange(deck);
        hand2.sort();
        hand2.show();
        sock2.request(hand2.toString());

        unsigned short score1 = hand1.judge();
        unsigned short score2 = hand2.judge();

        if (score1 < score2) {
            cout << "Player 1 wins!" << endl;
            sock1.request("You win!");
            sock2.request("You lose!");
        } else if (score1 > score2) {
            cout << "Player 2 wins!" << endl;
            sock1.request("You lose!");
            sock2.request("You win!");
        } else {
            cout << "It's a draw!" << endl;
            sock1.request("It's a draw!");
            sock2.request("It's a draw!");
        }
        sock1.finish();
        sock2.finish();
    }
}

int main()
{
    Socket sock1, sock2;
    cout << "Waiting for players..." << endl;
    sock1.serve(PORT);
    cout << "Player 1 connected" << endl;
    sock2.serve(PORT);
    cout << "Player 2 connected" << endl;
    cout << "Game started" << endl;

    pthread_t game_thread;

    if (pthread_create(&game_thread, NULL, handler, NULL) < 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }
    cout << "sock1: " << sock1.sockfd << endl;
    if (pthread_create(&game_thread, NULL, handler, NULL) < 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }
    cout << "sock2: " << sock2.sockfd << endl;

    pthread_join(game_thread, NULL);

    return 0;
}
