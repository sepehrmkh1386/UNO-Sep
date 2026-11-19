#ifndef PLAYER_PANEL_H
#define PLAYER_PANEL_H

// One seat on the table: name, type (Human/Bot), card count and status.
// Presentation only; all values come from the engine via refresh calls.

#include <QFrame>
#include <QString>

class QLabel;

class PlayerPanel : public QFrame {
    Q_OBJECT

public:
    explicit PlayerPanel(QWidget* parent = nullptr);

    void updateInfo(const QString& name, bool human, int handSize,
                    bool isCurrent, bool winner, const QString& note);

private:
    QLabel* nameLabel;
    QLabel* countLabel;
    QLabel* subLabel;
};

#endif
