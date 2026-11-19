#include "CardWidget.h"

#include <QFont>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "Theme.h"

namespace {

QPainterPath roundedCardPath(const QRectF& rect) {
    QPainterPath path;

    path.addRoundedRect(rect, rect.width() * 0.13, rect.height() * 0.09);

    return path;
}

void fillRounded(QPainter& painter, const QRectF& rect, const QColor& color) {
    painter.fillPath(roundedCardPath(rect), color);
}

void drawWhiteEllipse(QPainter& painter, const QRectF& rect, qreal rotation) {
    painter.save();
    painter.translate(rect.center());
    painter.rotate(rotation);

    painter.setPen(QPen(QColor(20, 20, 20, 60), 1.5));
    painter.setBrush(QColor("#f6f3ea"));

    painter.drawEllipse(QPointF(0, 0), rect.width() * 0.335,
                        rect.height() * 0.295);

    painter.restore();
}

void drawCornerText(QPainter& painter, const QRectF& rect, const QString& text,
                    const QColor& color) {
    QFont font(UnoTheme::cardFamily);
    font.setBold(true);
    font.setPixelSize(static_cast<int>(rect.height() * 0.13));

    painter.setFont(font);
    painter.setPen(color);

    painter.drawText(QRectF(rect.left() + rect.width() * 0.07,
                            rect.top() + rect.height() * 0.03,
                            rect.width() * 0.4, rect.height() * 0.16),
                     Qt::AlignLeft | Qt::AlignVCenter, text);

    painter.save();
    painter.translate(rect.center());
    painter.rotate(180);
    painter.translate(-rect.center());
    painter.drawText(QRectF(rect.left() + rect.width() * 0.07,
                            rect.top() + rect.height() * 0.03,
                            rect.width() * 0.4, rect.height() * 0.16),
                     Qt::AlignLeft | Qt::AlignVCenter, text);
    painter.restore();
}

void drawSkipGlyph(QPainter& painter, const QRectF& rect,
                   const QColor& color) {
    QPointF center = rect.center();
    qreal radius = rect.width() * 0.145;
    QPen pen(color, rect.width() * 0.05, Qt::SolidLine, Qt::RoundCap);

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, radius, radius);

    painter.drawLine(QPointF(center.x() - radius, center.y() + radius),
                     QPointF(center.x() + radius, center.y() - radius));
}

void drawReverseGlyph(QPainter& painter, const QRectF& rect,
                      const QColor& color) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);

    qreal unit = rect.width() * 0.09;
    QPointF center = rect.center();

    for (int side = 0; side < 2; ++side) {
        painter.save();

        if (side == 1) {
            painter.translate(center);
            painter.rotate(180);
            painter.translate(-center);
        }

        QPolygonF arrow;
        arrow << QPointF(center.x() + unit * 0.2, center.y() - unit * 0.4)
              << QPointF(center.x() + unit * 1.3, center.y() - unit * 0.4)
              << QPointF(center.x() + unit * 1.3, center.y() - unit * 0.9)
              << QPointF(center.x() + unit * 2.5, center.y())
              << QPointF(center.x() + unit * 1.3, center.y() + unit * 0.9)
              << QPointF(center.x() + unit * 1.3, center.y() + unit * 0.4)
              << QPointF(center.x() + unit * 0.2, center.y() + unit * 0.4);

        painter.drawPolygon(arrow);
        painter.restore();
    }
}

void drawWildQuadrants(QPainter& painter, const QRectF& rect) {
    QRectF inner = rect.adjusted(rect.width() * 0.12, rect.height() * 0.10,
                                 -rect.width() * 0.12, -rect.height() * 0.10);
    QPointF center = inner.center();

    struct Quadrant {
        QRectF rectArea;
        QColor color;
    };

    Quadrant quadrants[4] = {
        { QRectF(inner.left(), inner.top(),
                 inner.width() / 2.0 - 2, inner.height() / 2.0 - 2),
          UnoTheme::cardRed },
        { QRectF(center.x() + 2, inner.top(),
                 inner.width() / 2.0 - 2, inner.height() / 2.0 - 2),
          UnoTheme::cardBlue },
        { QRectF(inner.left(), center.y() + 2,
                 inner.width() / 2.0 - 2, inner.height() / 2.0 - 2),
          UnoTheme::cardGreen },
        { QRectF(center.x() + 2, center.y() + 2,
                 inner.width() / 2.0 - 2, inner.height() / 2.0 - 2),
          UnoTheme::cardYellow }
    };

    for (int i = 0; i < 4; ++i) {
        QPainterPath path;
        path.addEllipse(quadrants[i].rectArea);
        painter.fillPath(path, quadrants[i].color);
    }
}

}  // namespace

CardWidget::CardWidget(QWidget* parent)
    : QWidget(parent),
      faceDown_(false),
      selected_(false),
      playable_(true) {
    setFixedSize(92, 138);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
}

