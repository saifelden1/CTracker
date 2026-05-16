# Phase 0 — Source-Tree Reorganization, Explained

> ### 🟢 Plain-English note before you start
>
> Phase 0 is about **moving files around**. No code logic changes — just
> moving `.h` and `.cpp` files into tidier folders so the project is
> easier to navigate as it grows. If you've ever reorganised photos into
> folders by year, it's the same idea, just for source code.

> **Audience.** This document is written so a student who has never seen
> the project can follow along and *replicate* the work — not just read
> what we did, but understand **why** every decision was made and **how**
> to verify each step ended up correct. If you can read C++ and have
> built a small project with CMake before, you have everything you need.
>
> **What Phase 0 is.** A pure file-system reorganization: 28 source files
> are moved out of a flat `include/` + `src/` pair into nine
> *feature-grouped* folders. No behaviour changes, no logic added or
> removed. Done correctly, the program builds identically before and
> after.
>
> **Why it matters.** It's the only Phase where the cost is small and the
> payoff compounds: every future widget, view, model, and test lands in
> the right folder on first try. Doing this later — once the codebase is
> twice as large — is significantly more painful.

---

## 0. Mental model: feature-grouped vs. type-grouped

There are two common ways to organize a project:

| Layout | Example |
|---|---|
| **Type-grouped** (what we *had*) | `include/*.h` and `src/*.cpp` — one big bucket per file type. |
| **Feature-grouped** (what we *moved to*) | `core/`, `courses/`, `analytics/`, … — files that change together, live together. |

Type-grouped layouts look tidy when the project is tiny. Feature-grouped
layouts scale — when you open `analytics/` you see *every* file that
makes the analytics view work, and only those files. The Karpathy-style
rule of thumb: **the folder structure should answer the question "what
parts of the app would I touch to change behaviour X?"**

The convention we adopted (documented at the top of `.ai/specs/design.md`):

> *One folder per top-level feature; `core/` for the data layer;
> `shared/` for cross-feature widgets and chrome.*

Concrete folder map:

```
include/
├── core/        # data layer: DB, structs, importer/exporter
├── shared/      # cross-feature: MainWindow, SideNav, CircularProgressBar, …
├── courses/     # everything specific to the "Courses" experience
├── projects/    # …same, for Projects
├── todos/       # (placeholder — populated in Phase 6/7)
├── pomodoro/    # (placeholder)
├── analytics/   # heatmap, models, analytics view
├── calendar/    # (placeholder)
└── settings/    # SettingsView
src/             # mirrors include/ 1:1
```

The mirror between `include/` and `src/` is deliberate: every `Foo.h` in
`include/feature/` has its companion `Foo.cpp` in `src/feature/`. If they
ever diverge, you've forgotten to move one of the pair.

`main.cpp` stays in `src/` root — it's the program entry point, not a
feature.

---

## 1. Sub-task 0.1 — Create the folder skeleton

### Goal

Make every destination folder exist *before* you start moving files.
Git is happy to `git mv` into a folder that doesn't yet exist (it
creates it implicitly), but doing it as an explicit step means you can
review the skeleton, commit it on its own if you like, and notice
mistakes earlier.

### Commands (bash, run from `CTracker/`)

```bash
for f in core shared courses projects todos pomodoro analytics calendar settings; do
  mkdir -p "include/$f" "src/$f"
done
```

### `.gitkeep` — why and where

Git tracks files, not directories. A directory with no tracked file in
it disappears from the index. Several of our new folders are
intentionally empty *for now* (Phase 6/7 will populate them). To keep
them in the repo as scaffolding, we drop a zero-byte `.gitkeep` marker:

```bash
for f in todos pomodoro calendar; do
  touch "include/$f/.gitkeep" "src/$f/.gitkeep"
done
```

**Subtlety we caught here.** The tasks doc originally listed `settings`
as one of the empty folders, but `SettingsView.h/.cpp` already exists
and ends up *in* `settings/` after Phase 0.2. Likewise, `projects/`
receives the `ProjectDetailView.h` alias header. So those two folders
**don't** need a `.gitkeep` — they'll be populated by the moves in the
very next step. This is the kind of detail that's easy to copy-paste
wrong from a plan; always cross-check the plan against reality.

