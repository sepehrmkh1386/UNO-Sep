#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <string>

#include "../ds/DynamicArray.h"
#include "../model/Account.h"

using namespace std;

enum class RegisterResult
{
    Ok,
    InvalidUsername,
    UsernameTaken,
    WeakPassword,
    InvalidDisplayName,
    ForbiddenCharacter
};

class AccountManager
{
    private:
    DynamicArray<Account*> accounts;
    string filePath;
    int skippedLines;

    int indexOf(const string& username) const;
    bool addLoaded(const string& line);
    void clearAll();

    public:
    AccountManager(const string& path);
    ~AccountManager();

    AccountManager(const AccountManager&) = delete;
    AccountManager& operator=(const AccountManager&) = delete;

    RegisterResult registerAccount(const string& username,
                                    const string& password,
                                    const string& desplayNam);
    Account* login(const string& username, const string& password) const;
    Account* find(const string& username) const;

    int size() const;
    Account* getAt(int index) const;
    int getSkippedLines() const;

    void ranked(DynamicArray<Account*>& result) const;

    bool save() const;
    bool load();
};

#endif
