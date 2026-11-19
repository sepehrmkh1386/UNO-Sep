#include "GameWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

#include "../app/GuiController.h"
#include "CardWidget.h"
#include "PlayerPanel.h"
#include "Theme.h"

#include "dialogs/ColorPickDialog.h"

GameWidget::GameWidget(GuiController* controller, QWidget* parent)
    : QWidget(parent),
      controller_(controller),
      selectedCardIndex_(-1),
      shownIsCurrentPlayer_(false) {
    buildUi();

    if (controller_ != nullptr) {
        connect(controller_, &GuiController::tableChanged, this,
                &GameWidget::refreshFromEngine);
        connect(controller_, &GuiController::statusLine, statusLabel,
                &QLabel::setText);
        connect(controller_, &GuiController::logLine, logView,
                &QTextBrowser::append);
        connect(controller_, &GuiController::needStartColorChoice, this,
                [this]() { QTimer::singleShot(150, this, &GameWidget::askStartColor); });
        connect(controller_, &GuiController::roundOverReady, this,
                [this]() { QTimer::singleShot(700, this, &GameWidget::showRoundOverDialog); });
        connect(controller_, &GuiController::gameOverReady, this,
                [this]() { QTimer::singleShot(900, this, &GameWidget::showGameOverDialog); });
    }
}

void GameWidget::buildUi() {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 12, 24, 14);
    root->setSpacing(10);

    // --- Top bar ---
    QHBoxLayout* topBar = new QHBoxLayout();
    topBar->setSpacing(18);

    roundLabel = new QLabel("Round", this);
    roundLabel->setObjectName("CenterInfoLabel");

    winLabel = new QLabel("", this);
    winLabel->setObjectName("CenterInfoLabel");

    directionLabel = new QLabel("", this);
    directionLabel->setObjectName("CenterInfoLabel");

    topBar->addWidget(roundLabel);
    topBar->addWidget(winLabel);
    topBar->addStretch();
    topBar->addWidget(directionLabel);

    root->addLayout(topBar);

    // --- Opponents row (horizontal strip of seat panels) ---
    opponentsHost = new QWidget(this);
    opponentsLayout = new QHBoxLayout(opponentsHost);
    opponentsLayout->setContentsMargins(0, 0, 0, 0);
    opponentsLayout->setSpacing(10);

    opponentsHost->setFixedHeight(112);

    root->addWidget(opponentsHost);

    // --- Table center ---
    QHBoxLayout* tableRow = new QHBoxLayout();
    tableRow->setSpacing(26);

    QVBoxLayout* deckColumn = new QVBoxLayout();
    deckColumn->setSpacing(4);

    deckCard = new CardWidget(this);
    deckCard->setFaceDown(true);
    deckCard->setEnabled(false);

    deckLabel = new QLabel("Deck", this);
    deckLabel->setAlignment(Qt::AlignHCenter);
    deckLabel->setObjectName("CenterInfoLabel");

    deckColumn->addWidget(deckCard, 0, Qt::AlignHCenter);
    deckColumn->addWidget(deckLabel);
    deckColumn->addStretch();
    tableRow->addLayout(deckColumn);

    QVBoxLayout* discardColumn = new QVBoxLayout();
    discardColumn->setSpacing(4);

    discardCard = new CardWidget(this);
    discardCard->setEnabled(false);

    activeColorChip = new QLabel("Active color", this);
    activeColorChip->setAlignment(Qt::AlignHCenter);
    activeColorChip->setObjectName("CenterInfoLabel");

    discardColumn->addWidget(discardCard, 0, Qt::AlignHCenter);
    discardColumn->addWidget(activeColorChip);
    discardColumn->addStretch();
    tableRow->addLayout(discardColumn);

    QVBoxLayout* infoColumn = new QVBoxLayout();
    infoColumn->setSpacing(8);

    statusLabel = new QLabel("Welcome to UNO!", this);
    statusLabel->setObjectName("StatusMessage");
    statusLabel->setWordWrap(true);

    penaltyLabel = new QLabel("", this);
    penaltyLabel->setObjectName("PenaltyLabel");
    penaltyLabel->setWordWrap(true);

    localSeatLabel = new QLabel("", this);
    localSeatLabel->setObjectName("PlayerSubLabel");
    localSeatLabel->setWordWrap(true);

    infoColumn->addStretch();
    infoColumn->addWidget(statusLabel);
    infoColumn->addWidget(penaltyLabel);
    infoColumn->addWidget(localSeatLabel);
    infoColumn->addStretch();

    tableRow->addLayout(infoColumn, 1);

    root->addLayout(tableRow);

    // --- Hand area ---
    handTitle = new QLabel("Your hand", this);
    handTitle->setObjectName("SectionLabel");

    handHost = new QWidget(this);
    handRowLayout = new QHBoxLayout(handHost);
    handRowLayout->setContentsMargins(2, 6, 2, 6);
    handRowLayout->setSpacing(8);
    handRowLayout->addStretch();

    handScroll = new QScrollArea(this);
    handScroll->setWidgetResizable(true);
    handScroll->setWidget(handHost);
    handScroll->setFixedHeight(175);
    handScroll->setAlignment(Qt::AlignHCenter);

    root->addWidget(handTitle);
    root->addWidget(handScroll);

    // --- Action buttons ---
    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->setSpacing(10);

    playSelectedButton = new QPushButton("Play selected card", this);
    playDrawnButton = new QPushButton("Play drawn card", this);
    drawButton = new QPushButton("Draw a card", this);
    passButton = new QPushButton("Pass", this);
    chanceButton = new QPushButton("Chance machine", this);

    chanceButton->setToolTip("Device of Chance: replaces a pending +2/+4 "
                             "penalty with a weighted random draw.");

    buttons->addWidget(playSelectedButton);
    buttons->addWidget(playDrawnButton);
    buttons->addWidget(drawButton);
    buttons->addWidget(passButton);
    buttons->addWidget(chanceButton);
    buttons->addStretch();

    connect(playSelectedButton, &QPushButton::clicked, this,
            &GameWidget::onPlaySelected);
    connect(playDrawnButton, &QPushButton::clicked, this,
            &GameWidget::onPlayDrawn);
    connect(drawButton, &QPushButton::clicked, this, &GameWidget::onDraw);
    connect(passButton, &QPushButton::clicked, this, &GameWidget::onPass);
    connect(chanceButton, &QPushButton::clicked, this,
            &GameWidget::onChance);

    root->addLayout(buttons);

    // --- Event log ---
    logView = new QTextBrowser(this);
    logView->setReadOnly(true);
    logView->setFixedHeight(95);
    logView->setPlaceholderText("Game events will appear here...");

    root->addWidget(logView);

    refreshFromEngine();
}