### How to verify

```bash
ls include/   # 9 entries, all directories
ls src/       # 9 entries, all directories
```

Empty-by-design folders should show `.gitkeep` when you `ls -la` them.

---

## 2. Sub-task 0.2 — Move the files (`git mv` matters)

### Why `git mv`, not plain `mv`?

Plain `mv` looks identical to git as **delete old + create new**. The
blame history attached to the old file is severed. With `git mv`
(or `mv` followed by `git add`/`git rm` — git's rename detector usually
catches this when changes are small), git records the move as a rename
and `git log --follow path/to/Foo.h` will trace the file back to its
original birth.

This matters in practice: when you wonder months from now "why is this
weird branch in `updateSessionTaskProgress` here?", `git blame` should
point at the original commit that introduced it, not the rearrangement
commit that just shuffled folders.

### The move table

From Task 0.2 in `tasks.md`:

| File | New folder |
|---|---|
| `DataStructures.h`, `DatabaseManager.h/.cpp`, `DataImporter.h/.cpp`, `DataExporter.h/.cpp` | `core/` |
| `CircularProgressBar.h/.cpp`, `MainWindow.h/.cpp`, `SideNavigationBar.h/.cpp`, `HomeDashboard.h/.cpp` | `shared/` |
| `EntityCard.h/.cpp`, `UnitExpandableWidget.h/.cpp`, `SessionTaskRow.h/.cpp`, `EntityDetailView.h/.cpp`, `CourseDetailView.h` | `courses/` |
| `ProjectDetailView.h` | `projects/` |
| `ContributionHeatmap.h/.cpp`, `ActivityLogModel.h/.cpp`, `AnalyticsView.h/.cpp` | `analytics/` |
| `SettingsView.h/.cpp` | `settings/` |
| `main.cpp` | stays in `src/` root |

### Commands

The pattern: one `git mv` per file. Group them by feature folder so a
single failure stops the batch at a known boundary:

```bash
# core/
git mv include/DataStructures.h    include/core/DataStructures.h
git mv include/DatabaseManager.h   include/core/DatabaseManager.h
git mv src/DatabaseManager.cpp     src/core/DatabaseManager.cpp
git mv include/DataImporter.h      include/core/DataImporter.h
git mv src/DataImporter.cpp        src/core/DataImporter.cpp
git mv include/DataExporter.h      include/core/DataExporter.h
git mv src/DataExporter.cpp        src/core/DataExporter.cpp
```

…and so on for the remaining feature folders.

### `CourseDetailView.h` and `ProjectDetailView.h` are **alias headers**

These two headers do not pair with a `.cpp`. They exist only to give a
"semantic" name to `EntityDetailView` — `CourseDetailView` is what
`MainWindow` says it instantiates for course detail screens, even though
internally it's the same class. Phase 7 will eventually split them into
real subclasses. For now they're one-line `#include "courses/EntityDetailView.h"`
files. Treat them like any other header for purposes of the move.

### How to verify

```bash
git status --short
```

You should see a long list of `R  <old> -> <new>` lines. `R` = rename
detected. If you see `D` (delete) + `??` (untracked) pairs instead,
something went wrong — the file moved on disk but git didn't follow.

---

## 3. Sub-task 0.2b — Rewrite `#include` directives

### The new include style

With `include/` still the single include root (we didn't add subfolders
as additional include roots — see CMake step below), every
`#include "Foo.h"` that worked before now needs to be
`#include "<folder>/Foo.h"`:

```cpp
// Before
#include "DatabaseManager.h"
// After
#include "core/DatabaseManager.h"
```

This is more typing per line — and that's the point. The path tells you
at a glance which feature owns the dependency. If `src/analytics/AnalyticsView.cpp`
suddenly grows an `#include "pomodoro/PomodoroView.h"`, that's an
architectural smell visible right at the top of the file.

### Doing the rewrite mechanically

