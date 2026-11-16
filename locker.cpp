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

