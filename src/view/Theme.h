#ifndef THEME_H
#define THEME_H

// Central place for all GUI colors, fonts and the application stylesheet.
// Purely presentational: no game logic lives here.

#include <QColor>
#include <QString>

#include "../model/Card.h"

namespace UnoTheme {

// Window / table
inline const QColor feltDark("#07271b");
inline const QColor felt("#0d3b2a");
inline const QColor feltLight("#145232");
inline const QColor panel("#0f3f2e");
inline const QColor panelBorder("#2b6b4f");
inline const QColor textMain("#f2f7f4");
inline const QColor textDim("#b8cfc3");
inline const QColor accent("#f5c542");
inline const QColor danger("#ff8a80");

// UNO card colors
inline const QColor cardRed("#d33636");
inline const QColor cardYellow("#e6a817");
inline const QColor cardGreen("#3d9e4d");
inline const QColor cardBlue("#2f6fd0");
inline const QColor cardWildDark("#1d1d2b");
inline const QColor cardBack("#b32626");

inline QColor cardColorQ(CardColor color) {
    switch (color) {
        case CardColor::Red:
            return cardRed;
        case CardColor::Yellow:
            return cardYellow;
        case CardColor::Green:
            return cardGreen;
        case CardColor::Blue:
            return cardBlue;
        case CardColor::None:
        default:
            return cardWildDark;
    }
}

inline QString colorName(CardColor color) {
    switch (color) {
        case CardColor::Red:
            return "Red";
        case CardColor::Yellow:
            return "Yellow";
        case CardColor::Green:
            return "Green";
        case CardColor::Blue:
            return "Blue";
        case CardColor::None:
        default:
            return "None";
    }
}

// Font families that exist on Windows and fall back gracefully elsewhere.
inline const QString headingFamily = "Segoe UI";
inline const QString cardFamily = "Arial";

inline QString appStyleSheet() {
    return QString(
        "QMainWindow, QWidget#RootBackground { background: %1; }\n"

        "QLabel { color: %2; font-size: 13px; }\n"
        "QLabel#TitleLabel { color: %3; font-size: 42px; font-weight: 800; }\n"
        "QLabel#SubtitleLabel { color: %4; font-size: 13px; }\n"
        "QLabel#SectionLabel { color: %3; font-size: 15px; font-weight: 700; }\n"
        "QLabel#StatusMessage { color: %3; font-size: 15px; font-weight: 600; }\n"
        "QLabel#PenaltyLabel { color: %5; font-size: 14px; font-weight: 700; }\n"
        "QLabel#PlayerNameLabel { color: %2; font-size: 14px; font-weight: 700; }\n"
        "QLabel#PlayerCountLabel { color: %3; font-size: 20px; font-weight: 800; }\n"
        "QLabel#PlayerSubLabel { color: %4; font-size: 11px; }\n"
        "QLabel#CenterInfoLabel { color: %2; font-size: 13px; font-weight: 600; }\n"

        "QFrame#PanelFrame, QFrame#SeatFrame { background: %6; border: 1px "
        "solid %7; border-radius: 12px; }\n"
        "QFrame#SeatFrame[current='true'] { border: 2px solid %3; "
        "background: %8; }\n"

        "QPushButton { background: #1d5a41; color: %2; border: 1px solid %7; "
        "border-radius: 8px; padding: 8px 16px; font-size: 13px; "
        "font-weight: 600; }\n"
        "QPushButton:hover { background: #26704f; }\n"
        "QPushButton:pressed { background: #164732; }\n"
        "QPushButton:disabled { background: #16402f; color: #6f8c7e; "
        "border-color: #1d4433; }\n"
        "QPushButton#PrimaryButton { background: %3; color: #231a04; "
        "border: none; font-size: 15px; font-weight: 800; padding: 11px 26px; }\n"
        "QPushButton#PrimaryButton:hover { background: #ffd35c; }\n"
        "QPushButton#PrimaryButton:disabled { background: #6d6247; "
        "color: #9a8f74; }\n"
        "QPushButton#DangerButton { background: #8c2f2f; color: %2; }\n"
        "QPushButton#DangerButton:hover { background: #a83a3a; }\n"

        "QLineEdit, QSpinBox, QComboBox { background: #0a2e21; color: %2; "
        "border: 1px solid %7; border-radius: 8px; padding: 7px 10px; "
        "font-size: 13px; selection-background-color: #1d5a41; }\n"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border-color: %3; }\n"
        "QSpinBox QLineEdit, QAbstractSpinBox QLineEdit { background: "
        "transparent; color: %2; border: none; }\n"

        "QComboBox QAbstractItemView { background: #0f3f2e; color: %2; "
        "selection-background-color: #1d5a41; border: 1px solid %7; }\n"

        "QTextBrowser, QTextEdit { background: #0a2e21; color: %4; "
        "border: 1px solid %7; border-radius: 10px; padding: 6px; "
        "font-family: 'Consolas', 'Courier New', monospace; font-size: 12px; }\n"

        "QScrollArea { border: none; background: transparent; }\n"
        "QScrollArea > QWidget > QWidget { background: transparent; }\n"
        "QScrollBar:vertical { background: #0a2e21; width: 10px; "
        "border-radius: 5px; }\n"
        "QScrollBar::handle:vertical { background: #2b6b4f; border-radius: 5px; "
        "min-height: 30px; }\n"
        "QScrollBar::handle:vertical:hover { background: #388562; }\n"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical "
        "{ height: 0; }\n"
        "QScrollBar:horizontal { background: #0a2e21; height: 10px; "
        "border-radius: 5px; }\n"
        "QScrollBar::handle:horizontal { background: #2b6b4f; border-radius: 5px; "
        "min-width: 30px; }\n"
        "QScrollBar::handle:horizontal:hover { background: #388562; }\n"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal "
        "{ width: 0; }\n"

        "QMenuBar { background: %1; color: %4; }\n"
        "QMenuBar::item:selected { background: #1d5a41; }\n"
        "QMenu { background: #0f3f2e; color: %2; border: 1px solid %7; }\n"
        "QMenu::item:selected { background: #1d5a41; }\n"

        "QMessageBox { background: %6; }\n"
        "QMessageBox QLabel { color: %2; font-size: 13px; }\n")
        .arg(felt.name(), textMain.name(), accent.name(), textDim.name(),
             danger.name(), panel.name(), panelBorder.name(),
             feltLight.name());
}

}  // namespace UnoTheme

#endif
