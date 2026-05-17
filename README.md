<div align="center">

# 🚀 CTracker

**A high-performance, native desktop application for productivity tracking, learning management, and focused execution.**

[![C++17](https://img.shields.io/badge/C++-17-blue.svg?style=flat-square&logo=c%2B%2B)](https://isocpp.org/)
[![Qt6](https://img.shields.io/badge/Qt-6.0+-41CD52.svg?style=flat-square&logo=qt)](https://www.qt.io/)
[![SQLite](https://img.shields.io/badge/SQLite-3-003B57.svg?style=flat-square&logo=sqlite)](https://www.sqlite.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20+-064F8C.svg?style=flat-square&logo=cmake)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://opensource.org/licenses/MIT)

*A bespoke Dark Industrial Desktop UI designed for zero distractions and maximum focus.*

---

<!-- Placeholder for Hero Image or GIF -->
> 🖼️ *[Drop an interface screenshot or GIF demo here]*

</div>

## 📖 Overview

CTracker is a locally-hosted, standalone native application designed to unify your workflow. Whether you're tracking coursework, managing long-term projects, running Pomodoro focus sessions, or reviewing your detailed activity analytics, CTracker puts it all at your fingertips without the overhead of electron-based apps or cloud subscriptions.

## ✨ Core Features

*   **📚 Course & Project Mastery:** Create nested hierarchies. Break down high-level Projects and Courses into actionable Units, Sessions, and Tasks.
*   **⏱️ Built-in Pomodoro Engine:** A native desktop Pomodoro tracker that links directly to your active tasks and automatically logs your focused time.
*   **📅 Unified Calendar & Daily View:** Visualize tasks, deadlines, and completed Pomodoro sessions on an interactive calendar. Drill down into specific days to see detailed time-tracking logs.
*   **📊 Interactive Analytics:** Deep-dive into your productivity using QtCharts. View time spent across different categories, activity logs, and progress overviews.
*   **🎨 Dark Industrial UI:** A highly polished, bespoke UI utilizing native Qt Widgets customized with a distraction-free QSS theme (`background #1a1d24`).

## 🛠️ Tech Stack & Architecture

CTracker prioritizes speed, native OS integration, and low memory footprint.

*   **Language:** C++17
*   **Framework:** Qt 6 (Core, Widgets, Sql, Charts, Svg)
*   **Database:** SQLite (Managed via a thread-safe, robust `DatabaseManager` singleton)
*   **Build System:** CMake
*   **Architecture:** Strict Model–View separation. The UI leverages XML UI files (`.ui`) combined with dynamic C++ custom widgets, and interfaces with the backend exclusively via standard Qt Item Models (`QAbstractTableModel`, `QAbstractListModel`) and strong Signal/Slot wiring.

> [!NOTE]
> **Design Prototype:** The `Design/` directory contains a React + Tailwind prototype. This serves **exclusively as a visual reference** for spacing, colors, and layout structure. The production application translates these design concepts entirely into pure C++ and QSS.

## 📂 Project Structure

```text
├── CTracker/             # Main Source Code & Build Targets
│   ├── assets/           # QSS Stylesheets and SVG Icons
│   ├── include/          # C++ Headers (Grouped by module)
│   ├── src/              # C++ Implementations
│   └── tests/            # QtTest Framework Unit Tests
├── data/                 # Sample DB mapping, scripts & user data
├── Design/               # Visual Reference (React Prototype)
└── README.md             # This file
```

## 🚀 Getting Started

### Prerequisites
*   [CMake](https://cmake.org/download/) (3.20 or newer)
*   C++17 standard compatible compiler (MSVC / GCC / Clang)
*   [Qt 6 Toolkit](https://www.qt.io/) (requires `Core`, `Widgets`, `Sql`, `Charts`, and `Svg` modules)

### 1. Build the Project

Clone the repository and build via the provided PowerShell scripts or standard CMake commands:

```bash
git clone https://github.com/your-username/CTracker.git
cd CTracker

# Quick Build via script (Windows)
.\test-build.ps1
```

*Or manually:*
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 2. Deploy and Run (Windows)

Before running the executable, ensure the required Qt DLLs are in your build path:

```powershell
# Deploys active Qt environment DLLs to the build folder
.\deploy-qt-dlls.ps1

# Launch the Application
.\launch-ctracker.ps1
```

## 🧪 Testing

The repository features comprehensive unit tests using the Qt Test suite.

```powershell
cd CTracker
# Execute all defined tests
.\run-tests.ps1
```

## 🚦 Roadmap & Current Status

**Current Version:** `v0.0.8`
*   ✅ Core database layer setup
*   ✅ Architecture mapping (Model-View)
*   ✅ Extensive testing suite added
*   🚧 **Known Issue:** Application occasionally crashes when modifying the database via UI models. (Fix actively in progress).

## 📜 License

This project is licensed under the MIT License - see the LICENSE file for details.

---
<div align="center">
  <i>Built with focus for focused work.</i>
</div>