Player* GameWidget::shownPlayer(int& seatOut) const {
    seatOut = 0;

    if (!controller_->gameActive()) {
        return nullptr;
    }

    Game* game = controller_->game();
    Round* round = game->getRound();
    Player* current = round->getCurrentPlayer();

    if (current != nullptr && current->isHuman()) {
        seatOut = -1;

        // Identify which human seat is on turn.
        for (int i = 0; i < game->getPlayerCount(); ++i) {
            if (game->getPlayerAt(i) == current) {
                seatOut = i;
                break;
            }
        }

        return current;
    }

    // Between bot turns show the local (seat 0) hand, dimmed.
    return game->getPlayerAt(0);
}

void GameWidget::rebuildOpponents() {
    QLayoutItem* item;

    while ((item = opponentsLayout->takeAt(0)) != nullptr) {
        if (item->widget() != nullptr) {
            // Hide immediately so a pending deferred delete never leaves a
            // stale widget visible on top of the rebuilt row.
            item->widget()->hide();
            item->widget()->deleteLater();
        }

        delete item;
    }

    if (!controller_->gameActive()) {
        opponentsHost->setVisible(false);
        return;
    }

    opponentsHost->setVisible(true);

    Game* game = controller_->game();
    Round* round = game->getRound();
    Player* current = round->getCurrentPlayer();

    for (int i = 1; i < game->getPlayerCount(); ++i) {
        Player* player = game->getPlayerAt(i);
        PlayerPanel* panel = new PlayerPanel(opponentsHost);

        panel->updateInfo(QString::fromStdString(player->getName()),
                          player->isHuman(), player->getHandSize(),
                          player == current, false, QString());

        opponentsLayout->addWidget(panel);
    }

    opponentsLayout->addStretch();
}

void GameWidget::clearHandRow() {
    QLayoutItem* item;

    while ((item = handRowLayout->takeAt(0)) != nullptr) {
        if (item->widget() != nullptr) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }

        delete item;
    }

    while (!handWidgets_.isEmpty()) {
        handWidgets_.removeLast();
    }
}

void GameWidget::rebuildHand(int shownSeat) {
    clearHandRow();

    if (!controller_->gameActive() || shownSeat < 0) {
        return;
    }

    Game* game = controller_->game();
    Round* round = game->getRound();
    Player* player = game->getPlayerAt(shownSeat);

    if (player == nullptr) {
        return;
    }

    Player* current = round->getCurrentPlayer();
    bool interactive = (player == current && current->isHuman() &&
                        !round->needsStartColor());
    bool drawnPlayable = round->hasDrawnCard();

    for (int i = 0; i < player->getHandSize(); ++i) {
        Card card;
        player->getCard(i, card);

        CardWidget* widget = new CardWidget(handHost);
        widget->setCard(card);

        bool playable = interactive && round->canPlayCard(i);

        if (interactive && drawnPlayable && i == player->getHandSize() - 1) {
            playable = true;
        }

        widget->setPlayable(playable);
        widget->setSelected(selectedCardIndex_ == i);

        int index = i;
        connect(widget, &CardWidget::clicked, this,
                [this, index]() { onCardClicked(index); });

        handWidgets_.add(widget);
        handRowLayout->addWidget(widget);
    }

    handRowLayout->addStretch();
}

