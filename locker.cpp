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



// ============================================================
// Utils
// ============================================================
string Utils::trim(const string& s) 
{
    int start = 0;
    int end = (int)s.length() - 1;
    while (start <= end && isspace((unsigned char)s[start])) start += 1;
    while (end >= start && isspace((unsigned char)s[end])) end -= 1;
    return s.substr(start, end - start + 1);
}

int Utils::split(const string& line, char delim, string parts[], int maxParts) {
    int count = 0;
    string cur = "";
    for (size_t i = 0; i < line.length(); i += 1) {
        if (line[i] == delim) {
            if (count < maxParts) parts[count] = trim(cur);
            count += 1;
            cur = "";
        }
        else {
            cur += line[i];
        }
    }
    if (count < maxParts) parts[count] = trim(cur);
    count = count +1;
    return count;
}


int Utils::toInt(const string& s, bool& ok) {
    string t = trim(s);
    ok = false;
    if (t.empty()) return 0;
    int i = 0;
    int sign = 1;
    if (t[0] == '-') { sign = -1; i = 1; }
    else if (t[0] == '+') { i = 1; }
    if (i >= (int)t.length()) return 0;
    long val = 0;
    for (; i < (int)t.length(); i += 1) {
        if (t[i] < '0' || t[i] > '9') return 0;
        val = val * 10 + (t[i] - '0');
        if (val > 100000) return 0;
    }
    ok = true;
    return (int)(sign * val);
}

string Utils::intToString(int n) {
    if (n == 0) return "0";
    bool neg = n < 0;
    if (neg) n = -n;
    string s = "";
    while (n > 0) {
        s = (char)('0' + n % 10) + s;
        n /= 10;
    }
    return neg ? "-" + s : s;
}
string Utils::toUpper(const string& s) {
    string r = s;
    for (size_t i = 0; i < r.length(); i += 1) r[i] = (char)toupper((unsigned char)r[i]);
    return r;
}

string Utils::underscoreToSpace(const string& s) {
    string r = s;
    for (size_t i = 0; i < r.length(); i += 1) if (r[i] == '_') r[i] = ' ';
    return r;
}

string Utils::spaceToUnderscore(const string& s) {
    string r = s;
    for (size_t i = 0; i < r.length(); i += 1) if (r[i] == ' ') r[i] = '_';
    return r;
}

bool Utils::hasSpace(const string& s) {
    for (size_t i = 0; i < s.length(); i += 1) if (isspace((unsigned char)s[i])) return true;
    return false;
}

string Utils::pad(const string& s, int width) {
    string r = s;
    while ((int)r.length() < width) r += " ";
    return r;
}

string Utils::readLine() {
    string line;
    if (!getline(cin, line)) {
        cout << "\nInput closed. Exiting.\n";
        exit(0);
    }
    return line;
}

int Utils::readInt(const string& prompt, int lo, int hi) {
    cout << prompt;
    string line = readLine();
    bool ok;
    int v = toInt(line, ok);
    if (!ok || v < lo || v > hi) {
        throw InvalidInputException("Invalid choice. Enter a number from "
            + intToString(lo) + " to " + intToString(hi) + ".");
    }
    return v;
}






bool Utils::askYesNo(const string& prompt) {
    while (true) {
        cout << prompt;
        string a = toUpper(trim(readLine()));
        if (a == "Y") return true;
        if (a == "N") return false;
        cout << "Please enter Y or N.\n";
    }
}




void Utils::clearScreen() 
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void Utils::pause() 
{
    cout << "\nPress Enter to continue...";
    readLine();
}

string Utils::today() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char buf[16];
    strftime(buf, sizeof(buf), "%d-%m-%y", t);
    return string(buf);
}

void Utils::makeDirectory(const string& name) {
#ifdef _WIN32
    string cmd = "if not exist " + name + " mkdir " + name;
#else
    string cmd = "mkdir -p " + name;
#endif
    system(cmd.c_str());
}



