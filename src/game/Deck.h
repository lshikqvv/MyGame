#pragma once

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <array>

// #include "arrays.h"
#include "Card.h"

namespace deck
{
    class Deck
    {
    public:
        const int DECK_SIZE = 52;
        int cardlst[52];
        int cardidx = 0;

        void init();
        void shuffle();
        card:: Card draw();
        void copy(const Deck& deck);
    };
}