One `sed` invocation per mapping, applied across every `.h` and `.cpp`
in `include/`, `src/`, and `tests/`:

```bash
find include src tests -type f \( -name '*.h' -o -name '*.cpp' \) | xargs sed -i \
  -e 's|#include "DataStructures.h"|#include "core/DataStructures.h"|g' \
  -e 's|#include "DatabaseManager.h"|#include "core/DatabaseManager.h"|g' \
  -e 's|#include "DataImporter.h"|#include "core/DataImporter.h"|g' \
  # …one -e per header
```

### Verifying the rewrite

```bash
grep -rn '^#include "' include/ src/ tests/ | sort -u
```

Every line on stdout should be a folder-qualified path (`feature/Foo.h`).
If any unqualified `"Foo.h"` remains, you missed a `sed` rule.

A second sanity check: the project's own headers should never appear as
`<angle-bracket>` includes. Reserve `<…>` for system and Qt headers
(`<QWidget>`, `<vector>`); use `"…"` for our own code.

---

## 4. Sub-task 0.3 — Update CMakeLists.txt

### The single-include-root rule

After the moves, every source file uses `feature/Foo.h` style includes.
For the compiler to resolve those paths, CMake needs to know about
**one** include directory: the top-level `include/`. **Do not** add nine
`target_include_directories` lines, one per feature folder.

```cmake
target_include_directories(CTracker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

If you added `include/courses/` as a separate include root, then someone
writing `#include "EntityCard.h"` (unqualified!) would suddenly succeed
— and the feature-grouped structure would silently leak back into a
flat style. One root keeps the discipline enforced by the compiler.

### Rewriting `SOURCES` and `HEADERS`

The lists in `CMakeLists.txt` were flat. Group them by feature so the
file mirrors the disk layout:

```cmake
set(SOURCES
    src/main.cpp

    # core/ — data layer
    src/core/DatabaseManager.cpp
    src/core/DataImporter.cpp
    src/core/DataExporter.cpp

    # shared/
    src/shared/CircularProgressBar.cpp
    src/shared/SideNavigationBar.cpp
    src/shared/HomeDashboard.cpp
    src/shared/MainWindow.cpp

    # courses/
    src/courses/SessionTaskRow.cpp
    src/courses/UnitExpandableWidget.cpp
    src/courses/EntityCard.cpp
    src/courses/EntityDetailView.cpp

    # analytics/
    src/analytics/ActivityLogModel.cpp
    src/analytics/ContributionHeatmap.cpp
    src/analytics/AnalyticsView.cpp

    # settings/
    src/settings/SettingsView.cpp
)
```

### Tests target

`tests/CMakeLists.txt` references the same sources from the
main project (it builds them straight into each test binary). Update the
paths there too:

```cmake
set(CTRACKER_LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/analytics/ActivityLogModel.cpp
    …
)
```

### The silent fix we surfaced during this step

Before Phase 0, `SessionTaskRow.cpp` and `SessionTaskRow.h` existed on
disk but were **not listed** in the `SOURCES` / `HEADERS` blocks. CMake
silently never compiled them. The reason the app appeared to work
anyway is that `SessionTaskRow` was included via the moc-generated stub
of `UnitExpandableWidget`, which papered over the gap.

Why this is a real bug, not a curiosity:

1. Anything that uses `Q_OBJECT` in `SessionTaskRow` would have an
   undefined vtable at link time — the only reason linking succeeded is
   that nothing else *outside* `UnitExpandableWidget` referenced the
   class's signals/slots.
2. The header was not processed by AUTOMOC, so any `Q_PROPERTY` or
   `Q_INVOKABLE` declared in it wouldn't be registered with Qt's meta
   system.

The fix is one line in `SOURCES` and one in `HEADERS`. Phase 0 is a
good opportunity to surface this: when you rewrite the lists from
scratch by walking the disk, you naturally notice files that the old
list omitted.

### Verifying the build

