
#include <cstdlib>
#include <QApplication>

#include "logic/AccountManager.h"
#include "logic/Random.h"
#include "logic/TextUtils.h"
#include "view/MainWindow.h"

using namespace std;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    int seed = 0;

    if (argc > 1 && parseNonNegativeInt(argv[1], seed)) {
        srand(static_cast<unsigned int>(seed));
    } else {
        seedRandom();
    }

    AccountManager accounts("accounts.txt");

    MainWindow window(accounts);
    window.show();

    return app.exec();
}
