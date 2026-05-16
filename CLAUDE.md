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

The `.ai/specs/` folder is the **source of truth** for project state. **Every change or update made to the codebase MUST be mirrored back into `.ai/specs/` in the same turn** — code and specs ship together, never separately.

- Any new behavior, requirement, or scope change → update `.ai/specs/requirements.md`.
- Any deviation from, addition to, or refinement of the documented architecture, schema, widget structure, or styling → update `.ai/specs/design.md`.
- Any task started, advanced, or finished → tick / add the corresponding checkbox in `.ai/specs/tasks.md`.
- If a change touches multiple specs, update all of them — partial sync is not acceptable.
- Treat a code edit without the matching spec edit as an incomplete change; do not report the work as done until specs reflect reality.
- **Do not** create separate "thinking", "log", "notes", or "plan" files unless explicitly asked.

## 5. Project facts (stable context)

- **Stack:** C++17, Qt 6 (Core / Widgets / Sql / Charts / Svg), SQLite, CMake ≥ 3.20.
- **Architecture:** Strict Model–View separation; Signal/Slot wiring; `DatabaseManager` singleton; dark industrial QSS theme.
- **UI strategy (hybrid):**
  - `.ui` (Qt Designer) → static layouts only: `MainWindow` frame, `SettingsView`, `AnalyticsView` skeleton.
  - Pure C++ → all custom widgets.
- **Theme palette (canonical, matches `design/`):** background `#1a1d24` / elevated `#1f2229` / surface `#252932`; sidebar `#16181d`; primary accent `#10b981`; text `#e4e6eb`; borders `#2d323d`.
- **Hierarchy:** Entity (Course/Project) → Units → Sessions/Tasks → ActivityLog (cascade-delete on every FK). Schema includes core tables alongside Categories, ProjectMeta, Todos, PomodoroSessions, CalendarDayDetails, Settings.

## 5a. Visual reference: `design/` is reference-only

The repo contains a React + TypeScript + Tailwind prototype under `design/`. **It is a visual and interaction spec, never a build input.** Hard rules for every session and subagent:

- **Implementation is Qt 6 + C++17 + QSS.** No React, TSX, Tailwind, shadcn/ui, Radix, or Recharts code is ported, embedded, executed, or shipped.
- Translation map: JSX → `QWidget`/`QFrame` subclasses; Tailwind classes → handwritten QSS rules; Radix primitives → native Qt equivalents (`QMenu`, `QToolTip`, `QDialog`, `QComboBox`); Recharts → `Qt6::Charts`; Lucide icons → SVGs rendered via `Qt6::Svg`; React state hooks → member variables + `connect()` signal/slot wiring.
- When asked to "match the design," read the relevant `.tsx` file for layout/spacing/color *values* only; write the actual widget in C++.
- The React project is never built into the Qt binary. Plugin-generated `.cpp`/`.h` output (if any) is treated as reference material — never pasted into `src/`/`include/`.

## 6. Subagents inherit this file

Any subagent spawned via the Agent tool in this workspace must follow Sections 1–4. When delegating, the parent should remind the agent to read `.ai/onboarding.md` and the relevant `.ai/specs/*` files before acting — never rely on the agent guessing the active task.
