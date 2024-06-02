#pragma once

#include <iostream>
#include <tuple>

#include "arrays.h"
#include "Card.h"
#include "Deck.h"

namespace hand
{
    class Hand
    {
    public:
        const int STRAIGHT_FLUSH  = 1;
        const int FOUR_OF_A_KIND  = 2;
        const int FULL_HOUSE      = 3;
        const int FLUSH           = 4;
        const int STRAIGHT        = 5;
        const int THREE_OF_A_KIND = 6;
        const int TWO_PAIR        = 7;
        const int ONE_PAIR        = 8;
        const int HIGH_CARD       = 9;

        card::Card hand[5];

        Hand();
        void make(deck::Deck deck);
        void exchange(deck::Deck deck);
        void sort();
        void show();
        std::string toString();
        unsigned find_fast(unsigned u);
        unsigned short judge();
        int rank(unsigned short val);
    };
}
