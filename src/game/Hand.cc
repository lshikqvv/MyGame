#include "arrays.h"
#include "Card.h"
#include "Deck.h"
#include "Hand.h"
#include <algorithm>
#include <array>
#include <set>
#include <sstream>

using namespace card;
using namespace deck;
using namespace hand;
using namespace std;

Hand::Hand() {
    for (int i = 0; i < 5; i++) {
        hand[i] = Card(i);  // Initialize each card with a unique index
    }
}

void Hand::make(Deck& deck)
{
    for (int i = 0; i < 5; i++) {
        hand[i] = deck.draw();
    }
    cout << "Hand make" << endl;
}

void Hand::exchange(Deck& deck, const vector<int>& positions)
{
    set<int> unique_positions;
    for (int pos : positions) {
        if (pos >= 1 && pos <= 5) {
            unique_positions.insert(pos);
        }
    }
    for (int pos : unique_positions) {
        hand[pos - 1] = deck.draw();
    }
    cout << "Hand exchange" << endl;
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
    std::string num, suit;
    for (int i = 0; i < 5; i++) {
        std::tie(num, suit) = hand[i].getCard();
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

string Hand::toCodeString()
{
    stringstream ss;
    for (int i = 0; i < 5; i++) {
        if (i > 0) ss << "|";
        ss << hand[i].toCode();
    }
    return ss.str();
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

namespace {
struct HandStrength {
    int category;
    vector<int> tiebreak;
};

vector<int> sortedRanksDesc(const array<int, 5>& ranks)
{
    vector<int> v(ranks.begin(), ranks.end());
    sort(v.begin(), v.end(), greater<int>());
    return v;
}

int straightHigh(const array<int, 5>& ranks)
{
    vector<int> unique = sortedRanksDesc(ranks);
    unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
    if (unique.size() != 5) return -1;

    if (unique[0] == 12 && unique[1] == 3 && unique[2] == 2 && unique[3] == 1 && unique[4] == 0) {
        return 3;
    }

    for (size_t i = 1; i < unique.size(); ++i) {
        if (unique[i - 1] - 1 != unique[i]) return -1;
    }
    return unique[0];
}

HandStrength evaluate(const Card (&cards)[5], int category)
{
    array<int, 5> ranks {};
    for (int i = 0; i < 5; ++i) {
        ranks[i] = cards[i].getRank();
    }

    array<int, 13> counts {};
    for (int r : ranks) counts[r]++;

    vector<pair<int, int>> groups;  // count, rank
    for (int r = 12; r >= 0; --r) {
        if (counts[r] > 0) groups.push_back({counts[r], r});
    }
    sort(groups.begin(), groups.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first) return a.first > b.first;
        return a.second > b.second;
    });

    HandStrength hs{category, {}};
    switch (category) {
        case 1:
        case 5:
            hs.tiebreak.push_back(straightHigh(ranks));
            break;
        case 2:
            hs.tiebreak = {groups[0].second, groups[1].second};
            break;
        case 3:
            hs.tiebreak = {groups[0].second, groups[1].second};
            break;
        case 4:
        case 9:
            hs.tiebreak = sortedRanksDesc(ranks);
            break;
        case 6: {
            hs.tiebreak.push_back(groups[0].second);
            vector<int> kickers;
            for (size_t i = 1; i < groups.size(); ++i) kickers.push_back(groups[i].second);
            sort(kickers.begin(), kickers.end(), greater<int>());
            hs.tiebreak.insert(hs.tiebreak.end(), kickers.begin(), kickers.end());
            break;
        }
        case 7: {
            int kicker = -1;
            vector<int> pairs;
            for (const auto& g : groups) {
                if (g.first == 2) pairs.push_back(g.second);
                if (g.first == 1) kicker = g.second;
            }
            sort(pairs.begin(), pairs.end(), greater<int>());
            hs.tiebreak = {pairs[0], pairs[1], kicker};
            break;
        }
        case 8: {
            hs.tiebreak.push_back(groups[0].second);
            vector<int> kickers;
            for (size_t i = 1; i < groups.size(); ++i) kickers.push_back(groups[i].second);
            sort(kickers.begin(), kickers.end(), greater<int>());
            hs.tiebreak.insert(hs.tiebreak.end(), kickers.begin(), kickers.end());
            break;
        }
        default:
            hs.tiebreak = sortedRanksDesc(ranks);
    }
    return hs;
}

string joinInts(const vector<int>& values)
{
    stringstream ss;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) ss << ",";
        ss << values[i];
    }
    return ss.str();
}
}

int Hand::category()
{
    return rank(judge());
}

string Hand::categoryName()
{
    int cat = category();
    if (cat == STRAIGHT_FLUSH) return "STRAIGHT_FLUSH";
    if (cat == FOUR_OF_A_KIND) return "FOUR_OF_A_KIND";
    if (cat == FULL_HOUSE) return "FULL_HOUSE";
    if (cat == FLUSH) return "FLUSH";
    if (cat == STRAIGHT) return "STRAIGHT";
    if (cat == THREE_OF_A_KIND) return "THREE_OF_A_KIND";
    if (cat == TWO_PAIR) return "TWO_PAIR";
    if (cat == ONE_PAIR) return "ONE_PAIR";
    return "HIGH_CARD";
}

string Hand::tiebreakString()
{
    HandStrength hs = evaluate(hand, category());
    return joinInts(hs.tiebreak);
}

int Hand::compareTo(Hand& other)
{
    int my_category = rank(judge());
    int other_category = rank(other.judge());

    if (my_category != other_category) {
        return (my_category < other_category) ? 1 : -1;
    }

    HandStrength mine = evaluate(hand, my_category);
    HandStrength theirs = evaluate(other.hand, other_category);

    size_t len = min(mine.tiebreak.size(), theirs.tiebreak.size());
    for (size_t i = 0; i < len; ++i) {
        if (mine.tiebreak[i] > theirs.tiebreak[i]) return 1;
        if (mine.tiebreak[i] < theirs.tiebreak[i]) return -1;
    }
    return 0;
}