// ============================================================
// TileRack
// ============================================================
TileRack::TileRack(int cap) : tiles(NULL), positions(NULL), capacity(cap), count(0) {
    if (capacity < 1) capacity = 1;
    tiles = new Tile[capacity];
    positions = new int[capacity];
    for (int i = 0; i < capacity; i += 1) positions[i] = 0;
}

TileRack::TileRack(const TileRack& other)
    : tiles(NULL), positions(NULL), capacity(other.capacity), count(other.count) {
    tiles = new Tile[capacity];
    positions = new int[capacity];
    for (int i = 0; i < capacity; i += 1) {
        tiles[i] = other.tiles[i];
        positions[i] = other.positions[i];
    }
}

TileRack& TileRack::operator=(const TileRack& other) {
    if (this != &other) {
        Tile* nt = new Tile[other.capacity];
        int* np = new int[other.capacity];
        for (int i = 0; i < other.capacity; i += 1) {
            nt[i] = other.tiles[i];
            np[i] = other.positions[i];
        }
        delete[] tiles;
        delete[] positions;
        tiles = nt;
        positions = np;

        capacity = other.capacity;
        count = other.count;

    }
    
    return *this;
}





TileRack::~TileRack() {
    delete[] tiles;
    delete[] positions;
}

void TileRack::addTile(char letter, int position) {
    if (count >= capacity) throw RackFullException("The rack is full.");
    int at = 0;
    while (at < count&& positions[at] <= position) at += 1;
    for (int j = count; j > at; j -= 1) {
        tiles[j] = tiles[j - 1];
        positions[j] = positions[j - 1];
    }
    tiles[at].tileLetter = (char)toupper((unsigned char)letter);
    tiles[at].filled = true;
    positions[at] = position;
    count += 1;
}

void TileRack::removeTile(char letter) {
    if (count == 0) throw RackEmptyException("The rack is empty.");
    int at = -1;
    for (int i = 0; i < count; i += 1) {
        if (toupper((unsigned char)tiles[i].tileLetter) == toupper((unsigned char)letter)) {
            at = i;
            break;
        }
    }
    if (at == -1) throw WrongLetterException("That letter is not in the rack.");
    for (int j = at; j < count - 1; j += 1) {
        tiles[j] = tiles[j + 1];
        positions[j] = positions[j + 1];
    }
    tiles[count - 1] = Tile();
    positions[count - 1] = 0;
    count -= 1;
}
bool TileRack::isFull() const { return count >= capacity; }
bool TileRack::isEmpty() const { return count == 0; }
int TileRack::getCount() const { return count; }

string TileRack::toString() const {
    string s = "";
    for (int i = 0; i < count; i += 1) s += tiles[i].tileLetter;
    return s;
}

void TileRack::display() const {
    if (count == 0) {
        cout << "(empty)";
        return;
    }
    for (int i = 0; i < count; i += 1) {
        cout << tiles[i].tileLetter << "(" << positions[i] << ") ";
    }
}

TileRack TileRack::operator+(const TileRack& other) const {
    TileRack result(capacity + other.capacity + 1);
    int k = 0;
    for (int i = 0; i < count; i += 1) {
        result.tiles[k] = tiles[i];
        result.positions[k] = positions[i];
        k += 1;
    }
    result.tiles[k].tileLetter = ' ';      // the space between the two words
    result.tiles[k].filled = true;
    result.positions[k] = 0;
    k += 1;
    for (int i = 0; i < other.count; i += 1) {
        result.tiles[k] = other.tiles[i];
        result.positions[k] = other.positions[i];
        k += 1;
    }
    result.count = k;
    return result;
}



ostream& operator<<(ostream& os, const TileRack& r) {
    os << "Rack: ";
    if (r.count == 0) os << "(empty)";
    else os << r.toString();
    

    return os;
}


// ============================================================
// Character hierarchy
// ============================================================
Character::Character(const string& name, const string& role, const string& nat)
    : isCulprit(false), interviewed(false),
    characterName(name), nationality(nat), characterRole(role) {
}
Character::~Character() {}

int Character::requiredCredibility() const { return 0; }

