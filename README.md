# 🎓 Campus Lost & Found Management System

A high-performance, full-stack **100% C++** university property recovery platform. Built with a modern **Qt 6 C++ Desktop Client** and a lightweight, multi-threaded **Crow C++ REST API Microservice** backed by **SQLite3** and **BCrypt**.

---

## 📌 Table of Contents

- [Overview](#-overview)
- [Key Features](#-key-features)
- [Architecture & Tech Stack](#-architecture--tech-stack)
- [Project Directory Structure](#-project-directory-structure)
- [Database Schema](#-database-schema)
- [REST API Reference](#-rest-api-reference)
- [Prerequisites & Environment Setup](#-prerequisites--environment-setup)
- [Building & Running the Project](#-building--running-the-project)
  - [1. Start the Backend Server](#1-start-the-backend-server)
  - [2. Build & Launch the Qt GUI Client](#2-build--launch-the-qt-gui-client)
- [User Roles & Security](#-user-roles--security)
- [Visual Design & Assets](#-visual-design--assets)
- [Troubleshooting & FAQs](#-troubleshooting--faqs)

---

## 🌟 Overview

The **Campus Lost & Found Management System** provides students, faculty, and campus administrators with an intuitive and secure way to report, browse, search, and claim misplaced belongings across campus. 

Unlike traditional web applications, this project is built entirely in **modern C++ (C++17)**:
- **Zero Web Technologies**: No JavaScript, TypeScript, Node.js, Python, or browser engines.
- **Native Performance**: Ultra-fast startup, low memory footprint, and responsive desktop UI.
- **RESTful Decoupled Architecture**: The Qt GUI client communicates asynchronously with the Crow C++ microservice via JSON and multipart HTTP requests over `http://127.0.0.1:18080`.

---

## ✨ Key Features

### 🔐 Authentication & Session Management
- **User Registration & Login**: Account creation with email format validation and password confirmation.
- **BCrypt Password Hashing**: Passwords are securely hashed with salted BCrypt before database persistence.
- **Token-Based Sessions**: Server generates 128-bit hex tokens stored in SQLite and persisted on the client via `QSettings`.
- **Role-Based Access Control (RBAC)**: Distinct permissions for `student` and `admin` roles.

### 📦 Item Reporting & Image Attachments
- **Dual Reporting**: Report items classified as either **Lost** (`🔴 LOST`) or **Found** (`🟢 FOUND`).
- **Photo Upload**: Supports multipart/form-data image attachment (`.png`, `.jpg`, `.jpeg`) up to 5 MB.
- **Unique Filename Generation**: Files are sanitized and stored on the server as `uploads/item_<timestamp>_<counter>.<ext>`.
- **Static Image Serving**: Crow serves uploaded images with proper MIME types and HTTP cache headers (`Cache-Control: public, max-age=86400`).

### 🔍 Search, Filter & Campus Locations
- **Real-Time Search**: Instant keyword filtering across item title, description, and location.
- **Predefined Campus Locations**: Filter by campus zones (Central Library, Main Cafeteria, Computer Lab, Auditorium, Sports Ground, Hostel Block, Parking Area, etc.).
- **Category Filter Pills**: Quick toggle between *All Items*, *Lost Items*, and *Found Items*.

### 🗑 Ownership-Guarded Item Deletion
- **Granular Deletion Control**: Users can delete their own reported items; administrators can delete any item.
- **Cascade Cleanup**: Deleting an item removes both the database record and the associated physical image file from the server's `uploads/` folder.
- **Double Confirmation**: Safeguarded by confirmation dialogs in the UI.

### 📊 Live Analytics & Dashboard
- **Campus Overview KPI Cards**: Real-time counters for *Total Listed Items*, *Active Lost Items*, *Found Items*, and *Reports This Week*.
- **Smooth Number Animations**: Metrics animate fluidly using `QVariantAnimation` with cubic easing.
- **Personal Dashboard**: Lists all items reported by the signed-in user with quick navigation and delete options.

### 🛡️ Administrator Console
- **Master Inventory Audit Table**: Role-guarded grid displaying ID, title, type, location, user ID, status, and reporting date.
- **Quick Search & Refresh**: Instant client-side filtering across the entire campus registry.

---

## 🏗 Architecture & Tech Stack

```
┌──────────────────────────────────────────────────────────┐
│                   Qt 6 GUI Frontend                      │
│   (C++17, Qt Widgets, Qt Network, Qt Core, Segoe UI)     │
│       • Sidebar & Frosted Navbar Navigation              │
│       • Responsive Grid & Asynchronous ImageLoader       │
│       • Local Qt Resource System (.qrc)                  │
└────────────────────────────┬─────────────────────────────┘
                             │ HTTP / JSON / Multipart
                             │ (Bearer Token Auth)
                             ▼
┌──────────────────────────────────────────────────────────┐
│                   Crow C++ Microservice                  │
│       (C++17, Asio Standalone, Crow Framework 1.2)       │
│       • Port: 127.0.0.1:18080                            │
│       • Multithreaded Request Pool                       │
│       • BCrypt Security & Token Authentication Guard     │
└────────────────────────────┬─────────────────────────────┘
                             │
                             ▼
┌──────────────────────────────────────────────────────────┐
│                  SQLite3 Database Engine                 │
│       • File: lost_found.db                              │
│       • Tables: users, items, sessions                   │
│       • Directory: uploads/ (attached images)            │
└──────────────────────────────────────────────────────────┘
```

| Layer | Component | Description |
| :--- | :--- | :--- |
| **Frontend Framework** | Qt 6 (6.11.2) / Qt Widgets | Native C++ desktop UI with Segoe UI typography and custom styling. |
| **Networking (Client)** | `QNetworkAccessManager` | Asynchronous non-blocking HTTP REST client with callback dispatchers. |
| **Backend Framework** | [CrowCpp](https://github.com/CrowCpp/Crow) (v1.2.0) | Multi-threaded C++ microframework built on Asio. |
| **Network Core (Server)**| [Asio Standalone](https://github.com/chriskohlhoff/asio) (v1.28.0) | Header-only asynchronous I/O networking library. |
| **Database** | [SQLite3](https://sqlite.org/) | Embedded ACID-compliant database engine. |
| **Password Security** | [libbcrypt](https://github.com/rg3/libbcrypt) | Blowfish-based cryptographic password hashing. |
| **Build Tools** | CMake 3.15+ / MinGW-w64 (GCC 13.1.0) / qmake | Native C++ compilation and dependency resolution. |

---

## 📂 Project Directory Structure

```
lost-and-found-backend/
├── CMakeLists.txt                 # Root CMake configuration (backend + GUI subdirectory)
├── lost_found.db                  # Primary SQLite3 database
├── uploads/                       # Server storage directory for uploaded item photos
├── src/                           # Backend Server Source Code (Crow C++)
│   ├── main.cpp                   # Crow server startup and route registration
│   ├── config/
│   │   ├── database.h             # SQLite3 connection and query declarations
│   │   └── database.cpp           # SQLite3 connection, schema creation, migrations
│   ├── controllers/
│   │   ├── userController.cpp     # User registration, login, token generation
│   │   └── itemController.cpp     # CRUD, multipart upload, ownership check, static image server
│   ├── middleware/
│   │   ├── authMiddleware.h       # Bearer token verification interface
│   │   └── authMiddleware.cpp     # Session token validation against database
│   └── routes/
│       ├── userRoutes.cpp         # Routes: /api/users/register, /api/users/login
│       └── itemRoutes.cpp         # Routes: /api/items, /uploads/<string>, /api/admin/items
│
└── gui/                           # Frontend Client Source Code (Qt 6 C++)
    ├── CMakeLists.txt             # Native CMake configuration for Qt GUI
    ├── LostFoundGUI.pro          # Qt Creator / qmake project file
    ├── resources.qrc              # Qt Resource file (embedded PNG assets)
    ├── resources/
    │   └── images/
    │       ├── campus-background.png  # High-resolution GEHU Bhimtal background (1000x750)
    │       ├── hero_campus.png
    │       ├── badge_lost.png
    │       ├── badge_found.png
    │       └── empty_search.png
    └── src/
        ├── main.cpp               # GUI entrypoint, font setup, QApplication event loop
        ├── models/
        │   ├── User.h             # User model with role helpers
        │   ├── Item.h             # Item model with JSON serialization & image URL resolver
        │   └── CampusLocations.h  # Centralized campus location definitions
        ├── auth/
        │   ├── AuthManager.h      # Session singleton with QSettings persistence
        │   └── AuthManager.cpp
        ├── network/
        │   ├── ApiClient.h        # REST client wrapping QNetworkAccessManager
        │   ├── ApiClient.cpp
        │   ├── ImageLoader.h      # Asynchronous image downloader with QPointer safety & memory cache
        │   └── ImageLoader.cpp
        ├── components/
        │   ├── NavbarWidget.h/.cpp    # Frosted translucent top bar with auth controls
        │   ├── SidebarWidget.h/.cpp   # Deep navy sidebar with active state bullets
        │   ├── ItemCardWidget.h/.cpp  # Responsive card with badge, photo thumbnail, delete button
        │   ├── StatCardWidget.h/.cpp  # Metric KPI cards with smooth counting animations
        │   ├── ToastWidget.h/.cpp     # Animated floating notification toasts
        │   └── EmptyStateWidget.h/.cpp# Decorative placeholder when list is empty
        └── views/
            ├── MainWindow.h/.cpp      # Root window with CampusBackgroundArea and QStackedWidget
            ├── HomeScreen.h/.cpp      # Hero card, campus overview, recent item listings
            ├── BrowseScreen.h/.cpp    # Full search bar, location dropdown, category filter grid
            ├── ReportScreen.h/.cpp    # Multipart item submission form with photo preview
            ├── ItemDetailsDialog.h/.cpp # Detailed popup with full photo and delete action
            ├── LoginScreen.h/.cpp     # Clean authentication card
            ├── RegisterScreen.h/.cpp  # Account registration card
            ├── DashboardScreen.h/.cpp # User personal reports dashboard
            ├── AdminScreen.h/.cpp     # Role-guarded inventory audit console
            └── AccessDeniedScreen.h/.cpp # Security gate when unauthorized
```

---

## 🗄 Database Schema

The SQLite database (`lost_found.db`) includes automatic table initialization and startup migrations:

### 1. `users` Table
Stores registered student and administrator credentials.
```sql
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    email TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,           -- Salted BCrypt hash
    role TEXT NOT NULL DEFAULT 'student'  -- 'student' or 'admin'
);
```

### 2. `items` Table
Stores reported belongings with location, timestamps, and attached photos.
```sql
CREATE TABLE IF NOT EXISTS items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    description TEXT NOT NULL,
    type TEXT NOT NULL,               -- 'lost' or 'found'
    location TEXT NOT NULL,
    status TEXT DEFAULT 'active',     -- 'active', 'claimed', 'returned'
    user_id INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    image_path TEXT DEFAULT '',       -- Relative filename in uploads/ (e.g. item_1788508899527_1.png)
    FOREIGN KEY (user_id) REFERENCES users(id)
);
```

### 3. `sessions` Table
Manages active authentication session tokens.
```sql
CREATE TABLE IF NOT EXISTS sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    token TEXT NOT NULL UNIQUE,       -- 128-bit hex authorization token
    FOREIGN KEY (user_id) REFERENCES users(id)
);
```

---

## 📡 REST API Reference

Base URL: `http://127.0.0.1:18080`

### Authentication Endpoints

| Method | Endpoint | Auth | Description | Payload Example |
| :--- | :--- | :---: | :--- | :--- |
| `POST` | `/api/users/register` | No | Register a new user | `{"name":"John Doe","email":"john@campus.edu","password":"password123","role":"student"}` |
| `POST` | `/api/users/login` | No | Authenticate user & issue session token | `{"email":"john@campus.edu","password":"password123"}` |

**Successful Login Response (200 OK):**
```json
{
  "message": "Login successful",
  "token": "7fb4a119796af4da776bb61071c46b34",
  "user": {
    "id": 10,
    "name": "John Doe",
    "email": "john@campus.edu",
    "role": "student"
  }
}
```

### Item Endpoints

| Method | Endpoint | Auth | Description | Payload / Notes |
| :--- | :--- | :---: | :--- | :--- |
| `GET` | `/api/items` | No | List all active items | Returns `[{"id":1,"title":"...","image_path":"..."},...]` |
| `GET` | `/api/items/:id` | No | Fetch single item details by ID | Returns single item object (404 if not found) |
| `POST` | `/api/items` | Bearer | Report a new item | Supports JSON **or** `multipart/form-data` with `image` file |
| `PUT` | `/api/items/:id` | Bearer | Update item status | `{"status":"claimed"}` |
| `DELETE` | `/api/items/:id` | Bearer | Delete item & its photo file | Allowed only if owner (`user_id`) or `admin` role |
| `GET` | `/api/admin/items` | Bearer | Audit full item table | Allowed only for `admin` role |
| `GET` | `/uploads/:filename` | No | Static image serving | Returns raw binary image (`image/png`, `image/jpeg`) |

---

## ⚙️ Prerequisites & Environment Setup

### Recommended Toolchain (Windows)
- **C++ Compiler**: GCC 13.1.0 (MinGW-w64 64-bit)
- **Qt SDK**: Qt 6.11.2 MinGW 64-bit (QtWidgets, QtNetwork, QtGui, QtCore)
- **CMake**: Version 3.15 or newer
- **SQLite3**: Pre-installed or provided via Anaconda / MinGW libraries

---

## 🚀 Building & Running the Project

### 1. Start the Backend Server

The backend requires the MinGW runtime in your system `PATH`. Run from the project root:

```powershell
# Set environment PATH to include compiler & SQLite3 dependencies
$env:PATH = "C:\Users\Lenovo\anaconda3\Library\mingw-w64\bin;C:\Users\Lenovo\anaconda3\Library\bin;$env:PATH"

# Build with CMake
cmake -B build_native -S . -G "MinGW Makefiles"
cmake --build build_native --target LostFoundBackend

# Run the executable (runs on http://0.0.0.0:18080)
.\build_native\LostFoundBackend.exe
```

When started, the console will display:
```
Database connected successfully!
Database tables are ready!
[INFO] Crow/master server is running at http://0.0.0.0:18080 using 18 threads
```

---

### 2. Build & Launch the Qt GUI Client

#### Option A: Using Qt Creator (Recommended)
1. Open **Qt Creator**.
2. Select **Open File or Project** and choose `gui/LostFoundGUI.pro` (or `gui/CMakeLists.txt`).
3. Select your **Qt 6.x MinGW 64-bit** Kit.
4. Click **Run** (`Ctrl + R`).

#### Option B: Using Command Line (`mingw32-make`)
```powershell
# Set PATH for Qt 6 tools and MinGW
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.2\mingw_64\bin;$env:PATH"

# Build via make in the debug directory
mingw32-make -C gui\build\Desktop_Qt_6_11_2_MinGW_64_bit_Debug

# Launch the GUI client
.\gui\build\Desktop_Qt_6_11_2_MinGW_64_bit_Debug\debug\LostFoundGUI.exe
```

---

## 🛡️ User Roles & Security

| Role | Browse & Search | Report Item | Delete Item | Admin Console |
| :--- | :---: | :---: | :---: | :---: |
| **Guest (Unauthenticated)** | ✅ Yes | ❌ (Redirects to Login) | ❌ No | ❌ (Access Denied) |
| **Student** | ✅ Yes | ✅ Yes (JSON or Photo) | ✅ Own Items Only | ❌ (Access Denied) |
| **Administrator** | ✅ Yes | ✅ Yes (JSON or Photo) | ✅ Any Item | ✅ Yes (Full Registry) |

- Passwords are encrypted using salted **BCrypt** before insertion.
- The `DELETE /api/items/:id` endpoint checks token claims against item `user_id`. Non-owners receive `HTTP 403 Forbidden` (`"You can only delete your own reported items"`).
- Image uploads are validated on size (<= 5 MB), extension (`.png`, `.jpg`, `.jpeg`), and filename sanitized to prevent directory traversal attacks (`..`, `/`, `\`).

---

## 🎨 Visual Design & Assets

- **Environment Canvas**: Uses the high-resolution **GEHU Bhimtal campus photograph** (`1000 × 750`), embedded directly through the Qt Resource System (`:/assets/resources/images/campus-background.png`).
- **Aspect-Preserving Scaling**: Centered and scaled with `Qt::KeepAspectRatioByExpanding` and smooth bilinear filtering (`QPainter::SmoothPixmapTransform`).
- **Soft Neutral Overlay**: A subtle `rgba(248, 250, 252, 0.84)` translucent veil ensures all text, buttons, and inputs maintain AAA readability while preserving the campus atmosphere.
- **Modern Typography**: Standardized on clean `Segoe UI` fonts with high-contrast color palettes.
- **Interactive States**: Animated KPI counters, item card hover borders, and custom toast notifications.

---

## ❓ Troubleshooting & FAQs

### 1. GUI shows "Unable to connect to campus server"
- Ensure the backend server is running in a terminal at `http://127.0.0.1:18080`.
- Verify firewall permissions for port `18080`.

### 2. "Permission denied" during GUI compilation
- If you see `ld.exe: cannot open output file debug/LostFoundGUI.exe: Permission denied`, an existing instance of `LostFoundGUI.exe` is still running. Close it or run:
  ```powershell
  Get-Process -Name "LostFoundGUI*" | Stop-Process -Force
  ```

### 3. Missing DLLs when running backend
- Ensure your `PATH` includes the MinGW library directories containing `libsqlite3-0.dll`, `libgcc_s_seh-1.dll`, and `libstdc++-6.dll`.

---

## 📄 License

This project is developed for educational and campus property recovery management purposes. All rights reserved.