void CardWidget::setCard(const Card& card) {
    card_ = card;
    faceDown_ = false;
    update();
}

void CardWidget::setFaceDown(bool faceDown) {
    faceDown_ = faceDown;
    update();
}

void CardWidget::setSelected(bool selected) {
    selected_ = selected;
    update();
}

void CardWidget::setPlayable(bool playable) {
    playable_ = playable;
    update();
}

void CardWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF rect = this->rect().adjusted(1.0, 1.0, -2.0, -2.0);

    if (faceDown_) {
        paintCardBack(painter, rect);
    } else {
        paintFace(painter, rect);
    }

    if (!playable_) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(12, 20, 16, 150));
        painter.drawPath(roundedCardPath(rect));
    }

    if (selected_) {
        painter.setPen(QPen(UnoTheme::accent, 3.2));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(roundedCardPath(rect.adjusted(1.6, 1.6, -1.6, -1.6)));
    } else if (underMouse() && isEnabled() && !faceDown_) {
        painter.setPen(QPen(QColor(255, 255, 255, 190), 1.8));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(roundedCardPath(rect.adjusted(1.2, 1.2, -1.2, -1.2)));
    }
}

void CardWidget::paintFace(QPainter& painter, const QRectF& rect) {
    QColor base = UnoTheme::cardColorQ(card_.getColor());

    if (card_.isWildCard()) {
        fillRounded(painter, rect, UnoTheme::cardWildDark);
        drawWhiteEllipse(painter, rect, -28);
        drawWildQuadrants(painter, rect);
    } else {
        fillRounded(painter, rect, base);

        painter.save();
        painter.setPen(QPen(QColor(255, 255, 255, 80), 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(roundedCardPath(rect.adjusted(5, 5, -5, -5)));
        painter.restore();

        drawWhiteEllipse(painter, rect, -28);
    }

    QColor glyph = UnoTheme::cardWildDark;

    switch (card_.getType()) {
        case CardType::Number: {
            QFont font(UnoTheme::cardFamily);
            font.setBold(true);
            font.setPixelSize(static_cast<int>(rect.height() * 0.40));

            painter.setFont(font);
            painter.setPen(glyph);
            painter.drawText(rect, Qt::AlignCenter,
                             QString::number(card_.getValue()));

            drawCornerText(painter, rect, QString::number(card_.getValue()),
                           QColor("#f6f3ea"));
            break;
        }
        case CardType::Skip: {
            drawSkipGlyph(painter, rect, glyph);
            drawCornerText(painter, rect, "S", QColor("#f6f3ea"));
            break;
        }
        case CardType::Reverse: {
            drawReverseGlyph(painter, rect, glyph);
            drawCornerText(painter, rect, "R", QColor("#f6f3ea"));
            break;
        }
        case CardType::DrawTwo: {
            QFont font(UnoTheme::cardFamily);
            font.setBold(true);
            font.setPixelSize(static_cast<int>(rect.height() * 0.30));

            painter.setFont(font);
            painter.setPen(glyph);
            painter.drawText(rect, Qt::AlignCenter, "+2");

            drawCornerText(painter, rect, "+2", QColor("#f6f3ea"));
            break;
        }
        case CardType::Wild: {
            drawCornerText(painter, rect, "W", QColor("#f6f3ea"));
            break;
        }
        case CardType::WildDrawFour: {
            QFont font(UnoTheme::cardFamily);
            font.setBold(true);
            font.setPixelSize(static_cast<int>(rect.height() * 0.22));

            painter.setFont(font);
            painter.setPen(glyph);
            painter.drawText(rect.adjusted(0, rect.height() * 0.42, 0, 0),
                             Qt::AlignCenter, "+4");

            drawCornerText(painter, rect, "+4", QColor("#f6f3ea"));
            break;
        }
    }
}

void CardWidget::paintCardBack(QPainter& painter, const QRectF& rect) {
    fillRounded(painter, rect, UnoTheme::cardBack);

    painter.save();
    painter.setPen(QPen(QColor(255, 255, 255, 70), 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(roundedCardPath(rect.adjusted(5, 5, -5, -5)));
    painter.restore();

    painter.save();
    painter.translate(rect.center());
    painter.rotate(-28);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#131313"));
    painter.drawEllipse(QPointF(0, 0), rect.width() * 0.36,
                        rect.height() * 0.30);

    painter.restore();

    QFont font(UnoTheme::cardFamily);
    font.setBold(true);
    font.setPixelSize(static_cast<int>(rect.height() * 0.20));

    painter.setFont(font);
    painter.setPen(QColor("#f5c542"));
    painter.drawText(rect, Qt::AlignCenter, "UNO");
}

void CardWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && isEnabled()) {
        emit clicked();
    }

    QWidget::mousePressEvent(event);
}