Clue Character::getClue() const {
    Clue c;
    c.clueLetter = ' ';
    c.cluePosition = 0;
    c.clueRow = WORD_2;
    return c;
}
string Character::getName() const { return characterName; }
string Character::getRole() const { return characterRole; }
string Character::getNationality() const { return nationality; }
bool Character::getIsCulprit() const { return isCulprit; }
bool Character::isInterviewed() const { return interviewed; }

ostream& operator<<(ostream& os, const Character& c) {
    os << c.characterName << " (" << c.characterRole << ")";
    return os;
}

istream& operator>>(istream& is, Character& c) {
    is >> c.characterName >> c.nationality;
    return is;
}

Victim::Victim(const string& name, const string& nat, char letter, int pos)
    : Character(name, "Victim", nat), letterClueChar(letter), boardPosChar(pos), clueRowChar(WORD_2) {
}

void Victim::display() const {
    cout << "  " << characterName << " (" << nationality << ") owns the cottage and was robbed tonight.\n";
    cout << "  \"Whoever did this knew exactly which book opens the locker.\"\n";
}

int Victim::requiredCredibility() const 
{ 
    return 80;
}

Clue Victim::getClue() const {
    Clue c;
    c.clueLetter = letterClueChar;
    c.cluePosition = boardPosChar;
    c.clueRow = clueRowChar;
    return c;
}

Suspect::Suspect(const string& name, const string& role, const string& nat, char letter, int pos)
    : Character(name, role, nat), letterClueChar(letter), boardPosChar(pos), clueRowChar(WORD_2)
     {
}



Clue Suspect::getClue() const {
    Clue c;
    c.clueLetter = letterClueChar;
    c.cluePosition = boardPosChar;
    c.clueRow = clueRowChar;
    return c;
}

void Partner::display() const {
    cout << "  " << characterName << " (" << nationality << "), a business partner of the owner.\n";
    cout << "  \"I have plenty of deals to protect. Do not waste my time.\"\n";
}
int Partner::requiredCredibility() const { return 50; }

void PA::display() const {
    cout << "  " << characterName << " (" << nationality << "), the owner's trusted PA who knows the secrets.\n";
    cout << "  \"I hired you, detective. I want the truth as much as anyone.\"\n";
}
int PA::requiredCredibility() const { return 35; }

void Cook::display() const {
    cout << "  " << characterName << " (" << nationality << "), the cook, still smelling of the kitchen.\n";
    cout << "  \"I was busy with dinner all evening, I saw very little.\"\n";
}

void Guard::display() const {
    cout << "  " << characterName << " (" << nationality << "), the guard on duty during the party.\n";
    cout << "  \"The doors were locked. Nobody came in from outside.\"\n";
}

void Servant::display() const {
    cout << "  " << characterName << " (" << nationality << "), a servant who moved through every room.\n";
    cout << "  \"I only carried trays and cleaned up, sir.\"\n";
}

void OwnersChild::display() const {
    cout << "  " << characterName << " (" << nationality << "), the owner's teenage child, visibly panicked.\n";
    cout << "  \"I did not do anything! Please, I just want this to end.\"\n";
}


int OwnersChild::requiredCredibility() const { return 75; }

Detective::Detective(const string& name, int startCredibility)
    : Character(name, "Detective", "Pakistani"),
    credibility(startCredibility), turns(0), rack1(7), rack2(8), solved1(""), solved2("") {
}

void Detective::display() const {
    cout << "  Detective " << characterName << " | Credibility: " << credibility
        << " | Turns used: " << turns << "\n";
}

void Detective::useTurn() { turns += 1; }

void Detective::loseCredibility(int amount) {
    credibility -= amount;
    if (credibility < 0) credibility = 0;
}

int Detective::getCredibility() const { return credibility; }
int Detective::getTurns() const { return turns; }

TileRack& Detective::getRack(WordRow row) {
    return (row == WORD_1) ? rack1 : rack2;
}

void Detective::setSolved(WordRow row, const string& word) {
    if (row == WORD_1) solved1 = word; else solved2 = word;
}

