#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>

using namespace std;

class Account {
private:
    string username;
    string password;
    string displayName;
    int totalScore;
    int wins;
    int gamesPlayed;

public:
    Account()
        : username(""), password(""), displayName(""),
          totalScore(0), wins(0), gamesPlayed(0) {
    }

    Account(const string& user, const string& pass, const string& display)
        : username(user), password(pass), displayName(display),
          totalScore(0), wins(0), gamesPlayed(0) {
    }

    const string& getUsername() const {
        return username;
    }

    const string& getPassword() const {
        return password;
    }

    const string& getDisplayName() const {
        return displayName;
    }

    int getTotalScore() const {
        return totalScore;
    }

    int getWins() const {
        return wins;
    }

    int getGamesPlayed() const {
        return gamesPlayed;
    }

    bool checkPassword(const string& attempt) const {
        return password == attempt;
    }

    void recordGame(int score, bool won) {
        totalScore += score;
        ++gamesPlayed;

        if (won) {
            ++wins;
        }
    }

    void restoreStats(int score, int winCount, int gameCount) {
        totalScore = score;
        wins = winCount;
        gamesPlayed = gameCount;
    }
};

#endif