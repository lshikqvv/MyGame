#include "arrays.h"
#include "Card.h"
#include "Deck.h"
#include "Hand.h"

using namespace card;
using namespace deck;
using namespace hand;
using namespace std;

Hand::Hand() {
    for (int i = 0; i < 5; i++) {
        hand[i] = Card(i);  // Initialize each card with a unique index
    }
}

void Hand::make(Deck deck)
{
    for (int i = 0; i < 5; i++) {
        hand[i] = deck.draw();
    }
    cout << "Hand make" << endl;
}

void Hand::exchange(Deck deck)
{
    int cnt, num;
    cout << "Number of cards to be exchanged ? (0-5): ";
    cin >> cnt;
    if (cnt == 0)
        return;
    for (int i = 0; i < cnt; i++) {
        cout << "Exchange card number ? (1-5): ";
        cin >> num;
        hand[num-1] = deck.draw();
    }
    cout << "Hand change" << endl;
}

void Hand::sort()
{
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (hand[i].idx > hand[j].idx) {
                Card tmp = hand[i];
                hand[i] = hand[j];
                hand[j] = tmp;
            }
        }
    }
    cout << "Hand sort" << endl;
}

void Hand::show()
{
    cout << "Your hand is" << endl;
    char num, suit;
    for (int i = 0; i < 5; i++) {
        tie(num, suit) = hand[i].getCard();
        cout << num << " of " << suit << endl;
    }
}

string Hand::toString() {
    string handStr = "";
    for (int i = 0; i < 5; i++) {
        handStr += hand[i].toString() + " ";
    }
    return handStr;
}

// Perform a perfect hash lookup (courtesy of Paul Senzee).
unsigned Hand::find_fast(unsigned u)
{
    unsigned a, b, r;

    u += 0xe91aaa35;
    u ^= u >> 16;
    u += u << 8;
    u ^= u >> 4;
    b  = (u >> 8) & 0x1ff;
    a  = (u + (u << 2)) >> 19;
    r  = a ^ hash_adjust[b];
    return r;
}

unsigned short Hand::judge()
{
    int h1, h2, h3, h4, h5;
    int* hh[5] = {&h1, &h2, &h3, &h4, &h5};
    for (int i = 0; i < 5; i++) {
        *hh[i] = hand[i].idx;
    }

    int q = (h1 | h2 | h3 | h4 | h5) >> 16;
    short s;

    if (h1 & h2 & h3 & h4 & h5 & 0xf000) { // Flushes and Straight Flushes
        return flushes[q];
    }
    if ((s = unique5[q])) { // Straights and High Card
        return s;
    }
    // Perfect-hash lookup for remaining hands
    q = (h1 & 0xff) * (h2 & 0xff) * (h3 & 0xff) * (h4 & 0xff) * (h5 & 0xff);
    return hash_values[find_fast(q)];
}

// Returns the hand rank of the given equivalence class value.
// Note: the parameter "val" should be in the range of 1-7462.
int Hand::rank(unsigned short val)
{
    if (val > 6185) return HIGH_CARD;        // 1277 high card
    if (val > 3325) return ONE_PAIR;         // 2860 one pair
    if (val > 2467) return TWO_PAIR;         //  858 two pair
    if (val > 1609) return THREE_OF_A_KIND;  //  858 three-kind
    if (val > 1599) return STRAIGHT;         //   10 straights
    if (val > 322)  return FLUSH;            // 1277 flushes
    if (val > 166)  return FULL_HOUSE;       //  156 full house
    if (val > 10)   return FOUR_OF_A_KIND;   //  156 four-kind
    return STRAIGHT_FLUSH;                   //   10 straight-flushes
}
