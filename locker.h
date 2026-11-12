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
enum PenaltyType { PENALTY_NONE, PENALTY_TURN, PENALTY_WORD };sss

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




// ============================================================
// MODULE 7: Exception handling
// Every exception says whether it costs credibility and/or a turn.
// GameEngine maps PenaltyType to the values loaded from rules.txt.
// ============================================================
class GameException {
protected:
    string message;
    PenaltyType penaltyType;
    bool turnUsed;
public:
    GameException(const string& msg, PenaltyType p = PENALTY_NONE, bool usesTurnFlag = false)
        : message(msg), penaltyType(p), turnUsed(usesTurnFlag) {
    }
    virtual ~GameException() {}
    virtual const char* what() const { return message.c_str(); }
    PenaltyType getPenaltyType() const { return penaltyType; }
    bool usesTurn() const { return turnUsed; }
};

class InvalidInputException : public GameException {
public: InvalidInputException(const string& m) : GameException(m) {}
};
class FileException : public GameException {
public: FileException(const string& m) : GameException(m) {}
};
class RackFullException : public GameException {
public: RackFullException(const string& m) : GameException(m) {}
};
class RackEmptyException : public GameException {
public: RackEmptyException(const string& m) : GameException(m) {}
};
class InvalidPositionException : public GameException {
public: InvalidPositionException(const string& m) : GameException(m) {}
};
class SlotFilledException : public GameException {
public: SlotFilledException(const string& m) : GameException(m) {}
};
class WrongLetterException : public GameException {
public: WrongLetterException(const string& m) : GameException(m) {}
};
class InvalidWordException : public GameException {      // wrong word: word penalty
public: InvalidWordException(const string& m) : GameException(m, PENALTY_WORD, false) {}
};
class LowCredibilityException : public GameException {   // wrong turn
public: LowCredibilityException(const string& m) : GameException(m, PENALTY_TURN, true) {}
};
class IncompleteRowException : public GameException {    // wrong turn
public: IncompleteRowException(const string& m) : GameException(m, PENALTY_TURN, true) {}
};
class WrongAccusationException : public GameException { // wrong turn
public: WrongAccusationException(const string& m) : GameException(m, PENALTY_TURN, true) {}
};

// ============================================================
// Utility helpers (static only, no objects needed)
// ============================================================
class Utils {
public:
    static string trim(const string& s);
    static int split(const string& line, char delim, string parts[], int maxParts);
    static int toInt(const string& s, bool& ok);
    static string intToString(int n);
    static string toUpper(const string& s);
    static string underscoreToSpace(const string& s);
    static string spaceToUnderscore(const string& s);
    static bool hasSpace(const string& s);
    static string pad(const string& s, int width);
    static string readLine();
    static int readInt(const string& prompt, int lo, int hi);
    static bool askYesNo(const string& prompt);
    static void clearScreen();
    static void pause();
    static string today();
    static void makeDirectory(const string& name);
};