void GameWidget::refreshFromEngine() {
    if (!controller_->gameActive()) {
        roundLabel->setText("Round");
        winLabel->setText("");
        directionLabel->setText("");
        statusLabel->setText("No game in progress.");
        penaltyLabel->setText("");
        localSeatLabel->setText("");
        handTitle->setText("Hand");
        deckLabel->setText("Deck");
        activeColorChip->setText("Active color");
        activeColorChip->setStyleSheet("");

        rebuildOpponents();
        clearHandRow();
        updateButtons();
        return;
    }

    Game* game = controller_->game();
    Round* round = game->getRound();
    Player* current = round->getCurrentPlayer();

    // Top bar.
    QString roundText = "Round " + QString::number(game->getRoundsPlayed() + 1);

    if (game->getMode() == GameMode::Rounds) {
        roundText += " of " + QString::number(game->getLimit());
    }

    roundLabel->setText(roundText);
    winLabel->setText(controller_->winConditionText());
    directionLabel->setText(round->isClockwise()
                                ? "Direction: clockwise →"
                                : "Direction: ← counter-clockwise");

    // Center piles.
    Card top;

    if (round->getTopCard(top)) {
        discardCard->setCard(top);
    }

    deckLabel->setText("Deck: " + QString::number(round->getDeckSize()) +
                       "   •   Discard pile: " +
                       QString::number(round->getDiscardSize()));

    CardColor active = round->getActiveColor();

    if (active == CardColor::None) {
        activeColorChip->setText("Active color: —");
        activeColorChip->setStyleSheet("");
    } else {
        activeColorChip->setText("Active color: " +
                                 UnoTheme::colorName(active));
        activeColorChip->setStyleSheet(
            QString("color: white; background: %1; border-radius: 10px; "
                    "padding: 4px 12px; font-weight: 700;")
                .arg(UnoTheme::cardColorQ(active).name()));
    }

    int pending = round->getPendingPenalty();

    if (pending > 0) {
        penaltyLabel->setText(
            QString("Pending penalty: %1 %2 — play a +2/+4 to stack, use "
                    "the chance machine, or draw to accept.")
                .arg(pending)
                .arg(pending == 1 ? "card" : "cards"));
    } else {
        penaltyLabel->setText("");
    }

    // Opponents and hand.
    rebuildOpponents();

    int shownSeat = 0;
    Player* shown = shownPlayer(shownSeat);
    shownIsCurrentPlayer_ =
        (shown != nullptr && current != nullptr && shown == current &&
         current->isHuman());

    if (shown != nullptr) {
        QString who = QString::fromStdString(shown->getName());

        handTitle->setText("Hand of " + who +
                           (shownSeat == 0 ? "  (you)" : ""));

        localSeatLabel->setText(
            QString::fromStdString(game->getPlayerAt(0)->getName()) +
            " — your seat — " +
            QString::number(game->getPlayerAt(0)->getHandSize()) +
            " cards");
    } else {
        handTitle->setText("Hand");
        localSeatLabel->setText("");
    }

    rebuildHand(shownSeat);
    updateButtons();
}

void GameWidget::updateButtons() {
    bool humanTurn = false;
    bool startColorPending = false;
    bool hasDrawn = false;
    bool penaltyPending = false;
    bool selectedPlayable = false;

    if (controller_->gameActive()) {
        Round* round = controller_->game()->getRound();
        Player* current = round->getCurrentPlayer();

        humanTurn = (current != nullptr && current->isHuman());
        startColorPending = round->needsStartColor();
        hasDrawn = round->hasDrawnCard();
        penaltyPending = round->getPendingPenalty() > 0;

        if (humanTurn && !startColorPending && !hasDrawn &&
            selectedCardIndex_ >= 0) {
            selectedPlayable = round->canPlayCard(selectedCardIndex_);
        }
    }

    playSelectedButton->setEnabled(selectedPlayable);
    playDrawnButton->setEnabled(humanTurn && !startColorPending && hasDrawn);
    drawButton->setEnabled(humanTurn && !startColorPending && !hasDrawn);
    passButton->setEnabled(humanTurn && !startColorPending && hasDrawn);
    chanceButton->setEnabled(humanTurn && !startColorPending &&
                             penaltyPending);
}