bool Detective::isSolved(WordRow row) const {
    return row == WORD_1 ? !solved1.empty() : !solved2.empty();
}

string Detective::getSolved(WordRow row) const {
    return row == WORD_1 ? solved1 : solved2;
}



// ============================================================
// Scrabble
// ============================================================
void Scrabble::allocate() {
    rowLen[0] = 7;
    rowLen[1] = 8;
    grid = new Tile * [2];
    grid[0] = new Tile[rowLen[0]];
    grid[1] = new Tile[rowLen[1]];
}

void Scrabble::release() {
    if (grid != NULL) {
        delete[] grid[0];
        delete[] grid[1];
        delete[] grid;
        grid = NULL;
    }
}

void Scrabble::copyFrom(const Scrabble& other) {
    allocate();
    for (int r = 0; r < 2; r += 1) {
        for (int c = 0; c < rowLen[r]; c += 1) grid[r][c] = other.grid[r][c];
        target[r] = other.target[r];
    }
    lastInput = other.lastInput;
}

Scrabble::Scrabble() : grid(NULL), lastInput("") {
    allocate();
    target[0] = "";
    target[1] = "";
}

Scrabble::Scrabble(const Scrabble& other) : grid(NULL) { copyFrom(other); }

Scrabble& Scrabble::operator=(const Scrabble& other) {
    if (this != &other) 
    {
        release();
        copyFrom(other);
    }
    return *this;
}

Scrabble::~Scrabble() { release(); }

void Scrabble::setWords(const string& w1, const string& w2) {
    if ((int)w1.length() != rowLen[0] || (int)w2.length() != rowLen[1]) {
        throw FileException("Target words have the wrong length (need 7 and 8 letters).");
    }
    target[0] = Utils::toUpper(w1);
    target[1] = Utils::toUpper(w2);
}

void Scrabble::placeLetter(const Clue& c) {
    int r = (int)c.clueRow;
    if (c.cluePosition < 1 || c.cluePosition > rowLen[r]) {
        throw InvalidPositionException("Position " + Utils::intToString(c.cluePosition)
            + " does not exist on this row.");
    }
    Tile& slot = grid[r][c.cluePosition - 1];
    if (slot.filled) throw SlotFilledException("That slot is already filled.");
    if (!target[r].empty()) {
        Tile expected;
        expected.tileLetter = target[r][c.cluePosition - 1];
        Tile given;
        given.tileLetter = c.clueLetter;
        if (!(expected == given)) throw WrongLetterException("That letter does not belong in this slot.");
    }
    slot.tileLetter = (char)toupper((unsigned char)c.clueLetter);
    slot.filled = true;
}

bool Scrabble::isRowComplete(WordRow row) const {
    int r = (int)row;
    for (int i = 0; i < rowLen[r]; i += 1) if (!grid[r][i].filled) return false;
    return true;
}

string Scrabble::getWord(WordRow row) const {
    int r = (int)row;
    string w = "";
    for (int i = 0; i < rowLen[r]; i += 1) if (grid[r][i].filled) w += grid[r][i].tileLetter;
    return w;
}

bool Scrabble::checkWord(WordRow row, const string& guess) const {
    int r = (int)row;
    if ((int)guess.length() != rowLen[r]) {
        throw InvalidWordException("The word must have exactly " + Utils::intToString(rowLen[r]) + " letters.");
    }
    return Utils::toUpper(guess) == target[r];
}

string Scrabble::getLastInput() const { return lastInput; }

void Scrabble::display() const { cout << *this; }

ostream& operator<<(ostream& os, const Scrabble& s) {
    os << "Scrabble:\n";
    for (int r = 0; r < 2; r += 1) {
        os << "  Word " << (r + 1) << ": ";
        for (int c = 0; c < s.rowLen[r]; c += 1) os << s.grid[r][c];
        os << "\n";
    }
    return os;
}

istream& operator>>(istream& is, Scrabble& s) {
    s.lastInput = "";
    is >> s.lastInput;
    return is;
}

