#pragma once

#include <iostream>
#include <stdlib.h>
#include <time.h>

// #include "arrays.h"
#include "Card.h"

namespace deck
{
    class Deck
    {
    public:
        int primes[13];
        const int DECK_SIZE = 52;
        int cardlst[52];
        int cardidx = 0;

        void init();
        Deck shuffle();
        card:: Card draw();
        void copy(Deck deck);
    };
}
