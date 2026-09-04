# 🎓 Campus Lost & Found Management System

A C++-based campus Lost & Found Management System designed to help students, faculty, and staff report, discover, and manage lost and found belongings within a university campus.

The system consists of a **C++ Crow backend** and a **C++ Qt desktop GUI**, connected through REST APIs.

---

## 📌 Project Overview

Finding a lost item on a college campus can be difficult because there is often no centralized place to report or search for it.

The **Campus Lost & Found Management System** provides a centralized platform where users can:

- Register and log in
- Browse lost and found items
- Search and filter reported items
- View item details
- Report lost items
- Report found items
- Access a user dashboard
- Allow administrators to view the complete item catalog

The project is implemented using **C++ technologies**, with **Qt Widgets** for the graphical interface and **Crow** for the backend REST API.

---

## ✨ Features

### 👤 User Features

- User registration
- User login
- Authentication using tokens
- Browse all reported items
- Search items
- Filter Lost / Found items
- Filter by campus location
- View detailed information about an item
- Report a lost item
- Report a found item
- User dashboard
- Logout

### 👑 Admin Features

- Admin authentication
- Dedicated admin dashboard
- View all reported items
- View item status
- View reporting user ID
- View item creation time

### 🎨 User Interface

- Modern Qt Widgets interface
- Campus-focused design
- Responsive layouts
- Lost / Found status badges
- Search and filtering
- Loading states
- Success and error feedback
- Modern dashboard-style navigation

---

# 🏗️ System Architecture

```text
                 ┌──────────────────────────┐
                 │      Qt C++ GUI          │
                 │      Qt Widgets          │
                 └────────────┬─────────────┘
                              │
                              │ HTTP / REST API
                              ▼
                 ┌──────────────────────────┐
                 │      C++ Backend         │
                 │      Crow Framework      │
                 └────────────┬─────────────┘
                              │
                              ▼
                 ┌──────────────────────────┐
                 │       Database           │
                 │        SQLite             │
                 └──────────────────────────┘
