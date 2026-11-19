#include "MainWindow.h"

#include <QCloseEvent>
#include <QMenuBar>
#include <QMessageBox>
#include <QStackedWidget>

#include "../app/GuiController.h"
#include "GameWidget.h"
#include "MenuWidget.h"
#include "Theme.h"

MainWindow::MainWindow(AccountManager& accounts, QWidget* parent)
    : QMainWindow(parent), accounts_(accounts), controller_(nullptr) {
    setWindowTitle("UNO — Data Structures Project (Qt GUI)");
    setMinimumSize(1080, 760);
    resize(1180, 800);
    setStyleSheet(UnoTheme::appStyleSheet());

    controller_ = new GuiController(accounts_, this);

    pages_ = new QStackedWidget(this);
    setCentralWidget(pages_);

    menu_ = new MenuWidget(pages_);
    gameWidget_ = new GameWidget(controller_, pages_);

    pages_->addWidget(menu_);
    pages_->addWidget(gameWidget_);
    pages_->setCurrentWidget(menu_);

    menu_->setController(controller_);

    buildMenuBar();

    connect(controller_, &GuiController::gameStarted, this, [this]() {
        pages_->setCurrentWidget(gameWidget_);
        updateActionStates();
    });

    connect(controller_, &GuiController::gameClosed, this, [this]() {
        pages_->setCurrentWidget(menu_);
        menu_->refreshIdentity();
        menu_->setLeaderboardText(controller_->leaderboardText());
        updateActionStates();
    });

    connect(controller_, &GuiController::accountsChanged, this, [this]() {
        menu_->setLeaderboardText(controller_->leaderboardText());
        updateActionStates();
    });

    updateActionStates();
}

void MainWindow::buildMenuBar() {
    QMenu* gameMenu = menuBar()->addMenu("&Game");

    QAction* backAction =
        gameMenu->addAction("Back to &menu (abandon game)");
    connect(backAction, &QAction::triggered, this, [this]() {
        if (controller_ != nullptr && controller_->gameActive()) {
            controller_->leaveGame();
        }
    });

    QAction* exitAction = gameMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    QMenu* helpMenu = menuBar()->addMenu("&Help");

    QAction* helpAction = helpMenu->addAction("&How to play");
    connect(helpAction, &QAction::triggered, this, [this]() {
        QMessageBox box(this);
        box.setWindowTitle("How to play");
        box.setTextFormat(Qt::RichText);
        box.setText(
            "<b>Goal</b><br>Be the first to empty your hand. The round "
            "winner scores the total value of every other player's hand "
            "(computed by the engine's recursive algorithm).<br><br>"
            "<b>Your turn</b><br>"
            "• Click a card to select it, then press <i>Play selected "
            "card</i>.<br>"
            "• A card is playable when it matches the active color, matches "
            "the top card's number or symbol, or is a Wild. Wilds ask for a "
            "color.<br>"
            "• <b>Stacking:</b> while a +2/+4 penalty is pending you may "
            "only play +2/+4 cards to stack it.<br>"
            "• <b>Chance machine:</b> instead of accepting a pending "
            "penalty, spin the Device of Chance for a weighted random "
            "outcome.<br>"
            "• <i>Draw a card</i> — if the drawn card is playable you may "
            "play it or pass; otherwise your turn ends automatically."
            "<br><br><b>Bots</b><br>Robot players are driven entirely by the "
            "existing engine logic.");
        box.exec();
    });

    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::information(
            this, "About",
            "UNO — Data Structures Project (Qt GUI)\n\n"
            "Qt Widgets presentation layer on top of the existing UNO game "
            "engine: custom data structures, rule manager, turn manager, "
            "deck/discard piles, bot logic, scoring and accounts are the "
            "original engine code.");
    });
}

void MainWindow::updateActionStates() {
    QMenu* gameMenu = menuBar()->actions().value(0)->menu();

    if (gameMenu == nullptr || gameMenu->actions().isEmpty()) {
        return;
    }

    QAction* backAction = gameMenu->actions().value(0);
    backAction->setEnabled(controller_ != nullptr &&
                           controller_->gameActive());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (controller_ != nullptr && controller_->gameActive()) {
        QMessageBox::StandardButton answer = QMessageBox::question(
            this, "Leave game",
            "A game is in progress. Leave without saving results?");

        if (answer != QMessageBox::Yes) {
            event->ignore();
            return;
        }

        controller_->leaveGame();
    }

    event->accept();
}
