#ifndef COLOR_PICK_DIALOG_H
#define COLOR_PICK_DIALOG_H

// Modal dialog shown when a Wild card needs a color choice (or when the
// first card of a round is a Wild card and the engine waits for a start
// color). It only collects input; the engine applies it via setStartColor()
// or playCard(..., chosenColor).

#include <QDialog>

#include "../../model/Card.h"

class ColorPickDialog : public QDialog {
    Q_OBJECT

public:
    explicit ColorPickDialog(QWidget* parent = nullptr);

    // Opens the dialog and returns the chosen color, or CardColor::None if
    // the dialog was dismissed.
    static CardColor pick(QWidget* parent, const QString& title);
};

#endif
