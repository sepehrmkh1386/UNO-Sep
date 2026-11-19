#include "MenuWidget.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "../app/GuiController.h"
#include "Theme.h"

MenuWidget::MenuWidget(QWidget* parent)
    : QWidget(parent), controller(nullptr) {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(36, 24, 36, 24);
    root->setSpacing(16);

    QLabel* title = new QLabel("UNO", this);
    title->setObjectName("TitleLabel");
    title->setAlignment(Qt::AlignHCenter);

    QLabel* subtitle = new QLabel("Data Structures Project — Qt Edition",
                                  this);
    subtitle->setObjectName("SubtitleLabel");
    subtitle->setAlignment(Qt::AlignHCenter);

    root->addWidget(title);
    root->addWidget(subtitle);

    QHBoxLayout* columns = new QHBoxLayout();
    columns->setSpacing(24);

    // Left column: identity (login/register or logged-in panel).
    identityStack = new QStackedWidget(this);
    identityStack->setMinimumWidth(330);
    buildAuthPage();
    buildLoggedInPage();
    columns->addWidget(identityStack);

    // Right column: game setup + leaderboard.
    QVBoxLayout* rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(16);
    buildSetupCard(rightColumn);
    columns->addLayout(rightColumn, 1);

    root->addLayout(columns, 1);

    refreshIdentity();
}

void MenuWidget::setController(GuiController* newController) {
    controller = newController;

    if (controller == nullptr) {
        return;
    }

    connect(controller, &GuiController::errorRaised, this,
            &MenuWidget::showAuthError);
    connect(controller, &GuiController::accountsChanged, this,
            &MenuWidget::refreshIdentity);

    refreshIdentity();
    setLeaderboardText(controller->leaderboardText());
}

void MenuWidget::buildAuthPage() {
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    QLabel* loginCaption = new QLabel("Log in", page);
    loginCaption->setObjectName("SectionLabel");

    loginUser = new QLineEdit(page);
    loginUser->setPlaceholderText("Username");

    loginPass = new QLineEdit(page);
    loginPass->setPlaceholderText("Password");
    loginPass->setEchoMode(QLineEdit::Password);

    loginButton = new QPushButton("Log in", page);
    loginButton->setObjectName("PrimaryButton");

    layout->addWidget(loginCaption);
    layout->addWidget(loginUser);
    layout->addWidget(loginPass);
    layout->addWidget(loginButton);

    QLabel* registerCaption = new QLabel("Create account", page);
    registerCaption->setObjectName("SectionLabel");

    regUser = new QLineEdit(page);
    regUser->setPlaceholderText("Username");

    regPass = new QLineEdit(page);
    regPass->setPlaceholderText("Password");

    regDisplay = new QLineEdit(page);
    regDisplay->setPlaceholderText("Display name (optional)");

    registerButton = new QPushButton("Register", page);

    layout->addWidget(registerCaption);
    layout->addWidget(regUser);
    layout->addWidget(regPass);
    layout->addWidget(regDisplay);
    layout->addWidget(registerButton);

    authErrorLabel = new QLabel("", page);
    authErrorLabel->setStyleSheet("color: #ff8a80;");
    authErrorLabel->setWordWrap(true);

    layout->addWidget(authErrorLabel);
    layout->addStretch();

    connect(loginButton, &QPushButton::clicked, this, [this]() {
        showAuthError("");
        if (controller != nullptr) {
            controller->login(loginUser->text().toStdString(),
                              loginPass->text().toStdString());
        }
    });

    connect(registerButton, &QPushButton::clicked, this, [this]() {
        showAuthError("");
        if (controller != nullptr) {
            controller->registerAccount(regUser->text().toStdString(),
                                        regPass->text().toStdString(),
                                        regDisplay->text().toStdString());
        }
    });

    identityStack->addWidget(page);
}

void MenuWidget::buildLoggedInPage() {
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

    QLabel* caption = new QLabel("Account", page);
    caption->setObjectName("SectionLabel");

    welcomeLabel = new QLabel("", page);
    welcomeLabel->setWordWrap(true);

    layout->addWidget(caption);
    layout->addWidget(welcomeLabel);
    layout->addStretch();

    logoutButton = new QPushButton("Log out", page);
    quitButton = new QPushButton("Quit application", page);
    quitButton->setObjectName("DangerButton");

    layout->addWidget(logoutButton);
    layout->addWidget(quitButton);

    connect(logoutButton, &QPushButton::clicked, this, [this]() {
        if (controller != nullptr) {
            controller->logout();
        }
    });

    connect(quitButton, &QPushButton::clicked, this,
            &MenuWidget::quitRequested);

    identityStack->addWidget(page);
}