void GameWidget::onCardClicked(int cardIndex) {
    if (!controller_->gameActive() || !shownIsCurrentPlayer_) {
        return;
    }

    Round* round = controller_->game()->getRound();
    Player* current = round->getCurrentPlayer();

    if (current == nullptr || round->needsStartColor()) {
        return;
    }

    if (round->hasDrawnCard()) {
        if (cardIndex != current->getHandSize() - 1) {
            return;
        }

        onPlayDrawn();
        return;
    }

    Card card;

    if (!current->getCard(cardIndex, card)) {
        return;
    }

    if (card.isWildCard()) {
        CardColor color =
            ColorPickDialog::pick(this, "Choose a color for the Wild card");

        if (color == CardColor::None) {
            return;
        }

        controller_->playCard(cardIndex, color);
        selectedCardIndex_ = -1;
        return;
    }

    if (!round->canPlayCard(cardIndex)) {
        // Let the engine reject the move and report the reason.
        controller_->playCard(cardIndex, CardColor::None);
        selectedCardIndex_ = -1;
        return;
    }

    selectedCardIndex_ = cardIndex;
    refreshFromEngine();
}

void GameWidget::onPlaySelected() {
    if (!controller_->gameActive() || !shownIsCurrentPlayer_ ||
        selectedCardIndex_ < 0) {
        return;
    }

    Round* round = controller_->game()->getRound();
    Player* current = round->getCurrentPlayer();

    if (current == nullptr || !current->isHuman()) {
        return;
    }

    Card card;

    if (!current->getCard(selectedCardIndex_, card)) {
        selectedCardIndex_ = -1;
        refreshFromEngine();
        return;
    }

    if (card.isWildCard()) {
        CardColor color =
            ColorPickDialog::pick(this, "Choose a color for the Wild card");

        if (color == CardColor::None) {
            return;
        }

        controller_->playCard(selectedCardIndex_, color);
    } else {
        controller_->playCard(selectedCardIndex_, CardColor::None);
    }

    selectedCardIndex_ = -1;
}

void GameWidget::onPlayDrawn() {
    if (!controller_->gameActive() || !shownIsCurrentPlayer_) {
        return;
    }

    Round* round = controller_->game()->getRound();
    Player* current = round->getCurrentPlayer();

    if (current == nullptr || !current->isHuman() ||
        !round->hasDrawnCard()) {
        return;
    }

    Card card;

    if (!current->getCard(current->getHandSize() - 1, card)) {
        return;
    }

    if (card.isWildCard()) {
        CardColor color =
            ColorPickDialog::pick(this, "Choose a color for the Wild card");

        if (color == CardColor::None) {
            return;
        }

        controller_->playDrawn(color);
        return;
    }

    controller_->playDrawn(CardColor::None);
    selectedCardIndex_ = -1;
}

void GameWidget::onDraw() {
    selectedCardIndex_ = -1;
    controller_->drawCard();
}

void GameWidget::onPass() {
    selectedCardIndex_ = -1;
    controller_->passTurn();
}

void GameWidget::onChance() {
    selectedCardIndex_ = -1;
    controller_->useChance();
}

void GameWidget::askStartColor() {
    if (!controller_->gameActive() ||
        !controller_->game()->getRound()->needsStartColor()) {
        return;
    }

    CardColor color = ColorPickDialog::pick(
        this, "The first card is a Wild card — choose the starting color");

    if (color == CardColor::None) {
        QTimer::singleShot(120, this, &GameWidget::askStartColor);
        return;
    }

    controller_->chooseStartColor(color);
}

void GameWidget::showRoundOverDialog() {
    if (!controller_->gameActive() ||
        !controller_->game()->getRound()->isOver()) {
        return;
    }

    QMessageBox box(this);
    box.setWindowTitle("Round finished");
    box.setTextFormat(Qt::RichText);
    box.setText(controller_->roundSummaryText());

    QPushButton* nextButton =
        box.addButton("Next round", QMessageBox::AcceptRole);
    QPushButton* leaveButton =
        box.addButton("Leave game", QMessageBox::DestructiveRole);

    box.exec();

    if (box.clickedButton() == leaveButton) {
        controller_->leaveGame();
        return;
    }

    if (box.clickedButton() == nextButton) {
        controller_->nextRound();
    }
}

void GameWidget::showGameOverDialog() {
    if (controller_->game() == nullptr) {
        return;
    }

    QMessageBox box(this);
    box.setWindowTitle("Game over");
    box.setTextFormat(Qt::RichText);
    box.setText(controller_->gameOverText());
    box.addButton("Back to menu", QMessageBox::AcceptRole);

    box.exec();

    controller_->leaveGame();
}
