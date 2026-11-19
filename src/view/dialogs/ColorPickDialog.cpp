#include "ColorPickDialog.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include "../Theme.h"

namespace {

QPushButton* createColorButton(const QColor& color, const QString& name) {
    QPushButton* button = new QPushButton(name);
    button->setFixedSize(120, 120);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(
        QString("QPushButton { background: %1; color: white; border: none; "
                "border-radius: 14px; font-size: 16px; font-weight: 700; } "
                "QPushButton:hover { border: 3px solid #f5c542; }")
            .arg(color.name()));

    return button;
}

}  // namespace

ColorPickDialog::ColorPickDialog(QWidget* parent)
    : QDialog(parent) {
    setModal(true);
}

CardColor ColorPickDialog::pick(QWidget* parent, const QString& title) {
    ColorPickDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.setFixedWidth(560);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(18);

    QHBoxLayout* row = new QHBoxLayout();
    row->setSpacing(12);

    const CardColor colors[4] = { CardColor::Red, CardColor::Yellow,
                                  CardColor::Green, CardColor::Blue };

    for (int i = 0; i < 4; ++i) {
        QPushButton* button =
            createColorButton(UnoTheme::cardColorQ(colors[i]),
                              UnoTheme::colorName(colors[i]));
        CardColor chosen = colors[i];

        QObject::connect(button, &QPushButton::clicked, [&dialog, chosen]() {
            dialog.done(static_cast<int>(chosen) + 1);
        });

        row->addWidget(button);
    }

    layout->addStretch();
    layout->addLayout(row);
    layout->addStretch();

    int result = dialog.exec();

    if (result <= 0) {
        return CardColor::None;
    }

    return static_cast<CardColor>(result - 1);
}
