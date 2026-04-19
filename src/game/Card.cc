#include "Card.h"

using namespace card;

std::pair<std::string, std::string> Card::getCard()
{
    static const char *num_str[] = {"2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A"};
    std::string num;
    std::string suit;

    int rank = getRank();
    if (rank >= 0 && rank <= 12) {
        num = num_str[rank];
    } else {
        num = "?";
    }
    if (idx & 0x8000) {
        suit = "Club";
    } else if (idx & 0x4000) {
        suit = "Diamond";
    } else if (idx & 0x2000) {
        suit = "Heart";
    } else if (idx & 0x1000) {
        suit = "Spade";
    }

    return {num, suit};
}

std::string Card::toString()
{
    std::string cardStr = "";
    auto card = getCard();
    cardStr += card.first;
    cardStr += " of ";
    cardStr += card.second;

    return cardStr;
}

std::string Card::toCode()
{
    static const char rank_chars[] = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'};
    std::string code;
    int rank = getRank();
    if (rank >= 0 && rank <= 12) {
        code.push_back(rank_chars[rank]);
    } else {
        code.push_back('?');
    }
    code.push_back(getSuitCode());
    return code;
}

int Card::getRank() const
{
    return (idx >> 8) & 0xF;
}

char Card::getSuitCode() const
{
    if (idx & 0x8000) return 'C';
    if (idx & 0x4000) return 'D';
    if (idx & 0x2000) return 'H';
    return 'S';
}
