#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cctype>
using namespace std;

// ============================================================
// MODULE 1: Enums and Structs
// ============================================================
enum GameState { STATE_MENU, STATE_PLAYING, STATE_WON, STATE_LOST, STATE_EXIT };
enum MenuChoice { MENU_NEW_GAME = 1, MENU_SCOREBOARD, MENU_ABOUT, MENU_RULES, MENU_EXIT };
enum WordRow { WORD_1 = 0, WORD_2 = 1 };
enum PenaltyType { PENALTY_NONE, PENALTY_TURN, PENALTY_WORD };

struct Tile {
    char tileLetter;
    bool filled;
    Tile() : tileLetter(' '), filled(false) {}
    friend ostream& operator<<(ostream& os, const Tile& t);
    friend istream& operator>>(istream& is, Tile& t);
};

struct Clue {
    char clueLetter;
    int cluePosition;
    WordRow clueRow;
    Clue() : clueLetter(' '), cluePosition(0), clueRow(WORD_1) {}
};

struct Location {
    string locName;
    char letterClueLoc;
    int boardPosLoc;
    WordRow locRow;
    bool locSearchFlag;
    Location() : locName(""), letterClueLoc(' '), boardPosLoc(0),
        locRow(WORD_1), locSearchFlag(false) {
    }
    friend ostream& operator<<(ostream& os, const Location& l);
    friend istream& operator>>(istream& is, Location& l);
};

// Equality operators compare letters (MODULE 11)
bool operator==(const Tile& a, const Tile& b);
bool operator==(const Clue& a, const Clue& b);
