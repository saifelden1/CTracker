# CTracker Documentation

> One-page index for everything in `docs/`. Start here, then jump to whichever
> file matches the question you're actually trying to answer.

CTracker is a desktop app for tracking your courses, projects, study
sessions, todos, pomodoro timer, calendar, and analytics. It's written
in C++ using Qt 6, and stores everything in one small SQLite file on
your disk. The canonical project state lives in `.ai/specs/`
(requirements, design, tasks); the files in this folder are *teaching*
layers on top of that — they explain **why** each phase was built the
way it was, and **how** the pieces fit together. **Written in plain
English for beginners** — every doc opens with a short "before you
start" note in plain language.

> **Quick glossary.**
> - **v1 / v2** = two versions of the database layout. v1 was the
>   original 4 tables; v2 added 6 more later. Both labels coexist in
>   the code because the app upgrades old v1 databases to v2.
> - **Struct** = a custom data shape in C++. Like a spreadsheet row
>   with named columns.
> - **Singleton** = a class with exactly one instance for the whole
>   app. `DatabaseManager` is the only one in this project.
> - **Signal** = Qt's "shout into the air" mechanism. Anything that
>   cares can listen and react. Most-used signal here is
>   `dataChanged`.
> - **CRUD** = Create, Read, Update, Delete. The four basic operations
>   on any table.
> - **Migration** = code that upgrades an older database to a newer
>   layout without losing the user's data.
> - **Idempotent** = safe to run twice. Important for setup and
>   migration code.

---

## Map of the documentation

| File | Format | What it answers | When to read it |
|------|--------|----------------|-----------------|
| [`system-overview.html`](system-overview.html) | HTML + Mermaid | *"Show me the whole system on one page."* Layer diagram, ER schema, startup sequence, slider flow, page topology, class map, status matrix. | First contact, or when re-orienting after time away. |
| [`code-reference.md`](code-reference.md) | Markdown | *"What does class `X` expose? Where is method `Y` defined?"* Per-file class catalogue, method tables annotated `[v1] / [v2] / [Phase 3]`, full signal/slot connection map. | While coding — keep open in a side tab. |
| [`phase-0-explained.md`](phase-0-explained.md) | Markdown | *"Why is the source tree grouped by feature, and how was it restructured without breaking the build?"* Feature folders, `git mv` mechanics, include rewriting, CMake single-include-root rule, the silent `SessionTaskRow` fix. | When touching folder layout or CMake. |
| [`phase-1-and-2-explained.md`](phase-1-and-2-explained.md) | Markdown | *"How does the schema work and how do migrations stay idempotent?"* CMake/Qt fundamentals, singleton rationale, v1 schema decisions, v2 tables + indexes, `SchemaInfo` / `migrate()`, `INSERT OR IGNORE` seeding. | When touching `DatabaseManager` or schema. |
| [`phase-3-explained.md`](phase-3-explained.md) | Markdown | *"What changed in the data vocabulary, and why is there a `LEFT JOIN Categories` everywhere now?"* `EntityData` extension, the 7 v2 structs, 5 filter/state structs, the shared `kEntitySelectSql` constant, clangd noise note. | When touching `DataStructures.h` or entity reads. |

> Open the HTML file in any browser — it bundles its own Mermaid render via
> CDN; no build step. Markdown files render natively on GitHub and in any
> Markdown previewer.

---

## Reading order for a newcomer

1. **`system-overview.html`** — get the mental model. Skim the diagrams; don't
   read every word.
2. **`phase-0-explained.md`** — understand the source layout so the rest of
   the codebase doesn't feel arbitrary.
3. **`phase-1-and-2-explained.md`** — internalise the database story.
4. **`phase-3-explained.md`** — see how the data vocabulary grew.
5. **`code-reference.md`** — switch to lookup mode; stop reading
   front-to-back.

A reader who finishes (1)–(4) can replicate phases 0–3 of CTracker on their
own. (5) is the working manual after that.

---

## Where the code actually lives

| Artifact | Path |
|----------|------|
| Qt project root | `CTracker/` |
| Source tree (feature-grouped) | `CTracker/src/{core,shared,courses,projects,todos,pomodoro,analytics,calendar,settings}/` |
| Headers (mirrors `src/`) | `CTracker/include/...` |
| Top-level build | `CTracker/CMakeLists.txt` |
| Tests (placeholder) | `CTracker/tests/CMakeLists.txt` |
| Visual reference (React, **not built**) | `design/` |
| Project state of record | `.ai/specs/{requirements,design,tasks}.md` |
| Workflow rules | `CLAUDE.md`, `.ai/onboarding.md`, `.ai/skills/engineer/SKILL.md` |

The `design/` React prototype is **reference-only** — its `.tsx`/Tailwind
files exist to specify spacing, palette, and interaction. No JSX, no Radix,
no Recharts ever ship into the Qt binary. (See CLAUDE.md §5a.)

---

## Phase status at a glance

| Phase | Status | Notes |
|-------|--------|-------|
| 0 — Source reorg | ✅ Built | Feature folders + CMake single include root. |
| 1 — CMake / Qt skeleton | ✅ Built | `MainWindow`, dark QSS, sidebar, dashboard. |
| 2 — Database | ✅ Built | 10 core tables defined and seeded directly. |
| 3 — Data vocabulary | ✅ Built | `EntityData` extended; structural definitions available. |
| 4 — Courses module | 🟡 Partial | `EntityCard`, `UnitExpandableWidget`, detail view scaffolded; v2 fields not yet surfaced. |
| 5 — Projects module | 🟡 Partial | Alias header only; project meta UI pending. |
| 6 — Todos / Pomodoro | ⬜ Scheduled | Folders exist with `.gitkeep`; widgets not started. |
| 7 — Calendar | ⬜ Scheduled | Same. |
| 8 — Analytics polish | 🟡 Partial | `ContributionHeatmap` + `ActivityLogModel` exist; charts pending Qt6::Charts. |
| 9 — Tests + packaging | ⬜ Scheduled | `tests/CMakeLists.txt` skeleton present; no `test_*.cpp` yet. |

Authoritative tick-list: [`.ai/specs/tasks.md`](../.ai/specs/tasks.md).

---

## Conventions you'll see across these docs

- **Path style** — Folder-qualified includes (`#include "core/DatabaseManager.h"`)
  resolve against the single include root `CTracker/include/`. The matching
  `.cpp` lives under `CTracker/src/<same-folder>/`.
- **Phase tags** — `[v1]` = original schema/widget; `[v2]` = added in Phase 2.8–2.11;
  `[Phase 3]` = added in the data-vocabulary expansion.
- **Status tags** — ✅ Built · 🟡 Partial · ⬜ Scheduled.
- **Code blocks** — Always copy/paste-ready. Pseudocode is explicitly labelled.
- **Karpathy laws apply to docs too** — Minimum prose. No speculative
  abstractions. Every paragraph should change what the reader knows or does
  next.

---

## Updating these docs

Per CLAUDE.md §4 (Documentation integrity), every code change that affects
behaviour, schema, structure, or naming **must** be mirrored into
`.ai/specs/*.md` in the same turn. The files in `docs/` are explanatory and
update on a slower cadence — but when a phase finishes, add or extend the
corresponding `phase-N-explained.md` and refresh the status matrix above and
in `system-overview.html`.

If you add a new doc file, give it a row in the *Map of the documentation*
table at the top of this README — that's the only index the reader sees.
