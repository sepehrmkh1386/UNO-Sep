#ifndef MENU_WIDGET_H
#define MENU_WIDGET_H

// Main menu screen: account login/registration, game setup (human and bot
// players, win condition) and the leaderboard.
// It calls the application layer (GuiController) for every account or game
// operation and never touches the engine directly.

#include <QWidget>

#include "../logic/ScoreManager.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTextBrowser;
class QVBoxLayout;

class GuiController;

class MenuWidget : public QWidget {
    Q_OBJECT

public:
    explicit MenuWidget(QWidget* parent = nullptr);
    void setController(GuiController* controller);

    void refreshIdentity();
    void setLeaderboardText(const QString& text);
    void showAuthError(const QString& message);

signals:
    void quitRequested();

private slots:
    void handleStartGame();

private:
    void buildAuthPage();
    void buildLoggedInPage();
    void buildSetupCard(QVBoxLayout* rightColumn);

    GuiController* controller;

    QStackedWidget* identityStack;

    QLineEdit* loginUser;
    QLineEdit* loginPass;
    QLineEdit* regUser;
    QLineEdit* regPass;
    QLineEdit* regDisplay;
    QPushButton* loginButton;
    QPushButton* registerButton;
    QLabel* authErrorLabel;

    QLabel* welcomeLabel;
    QPushButton* logoutButton;
    QPushButton* quitButton;

    QSpinBox* humansSpin;
    QSpinBox* botsSpin;
    QSpinBox* limitSpin;
    QComboBox* modeCombo;
    QComboBox* extraUser[3];
    QLabel* extraLabel[3];
    QPushButton* startButton;
    QLabel* setupErrorLabel;

    QTextBrowser* leaderboardView;
};

#endif