void MenuWidget::buildSetupCard(QVBoxLayout* rightColumn) {
    QFrame* setupFrame = new QFrame(this);
    setupFrame->setObjectName("PanelFrame");

    QVBoxLayout* setupLayout = new QVBoxLayout(setupFrame);
    setupLayout->setContentsMargins(22, 18, 22, 18);
    setupLayout->setSpacing(12);

    QLabel* caption = new QLabel("New game", setupFrame);
    caption->setObjectName("SectionLabel");
    setupLayout->addWidget(caption);

    QGridLayout* grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(10);

    humansSpin = new QSpinBox(setupFrame);
    humansSpin->setRange(1, 4);
    humansSpin->setValue(1);

    botsSpin = new QSpinBox(setupFrame);
    botsSpin->setRange(0, 8);
    botsSpin->setValue(3);

    modeCombo = new QComboBox(setupFrame);
    modeCombo->addItem("First player to reach the score limit wins");
    modeCombo->addItem("Play a fixed number of rounds");

    limitSpin = new QSpinBox(setupFrame);
    limitSpin->setRange(20, 500);
    limitSpin->setValue(100);

    grid->addWidget(new QLabel("Human players (including you)", setupFrame),
                    0, 0);
    grid->addWidget(humansSpin, 0, 1);

    for (int i = 0; i < 3; ++i) {
        extraLabel[i] =
            new QLabel(QString("Account of human player %1").arg(i + 2),
                       setupFrame);
        extraUser[i] = new QComboBox(setupFrame);
        extraLabel[i]->setVisible(false);
        extraUser[i]->setVisible(false);

        grid->addWidget(extraLabel[i], 1 + i, 0);
        grid->addWidget(extraUser[i], 1 + i, 1);
    }

    grid->addWidget(new QLabel("Robot players", setupFrame), 4, 0);
    grid->addWidget(botsSpin, 4, 1);

    grid->addWidget(new QLabel("Win condition", setupFrame), 5, 0);
    grid->addWidget(modeCombo, 5, 1);

    grid->addWidget(new QLabel("Limit (points or rounds)", setupFrame), 6, 0);
    grid->addWidget(limitSpin, 6, 1);

    setupLayout->addLayout(grid);

    connect(modeCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index == 0) {
            limitSpin->setRange(20, 500);
            limitSpin->setValue(100);
        } else {
            limitSpin->setRange(1, 10);
            limitSpin->setValue(3);
        }
    });

    connect(humansSpin, &QSpinBox::valueChanged, this, [this](int value) {
        for (int i = 0; i < 3; ++i) {
            bool visible = (i + 2) <= value;
            extraLabel[i]->setVisible(visible);
            extraUser[i]->setVisible(visible);
        }
    });

    startButton = new QPushButton("Start game", setupFrame);
    startButton->setObjectName("PrimaryButton");
    connect(startButton, &QPushButton::clicked, this,
            &MenuWidget::handleStartGame);

    setupErrorLabel = new QLabel("", setupFrame);
    setupErrorLabel->setStyleSheet("color: #ff8a80;");
    setupErrorLabel->setWordWrap(true);

    setupLayout->addWidget(startButton);
    setupLayout->addWidget(setupErrorLabel);

    rightColumn->addWidget(setupFrame);

    QLabel* boardCaption = new QLabel("Leaderboard", this);
    boardCaption->setObjectName("SectionLabel");

    leaderboardView = new QTextBrowser(this);
    leaderboardView->setReadOnly(true);
    leaderboardView->setMinimumHeight(130);

    rightColumn->addWidget(boardCaption);
    rightColumn->addWidget(leaderboardView, 1);
}

void MenuWidget::handleStartGame() {
    if (controller == nullptr) {
        return;
    }

    setupErrorLabel->setText("");

    int humans = humansSpin->value();
    DynamicArray<std::string> extras;

    for (int i = 0; i < humans - 1; ++i) {
        if (extraUser[i]->currentText().trimmed().isEmpty()) {
            setupErrorLabel->setText(
                QString("Choose an account for human player %1, or reduce "
                        "the number of human players.")
                    .arg(i + 2));
            return;
        }

        extras.add(extraUser[i]->currentText().trimmed().toStdString());
    }

    GameMode mode = (modeCombo->currentIndex() == 0) ? GameMode::Score
                                                     : GameMode::Rounds;

    controller->startGame(humans, extras, botsSpin->value(), mode,
                          limitSpin->value());
}

void MenuWidget::refreshIdentity() {
    if (controller == nullptr || controller->currentAccount() == nullptr) {
        identityStack->setCurrentIndex(0);
        loginUser->clear();
        loginPass->clear();
        regUser->clear();
        regPass->clear();
        regDisplay->clear();
        showAuthError("");

        for (int i = 0; i < 3; ++i) {
            extraUser[i]->clear();
        }

        return;
    }

    identityStack->setCurrentIndex(1);
    welcomeLabel->setText(
        QString("Welcome, %1!\n\nStart a new game on the right. Robot "
                "players are handled by the existing game engine.")
            .arg(QString::fromStdString(
                controller->currentAccount()->getDisplayName())));

    // Populate the extra-human account combo boxes from AccountManager.
    QString self =
        QString::fromStdString(controller->currentAccount()->getUsername());

    for (int i = 0; i < 3; ++i) {
        extraUser[i]->clear();
    }

    for (int i = 0; i < controller->accountCount(); ++i) {
        const Account* account = controller->accountAt(i);
        QString username =
            QString::fromStdString(account->getUsername());

        if (username == self) {
            continue;
        }

        for (int c = 0; c < 3; ++c) {
            extraUser[c]->addItem(username);
        }
    }

    humansSpin->setMaximum(
        qMin(4, 1 + controller->accountCount() - 1));
}

void MenuWidget::setLeaderboardText(const QString& text) {
    leaderboardView->setPlainText(text);
}

void MenuWidget::showAuthError(const QString& message) {
    authErrorLabel->setText(message);
    setupErrorLabel->setText("");
}
