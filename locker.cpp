#include "locker.h"

static const char* CULPRIT_ROLES[7] = {
    "Cook", "Guard", "Servant", "Partner 1", "Partner 2", "PA", "Child"
};

// ============================================================
// Tile and Clue operators
// ============================================================
ostream& operator<<(ostream& os, const Tile& t) {
    os << "[" << (t.filled ? t.tileLetter : ' ') << "]";
    return os;
}

istream& operator>>(istream& is, Tile& t) {
    char c;
    if (is >> c) {
        t.tileLetter = (char)toupper((unsigned char)c);
        t.filled = true;
    }
    return is;
}

bool operator==(const Tile& a, const Tile& b) {
    return toupper((unsigned char)a.tileLetter) == toupper((unsigned char)b.tileLetter);
}

bool operator==(const Clue& a, const Clue& b) {
    return toupper((unsigned char)a.clueLetter) == toupper((unsigned char)b.clueLetter)
        && a.cluePosition == b.cluePosition && a.clueRow == b.clueRow;
}

ostream& operator<<(ostream& os, const Location& l) {
    os << l.locName << (l.locSearchFlag ? "  Searched" : "  NOT Searched");
    return os;
}

istream& operator>>(istream& is, Location& l) {
    is >> l.locName;
    return is;
}