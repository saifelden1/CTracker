# CTracker — Claude Native Configuration

> This file is auto-loaded into every Claude Code session and every spawned subagent for this workspace. It mirrors the bootstrap rules from `.ai/onboarding.md` so future chats inherit the same workflow without re-reading it.

## 1. Bootstrap on every new session

On session start, before doing any work:

1. Read `.ai/onboarding.md` (entry point).
2. Load the engineer skill from `.ai/skills/engineer/SKILL.md`.
3. Index `.ai/specs/requirements.md`, `.ai/specs/design.md`, and `.ai/specs/tasks.md`.
4. Locate the active task — the **first unchecked `- [ ]`** in `tasks.md`.
5. Produce an **Initial Sync Report** (Status / Methodology / Sub-tasks) before any implementation.

## 2. Transparent execution ("Think Out Loud")

**Before writing a single line of code:**

- Explain the **Why** and **How** of the chosen approach in chat.
- Decompose large tasks into atomic, verifiable mini-steps.
- Surface risks: hardware conflicts, race conditions, Qt/MOC pitfalls, DB schema implications.
- For each mini-step, state how success will be verified (build command, visual check, test output).

Do not hide assumptions. If the prompt is ambiguous, pause and clarify before coding.

## 3. Karpathy coding laws (project-wide)

- **Simplicity first** — minimum code necessary; no speculative abstractions.
- **Surgical changes** — only touch files the task requires; match existing style and naming exactly.
- **No black boxes** — every task is explained before it is written.

## 4. Documentation integrity (write-back rule)

The `.ai/specs/` folder is the **source of truth** for project state. After completing work:

- Tick the corresponding checkboxes in `.ai/specs/tasks.md`.
- Update `.ai/specs/design.md` if the implementation deviates from the documented design.
- **Do not** create separate "thinking", "log", "notes", or "plan" files unless explicitly asked.

## 5. Project facts (stable context)

- **Stack:** C++17, Qt 6 (Core / Widgets / Sql), SQLite, CMake ≥ 3.20.
- **Architecture:** Strict Model–View separation; Signal/Slot wiring; `DatabaseManager` singleton; dark industrial QSS theme.
- **UI strategy (hybrid):**
  - `.ui` (Qt Designer) → static layouts only: `MainWindow` frame, `SettingsView`, `AnalyticsView` skeleton.
  - Pure C++ → all custom widgets: `CircularProgressBar`, `ContributionHeatmap`, `SessionTaskRow`, `UnitExpandableWidget`, `EntityCard`.
- **Theme palette:** background `#0d1117`, panel `#161b22`, border `#30363d`, text `#c9d1d9`, accent `#39d353` (GitHub green).
- **Hierarchy:** Entity (Course/Project) → Units → Sessions/Tasks → ActivityLog (cascade-delete on every FK).

## 6. Subagents inherit this file

Any subagent spawned via the Agent tool in this workspace must follow Sections 1–4. When delegating, the parent should remind the agent to read `.ai/onboarding.md` and the relevant `.ai/specs/*` files before acting — never rely on the agent guessing the active task.
