# 🎨 Campus Lost & Found — Pure C++ Qt 6 GUI Client

## Architecture Overview

This desktop user interface is built strictly using **100% C++17 and the Qt 6 Framework (QtWidgets, QtNetwork, QtGui, QtCore)**. It operates natively without web layers, HTML, CSS, JavaScript, QML, or Chromium-based runtimes.

---

## 📂 Frontend Directory Structure (`gui/`)

```
gui/
├── CMakeLists.txt                 # Native CMake build configuration for Qt
├── LostFoundGUI.pro              # Qt Creator / qmake project configuration
├── resources.qrc                 # Qt Resource file (embedded PNG assets & campus backdrop)
├── resources/
│   └── images/
│       ├── campus-background.png # High-resolution GEHU Bhimtal campus photograph (1000x750)
│       ├── hero_campus.png
│       ├── badge_lost.png
│       ├── badge_found.png
│       └── empty_search.png
└── src/
    ├── main.cpp                  # Application entry point, high-DPI setup, Segoe UI typography
    ├── models/
    │   ├── User.h                # User model with role helpers
    │   ├── Item.h                # Item model with JSON parsing & image URL resolution
    │   ├── Claim.h               # Claim & ClaimStats structs with multi-key deserialization
    │   └── CampusLocations.h     # Standardized campus location list
    ├── auth/
    │   ├── AuthManager.h         # Session singleton with QSettings persistence
    │   └── AuthManager.cpp
    ├── network/
    │   ├── ApiClient.h           # Non-blocking REST client wrapping QNetworkAccessManager
    │   ├── ApiClient.cpp         # Endpoints for items, multipart photo uploads, claims, users
    │   ├── ImageLoader.h         # Asynchronous image downloader with cache & QPointer safety
    │   └── ImageLoader.cpp
    ├── components/
    │   ├── NavbarWidget.h/.cpp   # Polished header bar with university portal branding and profile chip
    │   ├── SidebarWidget.h/.cpp  # Deep navy sidebar with "My Claims" (🎒) navigation
    │   ├── ItemCardWidget.h/.cpp # Responsive card with badges, photo thumbnail, and action buttons
    │   ├── StatCardWidget.h/.cpp # Animated KPI counter cards
    │   ├── ToastWidget.h/.cpp    # Animated floating notification toasts
    │   ├── AvatarHelper.h/.cpp   # User avatar circle renderer based on user name
    │   └── EmptyStateWidget.h/.cpp# Decorative placeholder when list views are empty
    └── views/
        ├── MainWindow.h/.cpp     # Root container with backdrop painter & QStackedWidget router
        ├── HomeScreen.h/.cpp     # Hero header, quick actions, KPI cards, recent listings
        ├── BrowseScreen.h/.cpp   # Real-time search, category pills, location dropdown, card grid
        ├── ReportScreen.h/.cpp   # Item reporting form supporting JSON or multipart photo uploads
        ├── ItemDetailsDialog.h/.cpp # Full inspection modal with Claim button & status badges
        ├── ClaimFormDialog.h/.cpp# Ownership verification modal with 4 prompts & conduct check
        ├── MyClaimsScreen.h/.cpp # Student claim tracker with status tabs & Digital Pickup Pass
        ├── AdminClaimReviewDialog.h/.cpp # Side-by-side claim verification & handover dialog
        ├── ProfileDialog.h/.cpp  # User profile settings & account details editor
        ├── LoginScreen.h/.cpp    # Authentication screen connecting to POST /api/users/login
        ├── RegisterScreen.h/.cpp # Registration screen connecting to POST /api/users/register
        ├── DashboardScreen.h/.cpp# User dashboard with personal listings & pickup alert banner
        ├── AdminScreen.h/.cpp    # Role-guarded admin console (Claim Requests & Master Inventory)
        └── AccessDeniedScreen.h/.cpp # Barrier screen for unauthorized route access
```

---

## 🌟 Key Frontend Features

### 1. Item Claim & Verification Dialog (`ClaimFormDialog`)
- **Interactive Verification**: Requests specific proof (where misplaced, distinctive marks, inner contents, extra notes).
- **Code of Conduct Affirmation**: Mandatory confirmation checkbox before submission is enabled.
- **Immediate State Management**: Submit button resets immediately upon response; custom-styled, high-contrast modal dialog renders with clear reference code (`CLM-2026-XXXXX`).

### 2. Student Claims & Digital Pickup Pass (`MyClaimsScreen`)
- **Status Filter Tabs**: Easily toggle between *All*, *Pending Review*, *Ready for Pickup*, *Handed Over*, and *Rejected*.
- **Progress Stepper**: Live 3-step timeline (`Submitted` ➔ `Under Review` ➔ `Ready for Pickup`).
- **Emerald Digital Pickup Pass**: Shows official reference number, designated office (*Admin Block - Room 102*), operating hours, and Student ID presentation instructions.

### 3. Administrator Claim Console & Physical Handover
- **Admin Claim Review Dialog (`AdminClaimReviewDialog`)**: Side-by-side layout comparing reported item parameters against claimant answers.
- **Approval / Rejection**: Assign custom pickup locations or enter mandatory rejection explanations.
- **Physical Handover Desk Confirmation**: Checkbox verification confirming student presented campus ID card before finalizing handover.

### 4. User Profile Settings (`ProfileDialog`)
- Accessible from the top navbar user avatar chip.
- Shows User ID, Role badge, Name, and Email with live update capability.

---

## 🛠️ How to Build and Run

### Option 1: Using Qt Creator (Recommended)
1. Launch **Qt Creator**.
2. Open `gui/LostFoundGUI.pro` (or `gui/CMakeLists.txt`).
3. Select the **Qt 6.x MinGW 64-bit** Kit.
4. Click **Run** (`Ctrl + R`).

### Option 2: Using Command Line (`mingw32-make`)
```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.2\mingw_64\bin;$env:PATH"
mingw32-make -C gui\build\Desktop_Qt_6_11_2_MinGW_64_bit_Debug -j4
.\gui\build\Desktop_Qt_6_11_2_MinGW_64_bit_Debug\debug\LostFoundGUI.exe
```