// ============================================================
// Cottage
// ============================================================
Cottage::Cottage() : locations(NULL), count(7) {
    locations = new Location[count];
}

Cottage::Cottage(const Cottage& other) : locations(NULL), count(other.count) {
    locations = new Location[count];
    for (int i = 0; i < count; i += 1) locations[i] = other.locations[i];
}

Cottage& Cottage::operator=(const Cottage& other) {
    if (this != &other) {
        Location* nl = new Location[other.count];
        for (int i = 0; i < other.count; i += 1) nl[i] = other.locations[i];
        delete[] locations;
        locations = nl;
        count = other.count;
    }
    return *this;
}

Cottage::~Cottage() { delete[] locations; }

void Cottage::loadFromFile(const string& file, int culpritIndex) {
    ifstream in(file.c_str());
    if (!in) throw FileException("Cannot open " + file + ". File not found.");
    string line;
    if (!getline(in, line)) throw FileException(file + " is empty or corrupted.");   // header
    for (int i = 0; i < count; i += 1) {
        do {
            if (!getline(in, line)) throw FileException(file + " is corrupted: expected 7 locations.");
            line = Utils::trim(line);
        } while (line.empty());

        string p[9];
        int n = Utils::split(line, '|', p, 9);
        if (n < 9) throw FileException(file + " is corrupted: a line has missing columns.");
        bool ok;
        int pos = Utils::toInt(p[1], ok);
        if (!ok || pos < 1 || pos > 7) throw FileException(file + " is corrupted: bad board position.");
        string letter = p[2 + culpritIndex];
        if (letter.empty() || !isalpha((unsigned char)letter[0]))
            throw FileException(file + " is corrupted: bad letter.");

        locations[i].locName = Utils::underscoreToSpace(p[0]);
        locations[i].letterClueLoc = (char)toupper((unsigned char)letter[0]);
        locations[i].boardPosLoc = pos;
        locations[i].locRow = WORD_1;
        locations[i].locSearchFlag = false;
    }
}

int Cottage::getLocationCount() const { return count; }

bool Cottage::isSearched(int idx) const {
    if (idx < 0 || idx >= count) throw InvalidInputException("No such location.");
    return locations[idx].locSearchFlag;
}

void Cottage::markSearched(int idx) {
    if (idx < 0 || idx >= count) throw InvalidInputException("No such location.");
    locations[idx].locSearchFlag = true;
}

string Cottage::getLocationName(int idx) const {
    if (idx < 0 || idx >= count) throw InvalidInputException("No such location.");
    return locations[idx].locName;
}

Clue Cottage::getClue(int idx) const {
    if (idx < 0 || idx >= count) throw InvalidInputException("No such location.");
    Clue c;
    c.clueLetter = locations[idx].letterClueLoc;
    c.cluePosition = locations[idx].boardPosLoc;
    c.clueRow = locations[idx].locRow;
    return c;
}

void Cottage::displayLocations() const {
    for (int i = 0; i < count; i += 1) {
        cout << "[" << (i + 1) << "] " << Utils::pad(locations[i].locName, 12)
            << (locations[i].locSearchFlag ? "Searched" : "NOT Searched") << "\n";
    }
}


// ============================================================
// ScoreBoard
// ============================================================
ScoreBoard::ScoreBoard(const string& p) : path(p) {}

void ScoreBoard::addScore(const string& detective, bool won, int credibility,
    int turnsLeft, const string& culprit) const {
    Utils::makeDirectory("leader");
    bool needHeader = true;
    {
        ifstream test(path.c_str());
        if (test) {
            test.seekg(0, ios::end);
            if (test.tellg() > 0) needHeader = false;
        }
    }
    ofstream out(path.c_str(), ios::app);
    if (!out) throw FileException("Cannot write to " + path + ".");
    if (needHeader) out << "Detective|Result|Credibility|Turns_Left|Culprit|Date\n";
    string t = Utils::intToString(turnsLeft);
    if (turnsLeft < 10) t = "0" + t;
    out << detective << "|" << (won ? "WIN" : "LOSE") << "|" << credibility << "|"
        << t << "|" << culprit << "|" << Utils::today() << "\n";
}

