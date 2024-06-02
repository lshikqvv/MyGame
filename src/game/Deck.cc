// #include "arrays.h"
#include "Card.h"
#include "Deck.h"

using namespace deck;
using namespace card;
using namespace std;

/*
** each of the thirteen card ranks has its own prime number
**
** deuce = 2, trey  = 3, four  = 5, five  = 7, ..., king  = 37, ace   = 41
*/
int primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41 };

void Deck::init()
{
    const int suits[4] = {0x8000, 0x4000, 0x2000, 0x1000};

    int n = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 13; j++, n++) {
            cardlst[n] = primes[j] | (j << 8) | suits[i] | (1 << (16+j));
        }
    }

    cout << "Deck init" << endl;
}

Deck Deck::shuffle()
{
    for (int i = 0; i < DECK_SIZE; i++) {
        srand(time(NULL));
        int j = rand() % DECK_SIZE;
        int temp = cardlst[i];
        cardlst[i] = cardlst[j];
        cardlst[j] = temp;
    }

    cout << "Deck shuffle" << endl;

    return *this;
}

Card Deck::draw()
{
    Card card;
    card.idx = cardlst[cardidx];

    cardidx++;
    if (cardidx >= DECK_SIZE) {
        cardidx = 0;
        this->shuffle();
    }

    cout << " Deck draw" << endl;

    return card;
}

void Deck::copy(Deck deck)
{
    this->cardidx = deck.cardidx;
    this->cardlst[DECK_SIZE] = deck.cardlst[DECK_SIZE];

    cout << "Deck copy" << endl;
}
