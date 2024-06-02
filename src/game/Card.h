#pragma once

#include <utility>
#include <string>

namespace card
{
    class Card
    {
    public:
        // const int CLUB    = 0x8000;
        // const int DIAMOND = 0x4000;
        // const int HEART   = 0x2000;
        // const int SPADE   = 0x1000;

        // const int Deuse = 0;
        // const int Trey  = 1;
        // const int Four  = 2;
        // const int Five  = 3;
        // const int Six   = 4;
        // const int Seven = 5;
        // const int Eight = 6;
        // const int Nine  = 7;
        // const int Ten   = 8;
        // const int Jack  = 9;
        // const int Queen = 10;
        // const int King  = 11;
        // const int Ace   = 12;

        int idx;

        Card() : idx(0) {}
        Card(int index) : idx(index) {}

        std::pair<char, char> getCard();
        std::string toString();
    };
}
