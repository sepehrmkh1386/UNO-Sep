#include <cstdlib>

#include "app/AppController.h"
#include "logic/AccountManager.h"
#include "logic/Random.h"
#include "logic/TextUtils.h"

using namespace std;

int main(int argc, char* argv[]) {
    int seed = 0;

    if (argc > 1 && parseNonNegativeInt(argv[1], seed)) {
        srand(static_cast<unsigned int>(seed));
    } else {
        seedRandom();
    }

    AccountManager accounts("accounts.txt");
    AppController app(accounts);
    app.run();

    return 0;
}