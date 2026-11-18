#include <fstream>
#include <iostream>
#include <string>

#include "../src/logic/AccountManager.h"

using namespace std;

static int failedChecks = 0;
static const char* tempPath = "build/test_accounts_tmp.txt";

static void check(bool condition, const char* name) {
    if (condition) {
        cout << "[PASS] " << name << '\n';
    } else {
        cout << "[FAIL] " << name << '\n';
        ++failedChecks;
    }
}

static void testRegisterRules() {
    AccountManager manager("");

    check(manager.registerAccount("ali", "1234", "Ali Rezaei") ==
              RegisterResult::Ok && manager.size() == 1,
          "register: a valid account is added");
    check(manager.registerAccount("ali", "5678", "Other") ==
              RegisterResult::UsernameTaken && manager.size() == 1,
          "register: a duplicate username is rejected");
    check(manager.registerAccount("a li", "1234", "X") ==
              RegisterResult::InvalidUsername,
          "register: a username with a space is rejected");
    check(manager.registerAccount("", "1234", "X") ==
              RegisterResult::InvalidUsername,
          "register: an empty username is rejected");
    check(manager.registerAccount("sara", "123", "Sara") ==
              RegisterResult::WeakPassword,
          "register: a 3 character password is rejected");
    check(manager.registerAccount("sara", "12 34", "Sara") ==
              RegisterResult::WeakPassword,
          "register: a password with a space is rejected");
    check(manager.registerAccount("sara", "1234", "") ==
              RegisterResult::InvalidDisplayName,
          "register: an empty display name is rejected");
    check(manager.registerAccount("sa|ra", "1234", "Sara") ==
              RegisterResult::ForbiddenCharacter &&
          manager.registerAccount("sara", "12|34", "Sara") ==
              RegisterResult::ForbiddenCharacter &&
          manager.registerAccount("sara", "1234", "Sa|ra") ==
              RegisterResult::ForbiddenCharacter,
          "register: the '|' character is rejected everywhere");
    check(manager.registerAccount("sara", "1234", "Sara") ==
              RegisterResult::Ok && manager.size() == 2,
          "register: a 4 character password is enough");
}

static void testLoginAndFind() {
    AccountManager manager("");
    manager.registerAccount("ali", "1234", "Ali");
    manager.registerAccount("sara", "abcd", "Sara");

    Account* ali = manager.login("ali", "1234");
    check(ali != nullptr && ali->getDisplayName() == "Ali",
          "login: the right password works");
    check(manager.login("ali", "0000") == nullptr,
          "login: a wrong password fails");
    check(manager.login("nobody", "1234") == nullptr,
          "login: an unknown user fails");
    check(manager.login("ali", "") == nullptr,
          "login: an empty password fails");
    check(manager.find("sara") != nullptr && manager.find("zed") == nullptr,
          "find: returns the account or null");
    check(manager.getAt(0) == ali, "getAt: the first account is the first one");
}

static void testPointerStability() {
    AccountManager manager("");
    manager.registerAccount("user0", "1234", "User 0");
    Account* first = manager.getAt(0);

    for (int i = 1; i < 40; ++i) {
        manager.registerAccount("user" + to_string(i), "1234", "User");
    }

    first->recordGame(10, true);

    check(manager.size() == 40 && manager.getAt(0) == first &&
          first->getUsername() == "user0" &&
          manager.find("user0")->getTotalScore() == 10,
          "pointers: accounts keep their address after the array grows");
}

