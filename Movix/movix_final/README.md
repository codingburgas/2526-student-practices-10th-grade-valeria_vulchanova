# Movix v4.0 – Cinema Management Suite
### Built with C++17 + raylib 5.0 | Visual Studio 2022 | x64

---

## 🏗️ Three-Tier Architecture

```
Movix/
├── dal/          DATA ACCESS LAYER    (file I/O only, no UI/BLL knowledge)
│   ├── types.h               shared data types
│   ├── user_repo.h/.cpp      users.txt CRUD
│   ├── movie_repo.h/.cpp     movies.txt CRUD + seeding
│   ├── reservation_repo.h/.cpp
│   └── rating_repo.h/.cpp
├── bll/          BUSINESS LOGIC LAYER (rules, validation, services)
│   ├── session.h/.cpp        current user state
│   ├── auth_service.h/.cpp   login / register / change password
│   ├── movie_service.h/.cpp  search, filter, poster, rating
│   ├── booking_service.h/.cpp reserve / cancel / history
│   └── stats_service.h/.cpp  revenue, occupancy, charts
├── ui/           PRESENTATION LAYER   (raylib screens and widgets)
│   ├── theme.h               colours, fonts, SW/SH constants
│   ├── widgets.cpp           all widget & chrome definitions
│   ├── screens.h             forward declarations
│   ├── login_screen.cpp      LOGIN + REGISTER tabs
│   ├── movie_screen.cpp      catalogue, search, ratings, logout
│   ├── seat_screen.cpp       hall grid, tier pricing, back button
│   ├── admin_screen.cpp      4 tabs: Users/Reservations/Movies/Statistics
│   ├── profile_screen.cpp    change password, booking stats
│   └── history_screen.cpp    all reservations + cancel seats
└── main.cpp
```

---

## ✨ Features

### For Users
| Feature | Detail |
|---------|--------|
| 🔐 Register / Login | Tabs on login screen, password validation |
| 🎬 Movie Catalogue | 3-column grid, animated cards |
| 🔍 Search | Live search by title or genre |
| 🎟️ Seat Selection | 8×10 hall, Front/STD/VIP tier pricing |
| ⭐ Rate Movies | 1–5 star interactive widget, community avg shown |
| 📋 Booking History | All your seats, cancel individual reservations |
| 👤 Profile | Change password, spending stats, rated movies list |
| 🚪 Logout | Button in movie screen header |
| ← Back | Button on every sub-screen + ESC key |

### For Admin (admin / 1234)
| Feature | Detail |
|---------|--------|
| 👥 Users | List all users with reservation count, delete |
| 🎭 Reservations | Clear per movie or clear all |
| 🎬 Movies | Add / Delete movies, 8 colour themes |
| 🖼️ Movie Posters | SET IMAGE button → Windows file picker → auto-copy |
| 📊 Statistics | KPI cards, bar chart, revenue breakdown table |

---

## 🚀 Quick Start

1. Open **Movix.sln** in Visual Studio 2022
2. Right-click solution → **Retarget** (if prompted)
3. `Ctrl+Shift+B` — Build
4. `F5` — Run

Fonts (optional — fallback to default if missing):
- `Cinzel-Bold.ttf` → fonts.google.com/specimen/Cinzel
- `Raleway-Regular.ttf` and `Raleway-Bold.ttf` → fonts.google.com/specimen/Raleway

Place .ttf files in the project root before building.

---

## 📁 Data Files (auto-created next to .exe)

```
data/users.txt          username|password|0
data/movies.txt         id|title|genre|...|colours
data/reservations.txt   movieIdx|row|col|username
data/ratings.txt        username|movieId|stars
posters/movie_N.jpg     poster images (set via Admin → Movies → SET IMAGE)
```

## 🎭 Admin credentials
Login: `admin` / Password: `1234` (built-in, not stored in file)

## 🏛️ Seat Pricing
- Rows A-B (Front): −20% of base price
- Rows C-F (Standard): base price
- Rows G-H (VIP): +25% of base price
