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









// ============================================================
// MODULE 5: TileRack (raw DMA, deep copy)
// Letters are kept sorted by board position.
// ============================================================
class TileRack {
private:
    Tile* tiles;
    int* positions;
    int capacity;
    int count;
public:
    TileRack(int cap = 8);
    TileRack(const TileRack& other);              // deep copy
    TileRack& operator=(const TileRack& other);   // deep copy
    ~TileRack();

    void addTile(char letter, int position);      // throws RackFullException
    void removeTile(char letter);                 // throws RackEmptyException
    bool isFull() const;
    bool isEmpty() const;
    int getCount() const;
    string toString() const;
    void display() const;

    // MODULE 11: merge two racks into one clue with a space between
    TileRack operator+(const TileRack& other) const;
    friend ostream& operator<<(ostream& os, const TileRack& r);
};

// ============================================================
// MODULE 3 and 9: Character hierarchy
// Character is abstract. display() is resolved at runtime.
// ============================================================
class Character {
private:
    bool isCulprit;        // set by GameEngine (friend)
    bool interviewed;      // set by GameEngine (friend)
protected:
    string characterName;
    string nationality;
    string characterRole;
public:
    Character(const string& name, const string& role, const string& nat);
    virtual ~Character();

    virtual void display() const = 0;                 // pure virtual
    virtual int requiredCredibility() const;          // interview gate
    virtual Clue getClue() const;                     // letter carried by this person

    string getName() const;
    string getRole() const;
    string getNationality() const;
    bool getIsCulprit() const;
    bool isInterviewed() const;

    friend class GameEngine;                          // MODULE 11
    friend ostream& operator<<(ostream& os, const Character& c);
    friend istream& operator>>(istream& is, Character& c);
};

class Victim : public Character {
private:
    char letterClueChar;
    int boardPosChar;
    WordRow clueRowChar;
public:
    Victim(const string& name, const string& nat, char letter, int pos);
    void display() const;
    int requiredCredibility() const;
    Clue getClue() const;
};

class Suspect : public Character {
protected:
    char letterClueChar;
    int boardPosChar;
    WordRow clueRowChar;
public:
    Suspect(const string& name, const string& role, const string& nat, char letter, int pos);
    virtual ~Suspect() {}
    Clue getClue() const;
    // display() stays pure virtual, so Suspect is abstract too
};

// Virtual inheritance solves the diamond: Suspect exists once inside PA
class BusinessRelations : virtual public Suspect {
public:
    BusinessRelations(const string& name, const string& role, const string& nat, char letter, int pos)
        : Suspect(name, role, nat, letter, pos) {
    }
};

class CloseRelations : virtual public Suspect {
public:
    CloseRelations(const string& name, const string& role, const string& nat, char letter, int pos)
        : Suspect(name, role, nat, letter, pos) {
    }
};

class Partner : public BusinessRelations {
public:
    Partner(const string& name, const string& role, const string& nat, char letter, int pos)
        : Suspect(name, role, nat, letter, pos),
        BusinessRelations(name, role, nat, letter, pos) {
    }
    void display() const;
    int requiredCredibility() const;   // 50
};

class PA : public BusinessRelations, public CloseRelations {
public:
    PA(const string& name, const string& nat, char letter, int pos)
        : Suspect(name, "PA", nat, letter, pos),
        BusinessRelations(name, "PA", nat, letter, pos),
        CloseRelations(name, "PA", nat, letter, pos) {
    }
    void display() const;
    int requiredCredibility() const;   // 35
};

class HouseHelp : public Suspect {
public:
    HouseHelp(const string& name, const string& role, const string& nat, char letter, int pos)
        : Suspect(name, role, nat, letter, pos) {
    }
};

class Cook : public HouseHelp {
public:
    Cook(const string& name, const string& nat, char letter, int pos)
        : HouseHelp(name, "Cook", nat, letter, pos) {
    }
    void display() const;
};

class Guard : public HouseHelp {
public:
    Guard(const string& name, const string& nat, char letter, int pos)
        : HouseHelp(name, "Guard", nat, letter, pos) {
    }
    void display() const;
};

class Servant : public HouseHelp {
public:
    Servant(const string& name, const string& nat, char letter, int pos)
        : HouseHelp(name, "Servant", nat, letter, pos) {
    }
    void display() const;
};

class OwnersChild : public CloseRelations {
public:
    OwnersChild(const string& name, const string& nat, char letter, int pos)
        : Suspect(name, "Child", nat, letter, pos),
        CloseRelations(name, "Child", nat, letter, pos) {
    }
    void display() const;
    int requiredCredibility() const;   // 75
};

class Detective : public Character {
private:
    int credibility;
    int turns;
    TileRack rack1;          // letters for Word 1 (7)
    TileRack rack2;          // letters for Word 2 (8)
    string solved1;
    string solved2;
    void useTurn();          // only GameEngine increments turns
    friend class GameEngine;
public:
    Detective(const string& name, int startCredibility);
    void display() const;
    void loseCredibility(int amount);
    int getCredibility() const;
    int getTurns() const;
    TileRack& getRack(WordRow row);
    void setSolved(WordRow row, const string& word);
    bool isSolved(WordRow row) const;
    string getSolved(WordRow row) const;
};





// ============================================================
// MODULE 5: TileRack (raw DMA, deep copy)
// Letters are kept sorted by board position.
// ============================================================
class TileRack {
private:
    Tile* tiles;
    int* positions;
    int capacity;
    int count;
public:
    TileRack(int cap = 8);
    TileRack(const TileRack& other);              // deep copy
    TileRack& operator=(const TileRack& other);   // deep copy
    ~TileRack();

