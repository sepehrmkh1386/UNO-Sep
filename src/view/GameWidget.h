#ifndef GAME_WIDGET_H
#define GAME_WIDGET_H

// The game table screen: opponents, deck/discard piles, active color,
// direction, pending penalty, the human hand, action buttons and an event
// log.
//
// The widget reads ONLY public engine accessors (Round / Game / Player) and
// performs actions exclusively through the GuiController. It never decides
// move legality itself — legality hints use the engine's own
// Round::canPlayCard() query, and every action is validated by the engine.

#include <QWidget>

#include "../ds/DynamicArray.h"
#include "../model/Card.h"

class QLabel;
class QPushButton;
class QTextBrowser;
class QScrollArea;
class QVBoxLayout;
class QHBoxLayout;

class CardWidget;
class GuiController;
class Player;
class PlayerPanel;

class GameWidget : public QWidget {
    Q_OBJECT

public:
    explicit GameWidget(GuiController* controller, QWidget* parent = nullptr);

public slots:
    void refreshFromEngine();

private slots:
    void onPlaySelected();
    void onPlayDrawn();
    void onDraw();
    void onPass();
    void onChance();
    void onCardClicked(int cardIndex);
    void askStartColor();
    void showRoundOverDialog();
    void showGameOverDialog();

private:
    void buildUi();
    void clearHandRow();
    void rebuildHand(int shownPlayerSeat);
    void rebuildOpponents();
    void updateButtons();
    Player* shownPlayer(int& seatOut) const;

    GuiController* controller_;

    QLabel* roundLabel;
    QLabel* winLabel;
    QLabel* directionLabel;

    QHBoxLayout* opponentsLayout;
    QWidget* opponentsHost;

    CardWidget* deckCard;
    QLabel* deckLabel;
    CardWidget* discardCard;
    QLabel* activeColorChip;
    QLabel* statusLabel;
    QLabel* penaltyLabel;
    QLabel* localSeatLabel;

    QLabel* handTitle;
    QHBoxLayout* handRowLayout;
    QWidget* handHost;
    QScrollArea* handScroll;

    QPushButton* playSelectedButton;
    QPushButton* playDrawnButton;
    QPushButton* drawButton;
    QPushButton* passButton;
    QPushButton* chanceButton;

    QTextBrowser* logView;

    DynamicArray<CardWidget*> handWidgets_;
    int selectedCardIndex_;
    bool shownIsCurrentPlayer_;
};

#endif
