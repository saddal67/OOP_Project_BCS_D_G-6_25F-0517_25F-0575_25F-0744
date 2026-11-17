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