```bash
cd CTracker
rm -rf build
cmake -S . -B build -G "Ninja" \
  -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/mingw_64" \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

A clean rebuild from an empty `build/` exercises the configuration end
to end — no stale CMake cache hides a broken include path. You should
see N/N targets compile and `CTracker.exe` link successfully (we saw
21/21 after Phase 0). Any earlier number means a `.cpp` is missing from
`SOURCES`.

The `-DCMAKE_PREFIX_PATH` tells CMake where to find Qt6Config.cmake. On
this machine Qt lives at `C:/Qt/6.7.2/mingw_64`. Adjust to your install.

---

## 5. Sub-task 0.4 — Spec write-back

### The spec rule (CLAUDE.md §4)

> *Every change or update made to the codebase MUST be mirrored back
> into `.ai/specs/` in the same turn — code and specs ship together,
> never separately.*

For Phase 0, "the change" is structural: where each file lives. So:

1. In `design.md`, annotate every `### Component N: <Name>` heading
   with its owning folder, e.g.
   `### Component 1: MainWindow  *(owning folder: shared/)*`.
   That single change makes the doc usable as a reverse lookup — given
   a class name, you instantly know which feature folder to open.
2. In `design.md` (top), record the convention rule:
   *one folder per top-level feature; `core/` for data layer; `shared/`
   for cross-feature widgets and chrome*. This rule pre-existed for us;
   confirm it's still accurate after the moves.
3. In `tasks.md`, tick the Phase 0 checkboxes.

The intent: a new contributor opening `.ai/specs/` alone should be able
to predict the folder layout without grepping the source tree.

---

## 6. Acceptance checklist

You've finished Phase 0 correctly if all of the following are true:

- [ ] Nine folders exist under both `include/` and `src/`.
- [ ] `.gitkeep` exists in (and **only** in) the truly empty folders
      (`todos`, `pomodoro`, `calendar`).
- [ ] `git status --short` shows every move as `R` (rename), not `D`+`??`.
- [ ] Running `grep -rn '^#include "' include/ src/ tests/` shows
      **only** folder-qualified paths for our own headers.
- [ ] `rm -rf build && cmake -S . -B build && cmake --build build`
      succeeds end to end.
- [ ] `CMakeLists.txt` lists every `.cpp` on disk under `SOURCES` and
      every `.h` under `HEADERS` — no orphans.
- [ ] `tests/CMakeLists.txt` references the new paths.
- [ ] `design.md` shows owning-folder annotations on every component
      heading, and the convention rule is recorded.
- [ ] `tasks.md` Phase 0 boxes are all ticked.

---

## 7. Common mistakes (and how they surface)

- **Forgot to update `tests/CMakeLists.txt`.** Symptom: main target
  builds, test target fails with "no such file `src/DatabaseManager.cpp`".
- **Added per-folder include roots.** Symptom: unqualified
  `#include "Foo.h"` lines compile fine even though they shouldn't.
  Fix: keep `include/` as the sole include root.
- **Used `mv` instead of `git mv`.** Symptom: `git status` shows a wall
  of deletions and additions instead of renames. Fix: revert and use
  `git mv` — git's rename detector recovers most cases if changes are
  pure moves, but be explicit.
- **`sed` rewrote includes inside comments or strings.** Symptom: a
  comment that says `// see DatabaseManager.h for details` is now
  `// see core/DatabaseManager.h for details`. Harmless, but watch for
  it in user-facing strings.
- **One folder ends up empty without `.gitkeep` and the directory
  silently disappears from git.** Symptom: cloning the repo on a fresh
  machine leaves you with eight feature folders instead of nine. Fix:
  ensure intentional placeholders have `.gitkeep`.

---

## 8. What this unlocks

With Phase 0 in place, every later task lands cleanly:

- Phase 2.8–2.11 (schema v2) — touches `core/DatabaseManager.{h,cpp}`
  only. No "where should this go?" question.
- Phase 6 (custom widgets) — each new widget header/source pair drops
  into its owning feature folder. The diff for adding `StatsCard.h/.cpp`
  is *literally* the two new files plus two lines in `CMakeLists.txt`.
- Phase 7 (views) — the view file lives next to the widgets it
  composes.

That clarity is what Phase 0 was for.