void ScoreBoard::display() const {
    ifstream in(path.c_str());
    if (!in) throw FileException("Leader/score board file not found: " + path);
    string line;
    if (!getline(in, line)) throw FileException("Leader/score board file is empty or corrupted.");

    cout << "\n=========== LEADER / SCORE BOARD ===========\n";
    cout << Utils::pad("Detective", 14) << Utils::pad("Result", 7) << Utils::pad("Cred", 6)
        << Utils::pad("Left", 6) << Utils::pad("Culprit", 12) << "Date\n";
    int rows = 0;
    while (getline(in, line)) {
        line = Utils::trim(line);
        if (line.empty()) continue;
        string p[6];
        int n = Utils::split(line, '|', p, 6);
        if (n < 6) throw FileException("Leader/score board file is corrupted.");
        cout << Utils::pad(p[0], 14) << Utils::pad(p[1], 7) << Utils::pad(p[2], 6)
            << Utils::pad(p[3], 6) << Utils::pad(p[4], 12) << p[5] << "\n";
        rows += 1;
    }
    if (rows == 0) cout << "No games played yet.\n";
    cout << "============================================\n";
}



// ============================================================
// GameEngine (Singleton)
// ============================================================
GameEngine* GameEngine::instance = NULL;

GameEngine::GameEngine(ScoreBoard* sb)
    : state(STATE_MENU), running(true),
    maxTurns(25), startCredibility(100), wrongTurnPenalty(5), wrongWordPenalty(10),
    winCondition("Accuse_Correct_Culprit"),
    loseCondition("Credibility_Zero_Or_Turns_Exhausted"),
    usingDefaultRules(false), culpritIndex(0), word1(""), word2(""),
    detective(NULL), cottage(NULL), scrabble(NULL), mergedRack(NULL), scoreBoard(sb) {
    for (int i = 0; i < CHAR_COUNT; i += 1) characters[i] = NULL;
    srand((unsigned)time(NULL));
    loadRules();
}


GameEngine::~GameEngine() { cleanupGame(); }

GameEngine* GameEngine::getInstance(ScoreBoard* sb) {
    if (instance == NULL) instance = new GameEngine(sb);
    return instance;
}

void GameEngine::destroyInstance() {
    delete instance;
    instance = NULL;
}

bool GameEngine::isRunning() const { return running; }
void GameEngine::stop() { running = false; }

void GameEngine::cleanupGame() {
    delete detective;   detective = NULL;
    delete cottage;     cottage = NULL;
    delete scrabble;    scrabble = NULL;
    delete mergedRack;  mergedRack = NULL;
    for (int i = 0; i < CHAR_COUNT; i += 1) {
        delete characters[i];       // virtual destructor frees the derived object
        characters[i] = NULL;
    }
}

// Rules fall back to defaults if the file is missing or corrupt
void GameEngine::loadRules() {
    maxTurns = 25; startCredibility = 100; wrongTurnPenalty = 5; wrongWordPenalty = 10;
    winCondition = "Accuse_Correct_Culprit";
    loseCondition = "Credibility_Zero_Or_Turns_Exhausted";
    usingDefaultRules = false;

    ifstream in("rules.txt");
    if (!in) { usingDefaultRules = true; return; }

    int mt = 25, sc = 100, wt = 5, ww = 10;
    string win = winCondition, lose = loseCondition;
    int found = 0;
    bool corrupt = false;
    string line;
    while (getline(in, line)) {
        line = Utils::trim(line);
        if (line.empty()) continue;
        size_t eq = line.find('=');
        if (eq == string::npos) continue;        // ignore heading lines
        string key = Utils::trim(line.substr(0, eq));
        string val = Utils::trim(line.substr(eq + 1));
        bool ok = true;
        if (key == "Max_Turns") { mt = Utils::toInt(val, ok); found += 1; }
        else if (key == "Starting_Credibility") { sc = Utils::toInt(val, ok); found += 1; }
        else if (key == "Every_Wrong_Turn") { wt = Utils::toInt(val, ok); found += 1; }
        else if (key == "Every_Wrong_Word") { ww = Utils::toInt(val, ok); found += 1; }
        else if (key == "Win_Condition") { win = val; }
        else if (key == "Lose_Condition") { lose = val; }
        if (!ok) corrupt = true;
    }
    if (wt < 0) wt = -wt;
    if (ww < 0) ww = -ww;
    if (corrupt || found < 4 || mt < 1 || sc < 1) { usingDefaultRules = true; return; }
    maxTurns = mt; startCredibility = sc; wrongTurnPenalty = wt; wrongWordPenalty = ww;
    winCondition = win; loseCondition = lose;
}


