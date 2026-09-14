# 🎓 Campus Lost & Found Management System

A high-performance, enterprise-grade, full-stack **100% C++** university property recovery platform. Built with a modern **Qt 6 C++ Desktop Client** and a lightweight, multi-threaded **Crow C++ REST API Microservice** backed by **SQLite3** and **BCrypt**.

---

## 📌 Table of Contents

- [Overview](#-overview)
- [Key Features](#-key-features)
- [End-to-End Item Claim & Pickup Workflow](#-end-to-end-item-claim--pickup-workflow)
- [Architecture & Tech Stack](#-architecture--tech-stack)
- [Project Directory Structure](#-project-directory-structure)
- [Database Schema](#-database-schema)
- [REST API Reference](#-rest-api-reference)
- [Prerequisites & Environment Setup](#-prerequisites--environment-setup)
- [Building & Running the Project](#-building--running-the-project)
  - [1. Start the Backend Server](#1-start-the-backend-server)
  - [2. Build & Launch the Qt GUI Client](#2-build--launch-the-qt-gui-client)
- [User Roles & Security Permissions](#-user-roles--security-permissions)
- [Visual Design & Assets](#-visual-design--assets)
- [Troubleshooting & FAQs](#-troubleshooting--faqs)

---

## 🌟 Overview

The **Campus Lost & Found Management System** provides students, faculty, and campus administrators with an intuitive and secure way to report, browse, search, claim, verify, and physically recover misplaced belongings across campus. 

Unlike conventional web stacks, this application is engineered strictly in **modern C++ (C++17)**:
- **Zero Web Technologies**: No JavaScript, TypeScript, React, Node.js, Python, or heavy Chromium/Electron wrappers.
- **Native Desktop Performance**: Instant startup, minimal memory footprint (< 40 MB RAM), smooth UI animations, and robust multithreading.
- **RESTful Decoupled Architecture**: The Qt 6 GUI client communicates asynchronously with the Crow C++ microservice via JSON and multipart HTTP requests over `http://127.0.0.1:18080`.

---

## ✨ Key Features

### 🤝 Complete Item Claim & Physical Pickup Lifecycle
- **Ownership Verification Form**: Students submit detailed ownership proof (last seen location, distinctive marks, inner contents, extra proof) with a mandatory Student Code of Conduct affirmation.
- **Unique Claim Reference Tracking**: Generates permanent tracking codes (e.g. `CLM-2026-00101`).
- **Fraud Prevention Safeguards**: Only items of type `FOUND` can be claimed; reporting owners cannot claim their own items; duplicate active claims are prevented; returned items cannot be claimed again.
- **Interactive Student Tracking (`MyClaimsScreen`)**: Multi-tab dashboard (*All*, *Pending Review*, *Ready for Pickup*, *Handed Over*, *Rejected*) featuring a visual 3-step timeline.
- **Official Digital Pickup Pass**: When approved, students receive an emerald digital pass showing their claim reference, designated campus location (*Admin Block - Room 102*), operating hours, and mandatory Student ID reminder.
- **Admin Verification Console**: Side-by-side audit of the original item record against the claimant's answers with options to Approve (with custom pickup location), Reject (with required explanation), or Hand Over (with physical ID check).
- **Physical Handover & Inventory Resolution**: Finalizing handover marks the item as `returned` and auto-rejects any competing pending claims.
- **Accessible, High-Contrast UI**: Scoped modal dialogs (`QMessageBox`) styled with dark slate typography and royal blue buttons, ensuring 100% readability across Light and Dark operating system themes.

### 👤 User Profile Management & Clean Top Navbar
- **Profile Management (`ProfileDialog`)**: View user ID, role badge, generated avatar initial pill, and update name and email seamlessly via `PUT /api/users/me`.
- **Streamlined Top Navbar**: Clean university header (`Campus Lost & Found • University Portal`), primary navigation links, active user avatar chip, and instant logout.

### 🔐 Authentication & Session Management
- **User Registration & Login**: Validates credentials and email structure with password confirmation.
- **BCrypt Password Security**: Passwords salted and cryptographically hashed with Blowfish BCrypt prior to database persistence.
- **Token-Based Sessions**: Server generates 128-bit hex tokens stored in SQLite and persisted securely on the client via `QSettings`.
- **Role-Based Access Control (RBAC)**: Distinct permissions for `student` and `admin` roles.

### 📦 Item Reporting & Image Attachments
- **Dual Classification**: Report items as either **Lost** (`🔴 LOST`) or **Found** (`🟢 FOUND`).
- **Photo Upload**: Supports `multipart/form-data` image attachments (`.png`, `.jpg`, `.jpeg`) up to 5 MB.
- **Unique Filename Sanitization**: Saved as `uploads/item_<timestamp>_<counter>.<ext>`, guarding against path traversal attacks.
- **Static Image Serving**: Crow serves uploaded images with proper MIME types and HTTP cache headers (`Cache-Control: public, max-age=86400`).

### 🔍 Search, Filter & Campus Zones
- **Real-Time Search**: Instant keyword filtering across item title, description, and location.
- **Predefined Campus Locations**: Filter by campus zones (*Central Library*, *Main Cafeteria*, *Computer Lab*, *Auditorium*, *Sports Ground*, *Hostel Block*, *Parking Area*, etc.).
- **Category Filter Pills**: Quick toggle between *All Items*, *Lost Items*, and *Found Items*.

### 🗑 Ownership-Guarded Deletion
- **Granular Deletion Control**: Users can delete their own reported items; campus administrators can delete any item.
- **Cascade File Cleanup**: Deleting an item cleans up both the database record and the associated physical photo on disk.
- **Confirmation Guards**: Safeguarded by double-confirmation dialogs.

### 📊 Live Analytics & Dashboard
- **Campus KPI Cards**: Real-time metrics for *Total Listed Items*, *Active Lost Items*, *Found Items*, and *Reports This Week*.
- **Admin Claim KPI Cards**: Live counters for *Pending Review*, *Ready for Pickup*, *Handed Over*, and *Total Claims*.
- **Smooth Number Animations**: Animated counting transitions using `QVariantAnimation` with cubic easing.
- **Personal Dashboard**: Quick overview of student reports, plus an urgent alert banner when an approved claim is ready for pickup.

---

## 🔄 End-to-End Item Claim & Pickup Workflow

```
Student views FOUND item in Browse / Details
               │
               ▼
   [ Claim Verification Form ]
   - Where & when misplaced?
   - Unique identifying features & serials?
   - Internal contents & specific marks?
   - Student Code of Conduct declaration
               │
               ▼  POST /api/items/:id/claims
   [ Database: claims table ]
   - Status: 'pending'
   - Reference: CLM-2026-00101
               │
               ▼  GET /api/admin/claims
     [ Admin Review Console ]
   - Side-by-side verification: Item Record vs Claimant Proof
   - Claimant identity profile inspection
               │
      ┌────────┴────────┐
      ▼                 ▼
[ Approve Claim ]   [ Reject Claim ]
- Status: approved   - Status: rejected
- Location: Room 102 - Reason logged & shown to student
      │
      ▼  GET /api/users/me/claims
 [ Student Pickup Pass ]
- Digital Pickup Pass active in "My Claims"
- Reference code, office hours & Student ID reminder
- Urgent pickup banner on Home/Dashboard
      │
      ▼  (Student visits Admin Office in person)
 [ Admin Physical Handover ]
- Student ID verified at desk
- Handover checkbox confirmed
- PUT /api/admin/claims/:id/handover
      │
      ▼
- Claim Status: 'handed_over'
- Item Status: 'returned' (Permanently closed)
- Competing claims auto-rejected
```

---

## 🏗 Architecture & Tech Stack

```
┌──────────────────────────────────────────────────────────┐
│                   Qt 6 GUI Frontend                      │
│   (C++17, Qt Widgets, Qt Network, Qt Core, Segoe UI)     │
│       • Sidebar, Polished Navbar, Toast Notifications    │
│       • Responsive Grid & Asynchronous ImageLoader       │
│       • ClaimFormDialog, MyClaimsScreen, AdminConsole    │
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
│       • REST Controllers: user, item, claim              │
└────────────────────────────┬─────────────────────────────┘
                             │
                             ▼
┌──────────────────────────────────────────────────────────┐
│                  SQLite3 Database Engine                 │
│       • File: lost_found.db                              │
│       • Tables: users, items, sessions, claims           │
│       • Directory: uploads/ (attached images)            │
└──────────────────────────────────────────────────────────┘
```

| Layer | Component | Description |
| :--- | :--- | :--- |
| **Frontend Framework** | Qt 6 (6.11.2) / Qt Widgets | Native desktop UI with Segoe UI typography and scoped stylesheet styling. |
| **Networking (Client)** | `QNetworkAccessManager` | Asynchronous non-blocking HTTP REST client with typed callback dispatchers. |
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
│   │   └── database.cpp           # SQLite3 connection, schema creation, migrations (claims table)
│   ├── controllers/
│   │   ├── userController.h/.cpp  # User registration, login, profile view & update
│   │   ├── itemController.h/.cpp  # CRUD, multipart upload, ownership check, image serving
│   │   └── claimController.h/.cpp # Claim submission, verification, approval, rejection, handover
│   ├── middleware/
│   │   ├── authMiddleware.h       # Bearer token verification interface
│   │   └── authMiddleware.cpp     # Session token validation against database
│   └── routes/
│       ├── userRoutes.cpp         # Routes: /api/users/register, login, me
│       ├── itemRoutes.cpp         # Routes: /api/items, /uploads/<string>, /api/admin/items
│       └── claimRoutes.h/.cpp     # Routes: /api/items/:id/claims, /api/users/me/claims, /api/admin/claims
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
        │   ├── Claim.h            # Claim & ClaimStats models with status helpers & JSON parsing
        │   └── CampusLocations.h  # Centralized campus location definitions
        ├── auth/
        │   ├── AuthManager.h      # Session singleton with QSettings persistence
        │   └── AuthManager.cpp
        ├── network/
        │   ├── ApiClient.h        # REST client wrapping QNetworkAccessManager (items, claims, users)
        │   ├── ApiClient.cpp
        │   ├── ImageLoader.h      # Asynchronous image downloader with cache & QPointer safety
        │   └── ImageLoader.cpp
        ├── components/
        │   ├── NavbarWidget.h/.cpp    # Polished top bar with profile chip and navigation
        │   ├── SidebarWidget.h/.cpp   # Deep navy sidebar with "My Claims" & "Admin Console" tabs
        │   ├── ItemCardWidget.h/.cpp  # Responsive card with badge, thumbnail, and action buttons
        │   ├── StatCardWidget.h/.cpp  # Metric KPI cards with smooth counting animations
        │   ├── ToastWidget.h/.cpp     # Animated floating notification toasts
        │   ├── AvatarHelper.h/.cpp    # User avatar initial circle renderer
        │   └── EmptyStateWidget.h/.cpp# Clean placeholder illustration when lists are empty
        └── views/
            ├── MainWindow.h/.cpp      # Root window with background canvas and QStackedWidget
            ├── HomeScreen.h/.cpp      # Hero card, campus overview, recent item listings
            ├── BrowseScreen.h/.cpp    # Full search bar, location dropdown, category filter grid
            ├── ReportScreen.h/.cpp    # Multipart item submission form with photo preview
            ├── ItemDetailsDialog.h/.cpp # Item details popup with Claim action & status badge
            ├── ClaimFormDialog.h/.cpp # Student claim verification modal with validation
            ├── MyClaimsScreen.h/.cpp  # Student claim tracking dashboard with Digital Pickup Pass
            ├── AdminClaimReviewDialog.h/.cpp # Side-by-side admin claim review & handover dialog
            ├── ProfileDialog.h/.cpp   # User profile viewer and editor
            ├── LoginScreen.h/.cpp     # Authentication form
            ├── RegisterScreen.h/.cpp  # Account registration form
            ├── DashboardScreen.h/.cpp # Student dashboard with active pickup alerts
            ├── AdminScreen.h/.cpp     # Role-guarded admin console (Claims & Items tabs)
            └── AccessDeniedScreen.h/.cpp # Security barrier for restricted routes
```

---

## 🗄 Database Schema

The SQLite database (`lost_found.db`) includes automatic initialization and migrations on startup:

### 1. `users` Table
Stores registered student and administrator credentials.
```sql
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    email TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,               -- Salted BCrypt hash
    role TEXT NOT NULL DEFAULT 'student'  -- 'student' or 'admin'
);
```

### 2. `items` Table
Stores reported belongings with location, status, timestamps, and attached photos.
```sql
CREATE TABLE IF NOT EXISTS items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    description TEXT NOT NULL,
    type TEXT NOT NULL,                   -- 'lost' or 'found'
    location TEXT NOT NULL,
    status TEXT DEFAULT 'active',         -- 'active', 'claimed', 'returned'
    user_id INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    image_path TEXT DEFAULT '',           -- Relative filename in uploads/
    FOREIGN KEY (user_id) REFERENCES users(id)
);
```

### 3. `sessions` Table
Manages active authorization session tokens.
```sql
CREATE TABLE IF NOT EXISTS sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    token TEXT NOT NULL UNIQUE,           -- 128-bit hex authorization token
    FOREIGN KEY (user_id) REFERENCES users(id)
);
```

### 4. `claims` Table
Tracks ownership claims, verification responses, review decisions, and physical handovers.
```sql
CREATE TABLE IF NOT EXISTS claims (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    claim_reference TEXT NOT NULL UNIQUE,
    item_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'pending',   -- 'pending', 'approved', 'rejected', 'handed_over'
    last_seen_location TEXT NOT NULL,
    unique_identifying_details TEXT NOT NULL,
    item_contents TEXT NOT NULL,
    additional_details TEXT DEFAULT '',
    rejection_reason TEXT DEFAULT '',
    pickup_location TEXT DEFAULT 'Admin Block - Room 102',
    reviewed_by INTEGER,
    handover_admin_id INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    reviewed_at DATETIME,
    approved_at DATETIME,
    handed_over_at DATETIME,
    FOREIGN KEY (item_id) REFERENCES items(id),
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (reviewed_by) REFERENCES users(id),
    FOREIGN KEY (handover_admin_id) REFERENCES users(id)
);
```

---

## 📡 REST API Reference

Base URL: `http://127.0.0.1:18080`

### Authentication & User Profile Endpoints

| Method | Endpoint | Auth | Description | Payload Example |
| :--- | :--- | :---: | :--- | :--- |
| `POST` | `/api/users/register` | No | Register a new user | `{"name":"John Doe","email":"john@campus.edu","password":"password123","role":"student"}` |
| `POST` | `/api/users/login` | No | Authenticate user & issue session token | `{"email":"john@campus.edu","password":"password123"}` |
| `GET` | `/api/users/me` | Bearer | Fetch current user profile | Returns `{ "id": 1, "name": "...", "email": "...", "role": "student" }` |
| `PUT` | `/api/users/me` | Bearer | Update user profile details | `{"name":"Johnathan Doe","email":"new_email@campus.edu"}` |

### Item Endpoints

| Method | Endpoint | Auth | Description | Payload / Notes |
| :--- | :--- | :---: | :--- | :--- |
| `GET` | `/api/items` | No | List all active items | Returns `{"items": [...], "count": N}` |
| `GET` | `/api/items/:id` | No | Fetch single item details | Returns item object (404 if not found) |
| `POST` | `/api/items` | Bearer | Report a new item | Supports JSON **or** `multipart/form-data` with `image` file |
| `PUT` | `/api/items/:id` | Bearer | Update item status | `{"status":"claimed"}` |
| `DELETE` | `/api/items/:id` | Bearer | Delete item & its photo | Allowed only if owner (`user_id`) or `admin` role |
| `GET` | `/api/admin/items` | Bearer | Audit full item table | Admin-only registry |
| `GET` | `/uploads/:filename` | No | Static image serving | Returns raw binary image (`image/png`, `image/jpeg`) |

### Claim & Pickup Workflow Endpoints

| Method | Endpoint | Auth | Description | Payload / Notes |
| :--- | :--- | :---: | :--- | :--- |
| `POST` | `/api/items/:id/claims` | Bearer | Submit ownership claim | `{"last_seen_location":"...","unique_identifying_details":"...","item_contents":"...","additional_details":"..."}` |
| `GET` | `/api/users/me/claims` | Bearer | Get current user's claims | Returns claims list joined with item information |
| `GET` | `/api/items/:id/my-claim` | Bearer | Check claim state for an item | Returns `{"has_claim": true, "claim": {...}}` |
| `GET` | `/api/claims/:id` | Bearer | Fetch full claim details | Accessible by claimant or administrator |
| `GET` | `/api/admin/claims` | Bearer | List claims & KPI stats | Admin-only. Query params: `?status=pending` |
| `PUT` | `/api/admin/claims/:id/approve` | Bearer | Approve claim & set pickup | `{"pickup_location": "Admin Block - Room 102"}` |
| `PUT` | `/api/admin/claims/:id/reject` | Bearer | Reject claim with reason | `{"rejection_reason": "Description does not match serial number"}` |
| `PUT` | `/api/admin/claims/:id/handover` | Bearer | Complete physical handover | Marks claim `handed_over`, item `returned`, auto-rejects competitors |

---

## ⚙️ Prerequisites & Environment Setup

### Recommended Toolchain (Windows)
- **C++ Compiler**: GCC 13.1.0+ (MinGW-w64 64-bit)
- **Qt SDK**: Qt 6.11.2 MinGW 64-bit (`QtWidgets`, `QtNetwork`, `QtGui`, `QtCore`)
- **CMake**: Version 3.15 or newer
- **SQLite3**: Pre-installed or available via MinGW/Anaconda library paths

---

## 🚀 Building & Running the Project

### 1. Start the Backend Server

The Crow C++ backend runs as a standalone daemon on port `18080`.

```powershell
# Set PATH to MinGW runtime tools
$env:PATH = "C:\Users\Lenovo\anaconda3\Library\mingw-w64\bin;C:\Users\Lenovo\anaconda3\Library\bin;$env:PATH"

# Build with CMake
cmake -B build_native -S . -G "MinGW Makefiles"
cmake --build build_native --target LostFoundBackend

# Launch the server (http://127.0.0.1:18080)
.\build_native\LostFoundBackend.exe
```

Console Output:
```
Database connected successfully!
Database tables are ready!
[INFO] Crow/master server is running at http://0.0.0.0:18080 using 18 threads
```

---

### 2. Build & Launch the Qt GUI Client

#### Option A: Using Qt Creator (Recommended)
1. Open **Qt Creator**.
2. Click **Open File or Project** and select `gui/LostFoundGUI.pro` (or `gui/CMakeLists.txt`).
3. Select your **Qt 6.x MinGW 64-bit** Kit.
4. Click **Run** (`Ctrl + R`).

#### Option B: Using Command Line (`mingw32-make`)
```powershell
# Set PATH for Qt 6 tools and MinGW compiler
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.2\mingw_64\bin;$env:PATH"

# Compile inside the build directory
mingw32-make -C gui\build\Desktop_Qt_6_11_2_MinGW_64_bit_Debug -j4

# Launch the compiled executable
.\gui\build\Desktop_Qt_6_11_2_MinGW_64_bit_Debug\debug\LostFoundGUI.exe
```

---

## 🛡️ User Roles & Security Permissions

| Action | Guest (Unauthenticated) | Student | Campus Administrator |
| :--- | :---: | :---: | :---: |
| **Browse & Search Items** | ✅ Yes | ✅ Yes | ✅ Yes |
| **View Item Details** | ✅ Yes | ✅ Yes | ✅ Yes |
| **Report Lost or Found Item** | ❌ (Redirects to Login) | ✅ Yes (JSON / Multipart Photo) | ✅ Yes (JSON / Multipart Photo) |
| **Submit Ownership Claim** | ❌ (Redirects to Login) | ✅ Yes (Found items, not self-reported) | ✅ Yes |
| **Track My Claims & Pickup Pass** | ❌ No | ✅ Yes (`MyClaimsScreen`) | ✅ Yes |
| **Delete Item** | ❌ No | ✅ Own Reported Items Only | ✅ Any Item in System |
| **Approve / Reject Claims** | ❌ No | ❌ Forbidden | ✅ Yes (`AdminClaimReviewDialog`) |
| **Authorize Physical Handover** | ❌ No | ❌ Forbidden | ✅ Yes (With Student ID Verification) |
| **Admin Inventory & Claims Console** | ❌ No | ❌ Forbidden | ✅ Yes (`AdminScreen`) |
| **Update User Profile** | ❌ No | ✅ Own Profile Only | ✅ Own Profile Only |

---

## 🎨 Visual Design & Assets

- **Environment Canvas**: Uses the high-resolution **GEHU Bhimtal campus photograph** (`1000 × 750`), embedded directly through the Qt Resource System (`:/assets/resources/images/campus-background.png`).
- **Aspect-Preserving Scaling**: Centered and scaled with `Qt::KeepAspectRatioByExpanding` and smooth bilinear filtering (`QPainter::SmoothPixmapTransform`).
- **Soft Neutral Overlay**: A subtle `rgba(248, 250, 252, 0.84)` translucent veil ensures all cards, dialogs, and typography maintain AAA readability while preserving the campus atmosphere.
- **Accessible Typography & Scoped Stylesheets**: All dialogs use scoped stylesheets (`#ClaimFormDialog`, `#ItemDetailsDialog`, `#AdminClaimReviewDialog`) and high-contrast styling (`#0F172A` text, `#2563EB` action buttons), preventing white-on-white text rendering issues across all OS display settings.
- **Interactive Micro-Interactions**: Animated counting metrics, status badges, pickup pass card borders, and animated floating toasts.

---

## ❓ Troubleshooting & FAQs

### 1. GUI shows "Unable to connect to campus server"
- Confirm the Crow backend is active at `http://127.0.0.1:18080`.
- Verify port `18080` is not blocked by local firewall policies.

### 2. "Permission denied" during GUI compilation
- If you see `ld.exe: cannot open output file debug/LostFoundGUI.exe: Permission denied`, an instance of `LostFoundGUI.exe` is currently running. Terminate it via PowerShell:
  ```powershell
  Get-Process -Name "LostFoundGUI*" | Stop-Process -Force
  ```

### 3. Missing DLLs when starting the backend
- Ensure your `PATH` contains the MinGW runtime folder containing `libsqlite3-0.dll`, `libgcc_s_seh-1.dll`, and `libstdc++-6.dll`.

### 4. "You cannot claim an item you reported"
- The system prevents reporting students from filing claims against their own listings. Use a separate student test account to verify claim submission.

---

## 📄 License

This project is developed for educational and campus property recovery management purposes. All rights reserved.
