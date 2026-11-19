#ifndef CARD_WIDGET_H
#define CARD_WIDGET_H

// A single visual UNO card, fully custom painted with QPainter.
// Presentation only: it displays a Card value and reports clicks.
// The widget never decides whether a move is legal; the engine does.

#include <QWidget>

#include "../model/Card.h"

class CardWidget : public QWidget {
    Q_OBJECT

public:
    explicit CardWidget(QWidget* parent = nullptr);

    void setCard(const Card& card);
    void setFaceDown(bool faceDown);
    void setSelected(bool selected);
    void setPlayable(bool playable);

    const Card& card() const { return card_; }
    bool isSelected() const { return selected_; }
    bool isFaceDown() const { return faceDown_; }

    QSize sizeHint() const override { return QSize(92, 138); }
    QSize minimumSizeHint() const override { return QSize(92, 138); }

    // Draws the card-back motif (used for the deck pile).
    static void paintCardBack(QPainter& painter, const QRectF& rect);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void paintFace(QPainter& painter, const QRectF& rect);

    Card card_;
    bool faceDown_;
    bool selected_;
    bool playable_;
};

#endif