static void testSaveAndLoad() {
    {
        AccountManager manager(tempPath);
        manager.registerAccount("ali", "1234", "Ali Rezaei");
        manager.registerAccount("sara", "abcd", "Sara");
        manager.find("ali")->recordGame(120, true);
        manager.find("ali")->recordGame(30, false);
        check(manager.save(), "save: writing the file works");
    }

    AccountManager loaded(tempPath);
    check(loaded.load() && loaded.size() == 2 &&
          loaded.getSkippedLines() == 0,
          "load: both accounts are read back");

    Account* ali = loaded.find("ali");
    check(ali != nullptr && ali->getPassword() == "1234" &&
          ali->getDisplayName() == "Ali Rezaei" &&
          ali->getTotalScore() == 150 && ali->getWins() == 1 &&
          ali->getGamesPlayed() == 2,
          "load: passwords, names with spaces and stats are preserved");
    check(loaded.login("sara", "abcd") != nullptr,
          "load: a loaded account can log in");

    check(loaded.load() && loaded.size() == 2,
          "load: loading again does not duplicate accounts");

    {
        AccountManager autoSaved(tempPath);
        autoSaved.load();
        autoSaved.registerAccount("reza", "pass", "Reza");
    }

    AccountManager again(tempPath);
    again.load();
    check(again.size() == 3 && again.find("reza") != nullptr,
          "save: register saves the file automatically");
}

static void testMissingAndCorruptFiles() {
    AccountManager missing("build/this_file_does_not_exist.txt");
    check(missing.load() && missing.size() == 0,
          "load: a missing file gives an empty list");

    {
        ofstream out(tempPath, ios::trunc);
        out << "ali|1234|Ali Rezaei|120|2|5\n";
        out << "broken line\n";
        out << "sara|abcd|Sara|x|1|1\n";
        out << "\n";
        out << "ali|9999|Duplicate|1|1|1\n";
        out << "reza|pass|Reza|50|0|3\r\n";
        out << "a b|pass|Space|1|1|1\n";
        out << "tooMany|pass|X|1|1|1|extra\n";
    }

    AccountManager manager(tempPath);
    check(manager.load() && manager.size() == 2,
          "corrupt: only the 2 valid lines are loaded");
    check(manager.getSkippedLines() == 5,
          "corrupt: 5 bad lines are skipped (empty lines are ignored)");
    check(manager.find("ali") != nullptr &&
          manager.find("ali")->getPassword() == "1234" &&
          manager.find("reza") != nullptr,
          "corrupt: the first duplicate wins and CRLF lines work");

    {
        ofstream out(tempPath, ios::trunc);
    }
}

static void testRanking() {
    AccountManager manager("");
    manager.registerAccount("a", "1111", "A");
    manager.registerAccount("b", "1111", "B");
    manager.registerAccount("c", "1111", "C");
    manager.registerAccount("d", "1111", "D");
    manager.registerAccount("e", "1111", "E");

    manager.find("a")->recordGame(100, true);

    manager.find("b")->recordGame(40, true);
    manager.find("b")->recordGame(30, true);
    manager.find("b")->recordGame(30, true);

    manager.find("c")->recordGame(50, false);

    manager.find("d")->recordGame(40, true);
    manager.find("d")->recordGame(30, true);
    manager.find("d")->recordGame(30, true);

    DynamicArray<Account*> ranked;
    manager.ranked(ranked);

    string order = "";

    for (int i = 0; i < ranked.size(); ++i) {
        Account* account = nullptr;
        ranked.get(i, account);
        order += account->getUsername();
    }

    check(order == "bdace",
          "ranking: higher score first, then more wins, then registration order");
    check(manager.getAt(0)->getUsername() == "a" &&
          manager.getAt(1)->getUsername() == "b",
          "ranking: the stored order is not changed");

    AccountManager empty("");
    DynamicArray<Account*> none;
    empty.ranked(none);
    check(none.size() == 0, "ranking: an empty manager gives an empty list");
}

int main() {
    cout << "--- Register rules ---\n";
    testRegisterRules();

    cout << "\n--- Login and find ---\n";
    testLoginAndFind();

    cout << "\n--- Pointer stability ---\n";
    testPointerStability();

    cout << "\n--- Save and load ---\n";
    testSaveAndLoad();

    cout << "\n--- Missing and corrupt files ---\n";
    testMissingAndCorruptFiles();

    cout << "\n--- Ranking ---\n";
    testRanking();

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}