#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <string>

#include "../ds/DynamicArray.h"

using namespace std;

inline bool isSpaceChar(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline string toLowerText(const string& text) {
    string result = text;

    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i] >= 'A' && result[i] <= 'Z') {
            result[i] = static_cast<char>(result[i] - 'A' + 'a');
        }
    }

    return result;
}

inline string toUpperText(const string& text) {
    string result = text;

    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i] >= 'a' && result[i] <= 'z') {
            result[i] = static_cast<char>(result[i] - 'a' + 'A');
        }
    }

    return result;
}

inline bool hasWhitespace(const string& text) {
    for (size_t i = 0; i < text.size(); ++i) {
        if (isSpaceChar(text[i])) {
            return true;
        }
    }

    return false;
}

inline bool containsChar(const string& text, char target) {
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == target) {
            return true;
        }
    }

    return false;
}

inline bool parseNonNegativeInt(const string& text, int& result) {
    if (text.empty() || text.size() > 9) {
        return false;
    }

    int value = 0;

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] < '0' || text[i] > '9') {
            return false;
        }

        value = value * 10 + (text[i] - '0');
    }

    result = value;
    return true;
}

inline string wordAt(const DynamicArray<string>& words, int index) {
    string word;
    words.get(index, word);
    return word;
}

inline void splitWords(const string& line, DynamicArray<string>& words) {
    string current = "";

    for (size_t i = 0; i < line.size(); ++i) {
        if (isSpaceChar(line[i])) {
            if (!current.empty()) {
                words.add(current);
                current = "";
            }
        } else {
            current += line[i];
        }
    }

    if (!current.empty()) {
        words.add(current);
    }
}

inline void splitBy(const string& text, char separator,
                    DynamicArray<string>& parts) {
    string current = "";

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == separator) {
            parts.add(current);
            current = "";
        } else {
            current += text[i];
        }
    }

    parts.add(current);
}

inline string padRight(const string& text, int width) {
    string result = text;

    while (static_cast<int>(result.size()) < width) {
        result += ' ';
    }

    return result;
}

#endif