Character* GameEngine::createCharacter(const string& name, const string& role,
    const string& nat, char letter, int pos) {
    if (role == "Victim")    return new Victim(name, nat, letter, pos);
    if (role == "Cook")      return new Cook(name, nat, letter, pos);
    if (role == "Guard")     return new Guard(name, nat, letter, pos);
    if (role == "Servant")   return new Servant(name, nat, letter, pos);
    if (role == "Child")     return new OwnersChild(name, nat, letter, pos);
    if (role == "Partner 1" || role == "Partner 2") return new Partner(name, role, nat, letter, pos);
    if (role == "PA")        return new PA(name, nat, letter, pos);
    throw FileException("Unknown role in characters file: " + role);
}

// First data line must be the Victim, then the 7 suspects
void GameEngine::loadCharacters(const string& file) {
    ifstream in(file.c_str());
    if (!in) throw FileException("Cannot open " + file + ". File not found.");
    string line;
    if (!getline(in, line)) throw FileException(file + " is empty or corrupted.");   // header
    for (int i = 0; i < CHAR_COUNT; i += 1) {
        do {
            if (!getline(in, line)) throw FileException(file + " is corrupted: expected 8 characters.");
            line = Utils::trim(line);
        } while (line.empty());

        string p[12];
        int n = Utils::split(line, '|', p, 12);
        if (n < 12) throw FileException(file + " is corrupted: a line has missing columns.");
        bool ok;
        int pos = Utils::toInt(p[3], ok);
        if (!ok || pos < 1 || pos > 8) throw FileException(file + " is corrupted: bad board position.");
        string letter = p[4 + culpritIndex];
        if (letter.empty() || !isalpha((unsigned char)letter[0]))
            throw FileException(file + " is corrupted: bad letter.");

        characters[i] = createCharacter(p[0], p[1], p[2], (char)toupper((unsigned char)letter[0]), pos);
        if (p[1] == CULPRIT_ROLES[culpritIndex]) characters[i]->isCulprit = true;   // friend access
    }
    if (characters[0]->getRole() != "Victim")
        throw FileException(file + " is corrupted: first character must be the Victim.");
    if (findCulprit() == NULL)
        throw FileException(file + " is corrupted: culprit role not found.");
}
void GameEngine::loadClue(const string& file) {
    ifstream in(file.c_str());
    if (!in) throw FileException("Cannot open " + file + ". File not found.");
    string line;
    if (!getline(in, line)) throw FileException(file + " is empty or corrupted.");   // header
    bool found = false;
    while (getline(in, line)) {
        line = Utils::trim(line);
        if (line.empty()) continue;
        string p[3];
        int n = Utils::split(line, '|', p, 3);
        if (n < 3) throw FileException(file + " is corrupted: a line has missing columns.");
        if (p[0] == CULPRIT_ROLES[culpritIndex]) {
            word1 = Utils::toUpper(p[1]);
            word2 = Utils::toUpper(p[2]);
            found = true;
            break;
        }
    }
    if (!found) throw FileException(file + " has no profile for the chosen culprit.");
    scrabble->setWords(word1, word2);
}

