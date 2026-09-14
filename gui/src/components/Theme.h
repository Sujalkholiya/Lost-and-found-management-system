#ifndef THEME_H
#define THEME_H

#include <QString>

namespace Theme {

// ==========================================
// 1. Core Color Sub-Namespaces
// ==========================================
namespace Primary {
    inline const QString Main    = "#2563EB";
    inline const QString Dark    = "#1D4ED8";
    inline const QString Hover   = "#1D4ED8";
    inline const QString Light   = "#EFF6FF";
    inline const QString Subtle  = "#EFF6FF";
    inline const QString Border  = "#BFDBFE";
}

namespace Text {
    inline const QString Primary   = "#172B4D";
    inline const QString Secondary = "#6B7FA3";
    inline const QString Muted     = "#8A9AB5";
    inline const QString Light     = "#FFFFFF";
}

namespace Border {
    inline const QString Default = "#E2EAF4";
    inline const QString Subtle  = "#E3EAF3";
}

namespace Background {
    inline const QString Page  = "#F7FAFE";
    inline const QString Card  = "#FFFFFF";
    inline const QString Muted = "#F8FAFC";
}

namespace Status {
    inline const QString Success     = "#16A66A";
    inline const QString SuccessBg   = "#EAFBF4";
    inline const QString SuccessText = "#16A66A";
    inline const QString Warning     = "#F59E0B";
    inline const QString WarningBg   = "#FEF3C7";
    inline const QString WarningText = "#B45309";
    inline const QString Error       = "#EF4444";
    inline const QString ErrorBg     = "#FEF2F2";
    inline const QString ErrorText   = "#DC2626";
}

// Flat constants for compatibility
inline constexpr const char* DeepNavy      = "#17345F";
inline constexpr const char* PrimaryBlue   = "#2563EB";
inline constexpr const char* BrightBlue    = "#3B82F6";
inline constexpr const char* SoftBlue      = "#EAF2FF";
inline constexpr const char* VeryLightBlue = "#F4F8FF";
inline constexpr const char* PageBg        = "#F7FAFE";
inline constexpr const char* CardWhite     = "#FFFFFF";
inline constexpr const char* MainText      = "#172B4D";
inline constexpr const char* SecondaryText = "#6B7FA3";
inline constexpr const char* MutedText     = "#8A9AB5";
inline constexpr const char* BorderSubtle  = "#E3EAF3";

// Semantic Status Colors
inline constexpr const char* Success       = "#16A66A";
inline constexpr const char* SuccessBg     = "#EAFBF4";
inline constexpr const char* Warning       = "#F59E0B";
inline constexpr const char* WarningBg     = "#FEF3C7";
inline constexpr const char* Error         = "#EF4444";
inline constexpr const char* ErrorBg       = "#FEF2F2";

// ==========================================
// 2. Standard Stylesheet Generators
// ==========================================

// Primary Blue Action Button
inline QString primaryButton(int radius = 10, int px = 18, int py = 10, int fontSize = 13) {
    return QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: #FFFFFF;"
        "   font-size: %2px;"
        "   font-weight: 700;"
        "   padding: %3px %4px;"
        "   border-radius: %5px;"
        "   border: none;"
        "}"
        "QPushButton:hover { background-color: #1D4ED8; }"
        "QPushButton:pressed { background-color: #1E40AF; }"
        "QPushButton:disabled { background-color: #94A3B8; color: #E2E8F0; }"
    ).arg(PrimaryBlue).arg(fontSize).arg(py).arg(px).arg(radius);
}

// Secondary White/Light-Blue Button
inline QString secondaryButton(int radius = 10, int px = 18, int py = 10, int fontSize = 13) {
    return QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: %2;"
        "   font-size: %3px;"
        "   font-weight: 700;"
        "   padding: %4px %5px;"
        "   border-radius: %6px;"
        "   border: 1px solid %7;"
        "}"
        "QPushButton:hover { background-color: #F8FAFC; border-color: %8; color: %8; }"
        "QPushButton:pressed { background-color: %9; }"
        "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; border-color: #E2E8F0; }"
    ).arg(CardWhite, DeepNavy).arg(fontSize).arg(py).arg(px).arg(radius).arg(Border::Default, PrimaryBlue, SoftBlue);
}

// Subtle Outline Action Button
inline QString outlineButton(int radius = 8, int px = 14, int py = 7, int fontSize = 12) {
    return QString(
        "QPushButton {"
        "   background-color: %1;"
        "   color: %2;"
        "   font-size: %3px;"
        "   font-weight: 600;"
        "   padding: %4px %5px;"
        "   border-radius: %6px;"
        "   border: 1px solid %7;"
        "}"
        "QPushButton:hover { background-color: #2563EB; color: #FFFFFF; border-color: #2563EB; }"
        "QPushButton:pressed { background-color: #1D4ED8; color: #FFFFFF; }"
    ).arg(SoftBlue, PrimaryBlue).arg(fontSize).arg(py).arg(px).arg(radius).arg(Border::Default);
}

// Standard Form Input Style
inline QString inputStyle(int radius = 10, int px = 14, int py = 10, int fontSize = 13) {
    return QString(
        "QLineEdit, QTextEdit {"
        "   background-color: %1;"
        "   color: %2;"
        "   border: 1px solid %3;"
        "   border-radius: %4px;"
        "   padding: %5px %6px;"
        "   font-size: %7px;"
        "   font-family: 'Segoe UI', sans-serif;"
        "   selection-background-color: %8;"
        "   selection-color: #FFFFFF;"
        "}"
        "QLineEdit:hover, QTextEdit:hover { border-color: #CBD5E1; }"
        "QLineEdit:focus, QTextEdit:focus { border: 2px solid %8; background-color: #FFFFFF; color: %2; }"
    ).arg(CardWhite, MainText, Border::Default).arg(radius).arg(py).arg(px).arg(fontSize).arg(PrimaryBlue);
}

// Standard Dropdown ComboBox Style
inline QString comboStyle(int radius = 10, int px = 12, int py = 9, int fontSize = 13) {
    return QString(
        "QComboBox {"
        "   background-color: %1;"
        "   color: %2;"
        "   border: 1px solid %3;"
        "   border-radius: %4px;"
        "   padding: %5px %6px;"
        "   font-size: %7px;"
        "   font-weight: 500;"
        "}"
        "QComboBox:hover { border-color: #CBD5E1; }"
        "QComboBox:focus { border: 2px solid %8; }"
        "QComboBox::drop-down {"
        "   subcontrol-origin: padding;"
        "   subcontrol-position: top right;"
        "   width: 24px;"
        "   border-left: none;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: #FFFFFF;"
        "   color: %2;"
        "   border: 1px solid %3;"
        "   selection-background-color: %8;"
        "   selection-color: #FFFFFF;"
        "   outline: none;"
        "   padding: 4px;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "   min-height: 28px;"
        "   padding: 4px 8px;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "   background-color: %9;"
        "   color: %8;"
        "}"
    ).arg(CardWhite, MainText, Border::Default).arg(radius).arg(py).arg(px).arg(fontSize).arg(PrimaryBlue, SoftBlue);
}

// Clean Card Surface Style
inline QString cardStyle(const QString& objectId = "", int radius = 18) {
    QString selector = objectId.isEmpty() ? "QFrame" : QString("QFrame#%1").arg(objectId);
    return QString(
        "%1 {"
        "   background-color: %2;"
        "   border: 1px solid %3;"
        "   border-radius: %4px;"
        "}"
    ).arg(selector, CardWhite, Border::Default).arg(radius);
}

} // namespace Theme

#endif // THEME_H

