#include "Card.h"

using namespace card;

std::pair<char, char> Card::getCard()
{
    static const char *num_str[] = {"23456789TJQKA"};
    const char* num;
    const char* suit;

    num = num_str[((idx >> 8) & 0xF)];
    if (idx & 0x8000) {
        suit = "Club";
    } else if (idx & 0x4000) {
        suit = "Diamond";
    } else if (idx & 0x2000) {
        suit = "Heart";
    } else if (idx & 0x1000) {
        suit = "Spade";
    }

    return {*num, *suit};
}

std::string Card::toString()
{
    std::string cardStr = "";
    std::pair<char, char> card = getCard();
    cardStr += card.first;
    cardStr += " of ";
    cardStr += card.second;

    return cardStr;
}

