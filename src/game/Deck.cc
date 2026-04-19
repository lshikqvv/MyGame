// #include "arrays.h"
#include "Card.h"
#include "Deck.h"
#include <algorithm>
#include <random>

using namespace deck;
using namespace card;
using namespace std;

/*
** each of the thirteen card ranks has its own prime number
**
** deuce = 2, trey  = 3, four  = 5, five  = 7, ..., king  = 37, ace   = 41
*/
static const int kPrimes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41 };

void Deck::init()
{
    const int suits[4] = {0x8000, 0x4000, 0x2000, 0x1000};

    int n = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 13; j++, n++) {
            cardlst[n] = kPrimes[j] | (j << 8) | suits[i] | (1 << (16 + j));
        }
    }

    cout << "Deck init" << endl;
}

void Deck::shuffle()
{
    static random_device rd;
    static mt19937 gen(rd());
    std::shuffle(cardlst, cardlst + DECK_SIZE, gen);

    cout << "Deck shuffle" << endl;
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

void Deck::copy(const Deck& deck)
{
    this->cardidx = deck.cardidx;
    for (int i = 0; i < DECK_SIZE; ++i) {
        this->cardlst[i] = deck.cardlst[i];
    }

    cout << "Deck copy" << endl;
}
