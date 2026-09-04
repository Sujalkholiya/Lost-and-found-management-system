# Lost & Found Management System - Pure C++ Qt GUI Frontend

## Architecture Overview

This graphical user interface is built entirely using **C++ and the Qt Framework (Qt Widgets + Qt Network + Qt Core)**, strictly adhering to the 100% C++ requirement without any web technologies, JavaScript, TypeScript, or browser layers.

### Project Directory Structure (`gui/`)

```
gui/
├── CMakeLists.txt                # Native CMake build configuration for Qt
├── LostFoundGUI.pro             # Qt Creator / qmake project configuration
└── src/
    ├── main.cpp                 # Application entry point, styling, and QApplication loop
    ├── models/                  # C++ Data Transfer Objects
    │   ├── User.h               # User struct with JSON serialization & role helpers
    │   └── Item.h               # Item struct with JSON serialization & category helpers
    ├── auth/                    # Session & Authentication State
    │   ├── AuthManager.h        # Session singleton with QSettings persistence
    │   └── AuthManager.cpp
    ├── network/                 # Asynchronous HTTP Client
    │   ├── ApiClient.h          # QNetworkAccessManager REST wrapper for Crow backend
    │   └── ApiClient.cpp
    ├── components/              # Reusable UI Widgets & Micro-components
    │   ├── NavbarWidget.h/.cpp  # Header with dynamic role badge and navigation buttons
    │   ├── ItemCardWidget.h/.cpp# Item catalog card with status pill and double-click inspection
    │   ├── StatCardWidget.h/.cpp# Metric KPI counter cards
    │   └── ToastWidget.h/.cpp   # Animated floating feedback toast notifications
    └── views/                   # Full Screen Views & Dialogs
        ├── MainWindow.h/.cpp    # Top-level window managing QStackedWidget screen transitions
        ├── HomeScreen.h/.cpp    # Hero header, quick actions, KPI cards, recent listings
        ├── BrowseScreen.h/.cpp  # Live search filter, category tabs, location dropdown, grid
        ├── ReportScreen.h/.cpp  # Item submission form with validation and Crow POST /api/items
        ├── ItemDetailsDialog.h/.cpp # Modal dialog with full details and campus claim guidelines
        ├── LoginScreen.h/.cpp   # Authentication form connecting to POST /api/login
        ├── RegisterScreen.h/.cpp# Registration form connecting to POST /api/register
        ├── DashboardScreen.h/.cpp# Student dashboard with personal reported items
        └── AdminScreen.h/.cpp   # Role-guarded admin console with live master table
```

## How the Qt GUI Communicates with the C++ Backend

1. **REST Communication**: The `ApiClient` class wraps `QNetworkAccessManager` to perform non-blocking asynchronous HTTP requests to `http://127.0.0.1:18080`.
2. **Bearer Token Authentication**: When a user logs in via `POST /api/login`, the backend issues a 128-bit hex session token. The `AuthManager` stores this token and `ApiClient` automatically injects the `Authorization: Bearer <token>` header into authenticated requests (`POST /api/items`, `GET /api/user/items`, `GET /api/admin/items`).
3. **JSON Processing**: Serialization and deserialization use Qt's native `QJsonDocument`, `QJsonObject`, and `QJsonArray` classes, mapping directly to C++ model structs (`Item`, `User`).

## How to Build and Run

### Option 1: Using Qt Creator
1. Launch **Qt Creator**.
2. Open `gui/LostFoundGUI.pro` or `gui/CMakeLists.txt`.
3. Select your Desktop Qt Kit (e.g. Qt 5.15 or Qt 6 with MinGW or MSVC).
4. Click **Run** (Ctrl+R).

### Option 2: Using CMake from Command Line
```powershell
cmake -B build -S gui -DCMAKE_PREFIX_PATH="<path_to_qt_cmake>"
cmake --build build
./build/LostFoundGUI.exe
```

### Option 3: Using qmake from Command Line
```powershell
cd gui
qmake LostFoundGUI.pro
make (or mingw32-make / nmake)
./release/LostFoundGUI.exe
```
