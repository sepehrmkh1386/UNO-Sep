#include "AccountManager.h"

#include <fstream>

#include "Recursion.h"
#include "TextUtils.h"

static bool ranksBefore(Account* const& a, Account* const& b) {
    if (a->getTotalScore() != b->getTotalScore()) {
        return a->getTotalScore() > b->getTotalScore();
    }

    return a->getWins() > b->getWins();
}

AccountManager::AccountManager(const string& path)
    : filePath(path), skippedLines(0) {
}

AccountManager::~AccountManager() {
    clearAll();
}

void AccountManager::clearAll() {
    while (accounts.size() > 0) {
        Account* account = nullptr;
        accounts.get(accounts.size() - 1, account);
        delete account;
        accounts.removeLast();
    }
}

int AccountManager::indexOf(const string& username) const {
    for (int i = 0; i < accounts.size(); ++i) {
        Account* account = nullptr;
        accounts.get(i, account);

        if (account->getUsername() == username) {
            return i;
        }
    }

    return -1;
}

RegisterResult AccountManager::registerAccount(const string& username,
                                               const string& password,
                                               const string& displayName) {
    if (username.empty() || hasWhitespace(username)) {
        return RegisterResult::InvalidUsername;
    }

    if (containsChar(username, '|') || containsChar(password, '|') ||
        containsChar(displayName, '|')) {
        return RegisterResult::ForbiddenCharacter;
    }

    if (indexOf(username) >= 0) {
        return RegisterResult::UsernameTaken;
    }

    if (password.size() < 4 || hasWhitespace(password)) {
        return RegisterResult::WeakPassword;
    }

    if (displayName.empty()) {
        return RegisterResult::InvalidDisplayName;
    }

    accounts.add(new Account(username, password, displayName));
    save();

    return RegisterResult::Ok;
}

Account* AccountManager::find(const string& username) const {
    int index = indexOf(username);

    if (index < 0) {
        return nullptr;
    }

    return getAt(index);
}

Account* AccountManager::login(const string& username,
                               const string& password) const {
    Account* account = find(username);

    if (account == nullptr || !account->checkPassword(password)) {
        return nullptr;
    }

    return account;
}

int AccountManager::size() const {
    return accounts.size();
}

Account* AccountManager::getAt(int index) const {
    Account* account = nullptr;
    accounts.get(index, account);
    return account;
}

int AccountManager::getSkippedLines() const {
    return skippedLines;
}

void AccountManager::ranked(DynamicArray<Account*>& result) const {
    for (int i = 0; i < accounts.size(); ++i) {
        result.add(getAt(i));
    }

    recursiveMergeSort(result, ranksBefore);
}

bool AccountManager::save() const {
    if (filePath.empty()) {
        return true;
    }

    ofstream out(filePath, ios::trunc);

    if (!out.is_open()) {
        return false;
    }

    for (int i = 0; i < accounts.size(); ++i) {
        Account* account = getAt(i);

        out << account->getUsername() << '|'
            << account->getPassword() << '|'
            << account->getDisplayName() << '|'
            << account->getTotalScore() << '|'
            << account->getWins() << '|'
            << account->getGamesPlayed() << '\n';
    }

    return out.good();
}

bool AccountManager::addLoaded(const string& line) {
    DynamicArray<string> parts;
    splitBy(line, '|', parts);

    if (parts.size() != 6) {
        return false;
    }

    string username = wordAt(parts, 0);
    string password = wordAt(parts, 1);
    string displayName = wordAt(parts, 2);
    int score = 0;
    int wins = 0;
    int games = 0;

    if (username.empty() || hasWhitespace(username) ||
        password.empty() || displayName.empty()) {
        return false;
    }

    if (!parseNonNegativeInt(wordAt(parts, 3), score) ||
        !parseNonNegativeInt(wordAt(parts, 4), wins) ||
        !parseNonNegativeInt(wordAt(parts, 5), games)) {
        return false;
    }

    if (indexOf(username) >= 0) {
        return false;
    }

    Account* account = new Account(username, password, displayName);
    account->restoreStats(score, wins, games);
    accounts.add(account);

    return true;
}

bool AccountManager::load() {
    clearAll();
    skippedLines = 0;

    if (filePath.empty()) {
        return true;
    }

    ifstream in(filePath);

    if (!in.is_open()) {
        return true;
    }

    string line;

    while (getline(in, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }

        if (line.empty()) {
            continue;
        }

        if (!addLoaded(line)) {
            ++skippedLines;
        }
    }

    return true;
}