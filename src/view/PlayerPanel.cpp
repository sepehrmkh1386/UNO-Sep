#include "PlayerPanel.h"

#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>

PlayerPanel::PlayerPanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName("SeatFrame");
    setFixedWidth(168);
    setFixedHeight(112);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(3);

    nameLabel = new QLabel(this);
    nameLabel->setObjectName("PlayerNameLabel");

    countLabel = new QLabel(this);
    countLabel->setObjectName("PlayerCountLabel");

    subLabel = new QLabel(this);
    subLabel->setObjectName("PlayerSubLabel");

    layout->addWidget(nameLabel);
    layout->addWidget(countLabel);
    layout->addWidget(subLabel);
    layout->addStretch();
}

void PlayerPanel::updateInfo(const QString& name, bool human, int handSize,
                             bool isCurrent, bool winner,
                             const QString& note) {
    QString typeTag = human ? "Human" : "Bot";
    QString winnerTag = winner ? "  |  Winner" : "";

    nameLabel->setText(name + "  (" + typeTag + winnerTag + ")");
    countLabel->setText(QString::number(handSize));

    QString cardWord = (handSize == 1) ? "card" : "cards";
    QString base = QString::number(handSize) + " " + cardWord;

    if (!note.isEmpty()) {
        subLabel->setText(note);
    } else if (isCurrent) {
        subLabel->setText(base + " — on turn");
    } else {
        subLabel->setText(base);
    }

    setProperty("current", isCurrent);
    style()->unpolish(this);
    style()->polish(this);
}
