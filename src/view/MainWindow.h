#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

// Top-level window: hosts the menu and game screens and the application
// menu bar. Owns the GuiController (application layer) which in turn works
// exclusively through the existing engine classes.

#include <QMainWindow>

#include "../logic/AccountManager.h"

class QStackedWidget;

class GameWidget;
class GuiController;
class MenuWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(AccountManager& accounts, QWidget* parent = nullptr);

    // Access for tests / embedding.
    GuiController* controller() const { return controller_; }

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildMenuBar();
    void updateActionStates();

    AccountManager& accounts_;
    GuiController* controller_;
    QStackedWidget* pages_;
    MenuWidget* menu_;
    GameWidget* gameWidget_;
};

#endif