    void addTile(char letter, int position);      // throws RackFullException
    void removeTile(char letter);                 // throws RackEmptyException
    bool isFull() const;
    bool isEmpty() const;
    int getCount() const;
    string toString() const;
    void display() const;

    // MODULE 11: merge two racks into one clue with a space between
    TileRack operator+(const TileRack& other) const;
    friend ostream& operator<<(ostream& os, const TileRack& r);
};

// ============================================================
// MODULE 3 and 9: Character hierarchy
// Character is abstract. display() is resolved at runtime.
// ============================================================
class Character {
private:
    bool isCulprit;        // set by GameEngine (friend)
    bool interviewed;      // set by GameEngine (friend)
protected:
    string characterName;
    string nationality;
    string characterRole;
public:
    Character(const string& name, const string& role, const string& nat);
    virtual ~Character();

    virtual void display() const = 0;                 // pure virtual
    virtual int requiredCredibility() const;          // interview gate
    virtual Clue getClue() const;                     // letter carried by this person

    string getName() const;
    string getRole() const;
    string getNationality() const;
    bool getIsCulprit() const;
    bool isInterviewed() const;

    friend class GameEngine;                          // MODULE 11
    friend ostream& operator<<(ostream& os, const Character& c);
    friend istream& operator>>(istream& is, Character& c);
};

class Victim : public Character {
private:
    char letterClueChar;
    int boardPosChar;
    WordRow clueRowChar;
public:
    Victim(const string& name, const string& nat, char letter, int pos);
    void display() const;
    int requiredCredibility() const;
    Clue getClue() const;
};

class Suspect : public Character {
protected:
    char letterClueChar;
    int boardPosChar;
    WordRow clueRowChar;
public:
    Suspect(const string& name, const string& role, const string& nat, char letter, int pos);
    virtual ~Suspect() {}
    Clue getClue() const;
    // display() stays pure virtual, so Suspect is abstract too
};

// Virtual inheritance solves the diamond: Suspect exists once inside PA
class BusinessRelations : virtual public Suspect {
public:
    BusinessRelations(const string& name, const string& role, const string& nat, char letter, int pos)
        : Suspect(name, role, nat, letter, pos) {
    }
};

class CloseRelations : virtual public Suspect {
public:
    CloseRelations(const string& name, const string& role, const string& nat, char letter, int pos)
        : Suspect(name, role, nat, letter, pos) {
    }
};

class Partner : public BusinessRelations {
public:
    Partner(const string& name, const string& role, const string& nat, char letter, int pos)
        : Suspect(name, role, nat, letter, pos),
        BusinessRelations(name, role, nat, letter, pos) {
    }
    void display() const;
    int requiredCredibility() const;   // 50
};

class PA : public BusinessRelations, public CloseRelations {
public:
    PA(const string& name, const string& nat, char letter, int pos)
        : Suspect(name, "PA", nat, letter, pos),
        BusinessRelations(name, "PA", nat, letter, pos),
        CloseRelations(name, "PA", nat, letter, pos) {
    }
    void display() const;
    int requiredCredibility() const;   // 35
};

class HouseHelp : public Suspect {
public:
    HouseHelp(const string& name, const string& role, const string& nat, char letter, int pos)
        : Suspect(name, role, nat, letter, pos) {
    }
};

class Cook : public HouseHelp {
public:
    Cook(const string& name, const string& nat, char letter, int pos)
        : HouseHelp(name, "Cook", nat, letter, pos) {
    }
    void display() const;
};

class Guard : public HouseHelp {
public:
    Guard(const string& name, const string& nat, char letter, int pos)
        : HouseHelp(name, "Guard", nat, letter, pos) {
    }
    void display() const;
};

class Servant : public HouseHelp {
public:
    Servant(const string& name, const string& nat, char letter, int pos)
        : HouseHelp(name, "Servant", nat, letter, pos) {
    }
    void display() const;
};

class OwnersChild : public CloseRelations {
public:
    OwnersChild(const string& name, const string& nat, char letter, int pos)
        : Suspect(name, "Child", nat, letter, pos),
        CloseRelations(name, "Child", nat, letter, pos) {
    }
    void display() const;
    int requiredCredibility() const;   // 75
};

class Detective : public Character {
private:
    int credibility;
    int turns;
    TileRack rack1;          // letters for Word 1 (7)
    TileRack rack2;          // letters for Word 2 (8)
    string solved1;
    string solved2;
    void useTurn();          // only GameEngine increments turns
    friend class GameEngine;
public:
    Detective(const string& name, int startCredibility);
    void display() const;
    void loseCredibility(int amount);
    int getCredibility() const;
    int getTurns() const;
    TileRack& getRack(WordRow row);
    void setSolved(WordRow row, const string& word);
    bool isSolved(WordRow row) const;
    string getSolved(WordRow row) const;
};




// ============================================================
// MODULE 4: Scrabble board (jagged DMA grid)
// ============================================================
class Scrabble {
private:
    Tile** grid;
    int rowLen[2];
    string target[2];
    string lastInput;
    void allocate();
    void release();
    void copyFrom(const Scrabble& other);
public:
    Scrabble();
    Scrabble(const Scrabble& other);
    Scrabble& operator=(const Scrabble& other);
    ~Scrabble();

    void setWords(const string& w1, const string& w2);
    void placeLetter(const Clue& c);                       // throws on bad placement
    bool isRowComplete(WordRow row) const;
    string getWord(WordRow row) const;
    bool checkWord(WordRow row, const string& guess) const; // throws InvalidWordException
    string getLastInput() const;
    void display() const;

    friend ostream& operator<<(ostream& os, const Scrabble& s);
    friend istream& operator>>(istream& is, Scrabble& s);
};