Character* GameEngine::findCulprit() const {
    for (int i = 0; i < CHAR_COUNT; i += 1) {
        if (characters[i] != NULL && characters[i]->isCulprit) return characters[i];
    }
    return NULL;
}

void GameEngine::showTitle() const {
    cout << "============================================================\n";
    cout << "                     THE LOCKER HEIST\n";
    cout << "          A Detective Game backed by OOP Principles\n";
    cout << "============================================================\n";
    cout << "A locker hidden behind a bookshelf was robbed during a party.\n";
    cout << "Eight people were present. One of them is the culprit.\n";
    cout << "You are the detective. Find the clues before credibility runs out.\n";
}

void GameEngine::step() {
    if (state == STATE_EXIT) { running = false; return; }
    if (state == STATE_PLAYING) { gameStep(); return; }
    state = STATE_MENU;
    mainMenuStep();
}

void GameEngine::mainMenuStep() {
    cout << "\n========= THE LOCKER HEIST: MAIN MENU =========\n";
    cout << "  [1] New Game\n  [2] View Leader/Score Board\n  [3] About\n  [4] Rules\n  [5] Exit\n";
    cout << "===============================================\n";
    int c = Utils::readInt("Choice: ", 1, 5);
    switch ((MenuChoice)c) {
    case MENU_NEW_GAME:   newGame(); break;
    case MENU_SCOREBOARD:
        if (scoreBoard == NULL) throw FileException("No score board is attached.");
        scoreBoard->display();
        break;
    case MENU_ABOUT:      showAbout(); break;
    case MENU_RULES:      showRules(); break;
    case MENU_EXIT:       state = STATE_EXIT; running = false; break;
    }
}



void GameEngine::showAbout() const {
    ifstream in("about.txt");
    if (!in) throw FileException("Cannot open about.txt. File not found.");
    string line;
    cout << "\n";
    while (getline(in, line)) cout << line << "\n";
}

void GameEngine::showRules() const {
    cout << "\n================== RULES ==================\n";
    if (usingDefaultRules) cout << "(rules.txt missing or corrupt, showing default rules)\n";
    cout << "Max turns            : " << maxTurns << "\n";
    cout << "Starting credibility : " << startCredibility << "\n";
    cout << "Every wrong turn     : -" << wrongTurnPenalty << " credibility\n";
    cout << "Every wrong word     : -" << wrongWordPenalty << " credibility\n";
    cout << "Win condition        : " << winCondition << "\n";
    cout << "Lose condition       : " << loseCondition << "\n";
    cout << "\nInterview gates (minimum credibility):\n";
    cout << "  Business Owner 80 | Owner's Child 75 | Partners 50 | PA 35\n";
    cout << "  Cook, Guard, Servant and all locations: no condition\n";
    cout << "Viewing the Scrabble board is free and uses no turn.\n";
    cout << "A wrong turn is: a blocked interview, an incomplete word attempt,\n";
    cout << "or accusing the wrong person.\n";
    cout << "===========================================\n";
}

void GameEngine::newGame() {
    Utils::clearScreen();
    cout << "========= THE LOCKER HEIST: NEW GAME =========\n";
    string name;
    while (true) {
        cout << "Enter detective name (no spaces, use _ or . for a space): ";
        name = Utils::trim(Utils::readLine());
        if (name.empty()) { cout << "Name cannot be empty.\n"; continue; }
        if (Utils::hasSpace(name)) { cout << "Name cannot contain spaces.\n"; continue; }
        break;
    }
    cleanupGame();
    loadRules();
    if (usingDefaultRules) cout << "(rules.txt missing or corrupt, using default rules)\n";
    try {
        culpritIndex = rand() % SUSPECT_COUNT;
        detective = new Detective(name, startCredibility);
        cottage = new Cottage();
        scrabble = new Scrabble();
        cottage->loadFromFile("locations.txt", culpritIndex);
        loadCharacters("characters.txt");
        loadClue("clue.txt");
    }
    catch (...) {
        cleanupGame();
        throw;
    }
    state = STATE_PLAYING;
    cout << "A culprit has been secretly chosen. The game begins!\n";
}