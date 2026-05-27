# CTracker — Cloud, Accounts, Achievements & Analytics Roadmap

> Scope: planning document only. No code yet. Outlines how to add user accounts, cloud sync (offline-first), achievements, and developer-side analytics on top of the current Qt 6 + C++17 + SQLite app.

---

## 1. Goals

1. Each user has an **account** (email + password, or OAuth) — data follows them across devices.
2. Data is **saved to the cloud** so it is not lost if the local machine dies.
3. App must **work fully offline**; when the network returns, it **syncs** transparently.
4. Each user earns **achievements** (badges) for milestones.
5. As the developer, I want **aggregate analytics** across all users (total hours, courses finished, popular course names, retention, etc.).

---

## 2. Recommended Backend: Supabase

Supabase is the simplest fit for this app because:

- **Postgres** under the hood — relational, mirrors the existing SQLite schema closely.
- Built-in **Auth** (email/password, Google, GitHub, magic link) — no need to roll your own.
- **Row-Level Security (RLS)** — each user can only read/write their own rows by policy, not by app trust.
- **REST + Realtime + Storage** in one service.
- Generous free tier; self-hostable later if needed.

Alternative comparisons (in case of vendor lock-in concerns):

| Option | Pros | Cons |
|--------|------|------|
| **Supabase** (recommended) | Auth + DB + RLS bundled, Postgres, easy REST | C++ SDK is community-only — use REST/HTTPS |
| Firebase | Mature, great offline cache for mobile | NoSQL, weaker fit for relational hierarchy; no official C++ desktop SDK |
| Custom backend (Node/Go + Postgres) | Full control | You build & host everything: auth, sync, scaling |
| PocketBase | Single binary, embedded auth+DB | Smaller ecosystem, self-host required |

**Decision: Supabase**, accessed via plain HTTPS using `QNetworkAccessManager` (Qt already pulls in `Qt6::Network`). No third-party C++ SDK needed.

---

## 3. High-Level Architecture (Offline-First)

```
+-------------------+        +-----------------+        +------------------+
|  Qt UI (existing) |  <-->  |  Local SQLite   |  <-->  |  Sync Engine     |
+-------------------+        |  (source of     |        |  (background)    |
                             |   truth for UI) |        +--------+---------+
                             +-----------------+                 |
                                                                 |  HTTPS / REST
                                                                 v
                                                       +-------------------+
                                                       | Supabase (Postgres|
                                                       |  + Auth + RLS)    |
                                                       +-------------------+
```

Key principle: **the UI never talks to the cloud directly.** It only reads/writes local SQLite. A separate Sync Engine reconciles local ↔ cloud on a timer / on connectivity change. This keeps the app fully usable offline and avoids UI freezes.

---

## 4. Authentication Flow

1. **Login / Signup screen** added before `MainWindow` is shown.
2. On signup → POST to Supabase Auth `/auth/v1/signup` → receive `access_token` + `refresh_token` + `user.id` (UUID).
3. Tokens stored locally using the OS secure store:
   - Windows: **Credential Manager** via `wincred` (or `QKeychain` library).
   - Never plain-text in QSettings.
4. On app launch:
   - If a refresh token exists → silently refresh → enter MainWindow.
   - If refresh fails (offline) → enter MainWindow in **offline mode** using cached `user.id`.
   - If no token → show login screen.
5. **Guest mode** (optional): allow the app to run without an account, with a "Sign up to back up your data" banner; on signup, migrate the local DB to that user.

---

## 5. Database Schema Changes (Local + Cloud)

### Local SQLite changes (additive, non-breaking)

Add to **every user-owned table** (`CoursesProjects`, `Units`, `SessionsTasks`, `ActivityLog`, `Categories`, `ProjectMeta`, `Todos`, `PomodoroSessions`, `CalendarDayDetails`, `Settings`):

| Column | Type | Purpose |
|--------|------|---------|
| `uuid` | TEXT | Globally unique ID (generated on insert). Used as the primary key when talking to the cloud, so local integer `ID`s do not collide across devices. |
| `user_id` | TEXT | Owner UUID from Supabase Auth. |
| `updated_at` | INTEGER | Unix ms timestamp of last local change. Used for last-write-wins conflict resolution. |
| `deleted_at` | INTEGER NULL | Soft-delete marker. Hard delete only happens after the row is confirmed deleted on the server. |
| `sync_state` | TEXT | `'clean' | 'dirty' | 'conflicted'`. Drives the outbound sync queue. |

Add a new table:

```
SyncQueue (
  id INTEGER PRIMARY KEY,
  table_name TEXT,
  row_uuid TEXT,
  op TEXT,            -- 'upsert' | 'delete'
  payload TEXT,       -- JSON snapshot
  enqueued_at INTEGER
)
```

### Cloud Postgres (Supabase) mirror

- One table per local table, same column names, but `uuid` is the **PRIMARY KEY**.
- Every table has `user_id UUID REFERENCES auth.users(id)`.
- **RLS policy** on every table:
  `USING (auth.uid() = user_id)` — guarantees users only see/edit their own rows even if the client is malicious.

---

## 6. Sync Engine Design

A `CloudSyncManager` singleton (sibling to `DatabaseManager`) runs in a background thread:

1. **Triggers**:
   - App start (after login).
   - Every N minutes (e.g., 5 min) while online.
   - On `QNetworkInformation::reachabilityChanged()` → online.
   - On user action "Sync now" button in settings.

2. **Outbound (push)**:
   - Read up to K rows from `SyncQueue`.
   - Batch into a single `POST /rest/v1/{table}?on_conflict=uuid` with `Prefer: resolution=merge-duplicates`.
   - On 2xx → mark rows `sync_state='clean'`, remove from queue.
   - On 4xx/5xx → exponential backoff; keep in queue.

3. **Inbound (pull)**:
   - Track a `last_pulled_at` per table in Settings.
   - `GET /rest/v1/{table}?updated_at=gt.{last_pulled_at}&order=updated_at.asc`.
   - For each row: if local row missing or local `updated_at` < remote `updated_at` → upsert locally. If equal → skip. If local newer → push instead.

4. **Conflict resolution**: **Last-Write-Wins** by `updated_at`. Simple, predictable, and good enough for a single-user-multi-device app. (True merge only matters if two devices edit the same row offline simultaneously — rare here.)

5. **Deletes**: soft delete (`deleted_at` set) → propagates → both ends hard-delete only after the sync round confirms.

6. **Connectivity detection**: Qt 6 ships `QNetworkInformation` — listen to its signal, do not poll.

---

## 7. Achievements System

### Two layers

**A. Definition (static, ships with the app):**

`achievements.json` (or a `Achievements` table seeded at startup) describing each badge:

```
{
  "id": "first_hour",
  "title": "First Hour",
  "description": "Log your first hour of focused work",
  "icon": "trophy.svg",
  "criteria": { "type": "total_minutes", "threshold": 60 }
}
```

Criteria types to start with:
- `total_minutes` ≥ N
- `streak_days` ≥ N (consecutive days with ≥1 activity)
- `courses_completed` ≥ N (entity status = `'completed'`)
- `units_completed` ≥ N
- `pomodoros_completed` ≥ N
- `todos_completed` ≥ N
- `category_hours` ≥ N for a specific category

**B. User unlocks (per user, synced):**

```
UserAchievements (
  uuid TEXT PK,
  user_id TEXT,
  achievement_id TEXT,
  unlocked_at INTEGER,
  updated_at INTEGER,
  sync_state TEXT
)
```

### Evaluation

An `AchievementEvaluator` listens to relevant signals:
- `DatabaseManager::activityLogged()`
- `DatabaseManager::entityCompleted()`
- `PomodoroManager::sessionFinished()`

After each event it re-evaluates only the criteria types touched by that event, inserts a row in `UserAchievements` if newly unlocked, and shows a Qt `QSystemTrayIcon` / toast: *"🏆 First Hour unlocked!"*.

Cheap because most criteria are simple `COUNT()` / `SUM()` queries.

---

## 8. Developer-Side Analytics (Aggregate)

You want to see, across **all users**: total hours, courses finished, popular course names, etc. — without violating any single user's privacy.

### Approach: views + a developer dashboard

In Supabase, create **SQL views** that aggregate but never expose row-level data:

```
CREATE VIEW dev_global_stats AS
SELECT
  COUNT(DISTINCT user_id)                AS total_users,
  SUM(progress_delta_minutes) / 60.0     AS total_hours_logged,
  COUNT(*) FILTER (WHERE status='completed') AS total_courses_completed
FROM ActivityLog JOIN CoursesProjects USING (uuid);

CREATE VIEW dev_popular_course_names AS
SELECT lower(name) AS normalized_name, COUNT(*) AS users_with_this_course
FROM CoursesProjects
WHERE type='Course'
GROUP BY 1
ORDER BY 2 DESC;

CREATE VIEW dev_retention_dau AS ...
```

### Access control

- Add a `role` column to `auth.users` metadata. Mark your own account `role='admin'`.
- RLS on the views: `USING (auth.jwt() ->> 'role' = 'admin')`.
- Build a tiny separate **developer dashboard** — either:
  - A second Qt screen behind an "I am the developer" gate, **or**
  - A small standalone web page (Next.js / plain HTML + Supabase JS) — easier to iterate on and doesn't bloat the desktop binary.

### Anonymization & ethics

- Never display individual user emails or names in aggregate dashboards.
- Treat course **names** as potentially personal (e.g., "Therapy session prep"). Two options:
  - Show only counts per normalized name and **only when ≥ 5 users share the name** (k-anonymity).
  - Or hash names client-side before upload and show only hashes (loses readability — usually worse).
- Add a clear **Privacy Notice** in the signup screen and a **"Help improve CTracker by sharing anonymous usage stats"** opt-in toggle. Respect it: gate the dev-side analytics views on `Settings.share_anonymous_stats = true`.

---

## 9. Implementation Phases (suggested order)

| Phase | Deliverable | Why this order |
|-------|-------------|----------------|
| 1 | Add `uuid`, `user_id`, `updated_at`, `deleted_at`, `sync_state` columns to local schema; backfill existing rows with a single local user_id ("local-only") | Pure local migration, zero risk, unlocks everything else |
| 2 | Login / signup screen + Supabase Auth via HTTPS + secure token storage (`QKeychain`) | Identity must exist before sync |
| 3 | Read-only cloud pull for one table (e.g., `CoursesProjects`) — validate plumbing | Smallest end-to-end slice |
| 4 | Outbound `SyncQueue` + push for the same table | Completes the loop |
| 5 | Roll sync out to all tables | Mechanical once 3+4 work |
| 6 | Connectivity-aware retry, conflict policy, "Sync now" button, sync status indicator in side nav footer | UX polish |
| 7 | Achievements engine + 10 starter badges | Built on top of existing signals — no schema changes |
| 8 | Developer dashboard (separate web page) reading aggregate views | Doesn't touch the desktop app |
| 9 | Privacy toggle + opt-in flow | Required before shipping aggregate analytics |

Phases 1–2 are the only ones that touch the entire codebase. Phases 3 onward are isolated to the new `CloudSyncManager` / `AchievementEvaluator` modules.

---

## 10. Other Features Worth Considering (Same Document, No Code)

Loosely ordered by user value vs. effort:

1. **Multi-device sync indicator** — "Last synced 2 min ago" in the footer; spinning icon while syncing.
2. **Data export / import** — JSON or CSV dump of the user's whole DB ("My Data"), useful for both backup and GDPR-style data portability.
3. **Account deletion** — one-click "Delete my account and all my data" → calls a Supabase RPC that cascades.
4. **Shared courses / collaboration** — let two users see the same course (e.g., study buddy). Requires a `CourseMembers` join table and richer RLS policies. Big feature; defer.
5. **Push reminders** — daily "you haven't logged today" notification via Qt's `QSystemTrayIcon`; eventually mobile push if you build a companion app.
6. **Streak protection / "freeze" tokens** — earn a token per week, spend one to keep a streak alive after a missed day. Gamification booster.
7. **Leaderboards (opt-in)** — global "top 100 by hours this week" — purely opt-in, anonymous handles only.
8. **Goals & forecasting** — "Finish this course by July 1" → app projects required minutes/day from current pace.
9. **Focus mode integration** — block distracting apps during Pomodoro (Windows: WinAPI; needs care).
10. **Mobile companion (read-only first)** — a small Flutter/React Native app that hits the same Supabase backend, view-only at first. The desktop app remains the editor.
11. **Public profile pages** — opt-in, share a read-only summary URL ("look at my last 90 days"). Marketing benefit.
12. **AI summarizer** — weekly digest generated from `ActivityLog` ("This week you spent 12h on Course X, your longest session was Tuesday"). Cheap to add via the Claude API once data is in the cloud.
13. **Tags / search across everything** — full-text search over course/unit/session/todo names, ranked by recency.
14. **Calendar 2-way sync** — export Pomodoro sessions to Google Calendar / .ics.
15. **Theme / accent customization** — let users pick the accent color (currently hardcoded `#10b981`).
16. **Per-category goals & budgets** — "spend ≥ 5h/week on Engineering category"; surfaces underused categories.
17. **Audit log / undo** — keep the last N destructive operations recoverable (e.g., accidentally deleted a unit).
18. **End-to-end encryption (advanced)** — encrypt sensitive fields (course names) client-side before upload, so even you cannot read them. Conflicts with aggregate analytics on those fields — pick one or the other per field.

---

## 11. Risks & Things to Watch

- **Token leakage**: never log `access_token` / `refresh_token`. Strip them from any crash reports.
- **Clock skew**: `updated_at` from the client can be wrong on a misconfigured machine. Mitigate by re-stamping on the server (Postgres `DEFAULT now()` with a trigger).
- **Schema migrations** must run on **both** SQLite and Postgres. Keep a single migrations folder with two SQL dialect files per migration, or use one dialect-neutral subset.
- **Large local DBs syncing for the first time** — gate the initial push behind a progress dialog; do it in chunks of e.g. 200 rows.
- **GDPR / privacy** — once you store user data on a server, you accept responsibility. Privacy policy + delete-my-data path are non-negotiable.
- **Cost** — Supabase free tier is generous, but `ActivityLog` grows fast (one row per progress edit). Compact or downsample server-side after 90 days if needed.

---

## 12. UI / UX — Courses Page & Projects Page

> Observations after reading `CoursesView.cpp`, `EntityCard.cpp`, `ProjectsView.cpp`, `ProjectCard.cpp`, `ProjectsFilterBar.cpp`. No code changes yet — just a critique and a redesign proposal.

### 12.1 What's wrong with the Projects page today

1. **Cards have no fixed size.** `ProjectCard` doesn't call `setFixedSize()` like `EntityCard` does, so cards stretch to fill grid cells. Across rows of different content they look uneven and the grid feels ragged.
2. **The card visual language is completely different from the Courses card.** Courses use a big **circular progress ring** (the app's signature visual); Projects use a thin **horizontal `QProgressBar`**. Same app, two visual identities — that's the main reason it feels "ugly."
3. **Fixed-size badges with truncation risk.** `m_priorityBadge` is hard-coded `60×20` and `m_deadlineBadge` `80×20`. Localized strings or "Overdue 12d" will clip. Use content-sized pills with horizontal padding instead.
4. **Task count is fake.** `card->setTaskCount(0, 0)` — every project shows `0/0 tasks`. This actively misleads the user and makes the card look broken.
5. **Description is height-clamped, not properly line-clamped.** `setMaximumHeight(40)` with `setWordWrap(true)` gives a half-line cut-off on some font sizes; the third line peeks through. Need a real elide-to-2-lines.
6. **Team-size pill uses an emoji 👥**, while elsewhere the app uses SVG icons. Inconsistent and renders differently across Windows font-rendering paths.
7. **No "last activity" / "last worked on" indicator**, even though the recent UnitCard/EntityCard work added this concept (per commit `d26905f`). Projects feel less alive than courses for no good reason.
8. **Filter bar parity issue.** Projects has a collapsible filter panel (`m_filterToggleBtn` toggles a panel with priority + status combos). Courses uses an always-visible inline filter row. Two pages, two different filter interaction patterns — pick one.
9. **No subtitle / count.** `CoursesView` shows `"X courses, Y completed"` via `m_subtitleLabel`. `ProjectsView` doesn't. Small omission but it makes the page feel emptier.
10. **No sort control.** Both pages: you can filter but not sort (by deadline, by progress, by last activity, by name). For projects especially, "sort by deadline" is the natural first move.
11. **Empty-state replaces the scroll area entirely.** When a filter yields zero results, you lose the filter bar context unless you re-trigger a scroll. Better to keep the filter bar visible and show the empty state *inside* the grid area.
12. **No bulk-action / right-click affordances.** You cannot quickly pause, complete, archive, change category/priority, or delete from the grid. Every operation requires drilling into the detail view.
13. **Hover affordance is weak.** Both cards rely on a `hover` property polish — there is no clear visible elevation, accent border, or "click me" cue. New users don't realize the card is clickable.

### 12.2 Cross-page consistency goals

Before changing visuals, agree on the rules so both pages obey them:

- **One card geometry**: same width, same height, same internal padding, same corner radius, same border treatment.
- **One progress visual**: pick the circular ring as the canonical (it's the app's signature). Use it on both Courses and Projects cards.
- **One filter bar**: same layout — search + sort + filter chips inline, no collapsible drawer. Filter state shown as removable chips below the bar.
- **One metadata strip**: bottom row of every card carries the same 3 slots — *category/type, last activity, progress count*. Project-specific extras (deadline, priority) live in a different zone (top-right corner badge), not crammed into the bottom row.
- **One hover state**: lift 2 px + 1 px accent (`#10b981`) border, same on both pages.
- **One empty-state pattern**: filter bar stays, grid area shows the empty state.

### 12.3 Redesigned Projects card — proposed layout

Target size: **240 × 200** (wider than courses' 160×180 to leave room for description + deadline; height harmonized).

```
┌──────────────────────────────────────┐
│ ◯ category-dot  Project Name      🔴 │  ← top: tiny color dot + name, status bubble (top-right)
│                                      │
│ Two-line description, elided…        │
│                                      │
│   ┌──────────┐                       │
│   │   72%    │   Priority: High      │  ← circular ring (64–72 px) on left, meta column on right
│   │  (ring)  │   Due in 5 days       │
│   └──────────┘   12 / 18 tasks       │
│                  Last worked 2h ago  │
└──────────────────────────────────────┘
```

Key decisions:

- **Same `CircularProgressBar` as Courses** (smaller — 64 px) so the visual family is unified, but moved to the left so the rich textual metadata gets room.
- **Status bubble** in the top-right (small colored dot + tooltip): green=active, amber=paused, gray=completed. Replaces the muted "Paused" pill — saves space.
- **Priority** is shown as a colored left-edge accent stripe (4 px) instead of a centered pill. Lets you see priority at a glance across the grid by color band: red / amber / gray.
- **Deadline** moves into the meta column as plain text with red/amber/gray foreground color depending on urgency.
- **"Last worked X ago"** added — uses the same `last_activity` source as UnitCard.
- **No emoji icons** — use the existing `Qt6::Svg` icon set.

### 12.4 Redesigned Courses card — small refinements

The Courses card is already the better of the two. Light edits to bring it in line:

- Add a **last-activity** line below the name (already shipped on UnitCard; just port the same look).
- Move the **"Course" / "Project"** type label out — it's redundant on a page filtered to one type. Show it only on the Home dashboard mixed grid.
- Add the same **left-edge accent stripe** but tied to **category color** (matching the CategoryPill). Removes the need for the floating pill overlay at top-left, which can clip the ring on small widths.
- Harmonize size to **240 × 200** with Projects.

### 12.5 Redesigned filter / sort bar (shared)

Single component used by both pages, parameterized by which combos to show:

```
┌────────────────────────────────────────────────────────────────────────┐
│ [🔍  Search projects…              ]   Sort: [Deadline ▾]   [+ Add New]│
│  [×] Priority: High   [×] Status: Active   [Clear all]                 │
└────────────────────────────────────────────────────────────────────────┘
```

- Search + sort + add are always visible; the **filter chips appear directly under** the bar once active. No collapsible panel.
- Clicking the chip's `×` removes only that filter.
- **Sort options** (new): Recent activity, Deadline (Projects only), Progress ↑/↓, Name A→Z, Created date.
- For multi-select filters (e.g., multiple categories), open a small popover anchored to a "+ Filter" button — not a drawer that pushes the grid down.

### 12.6 Interaction improvements (both pages)

- **Right-click context menu** on a card: *Open, Mark complete, Pause/Resume, Change category, Change priority, Archive, Delete*. This single change removes 80% of the friction for power users.
- **Keyboard nav**: arrow keys move focus across the grid, Enter opens detail, Space toggles status.
- **Drag-and-drop reorder** (later): manual ordering becomes a sort option ("Custom").
- **Inline progress edit** on hover: a tiny `+5%` / `−5%` step buttons appear on hover for fast logging without opening detail view. (Optional / power-user.)
- **Hover preview** (later): hovering a card for >500 ms shows a popover with the most recent ActivityLog entries — answers "what did I last do here?" without a click.

### 12.7 Information density & responsive behavior

Current breakpoints (`<500 / <700 / <1000 / ≥1000` → 1/2/3/4 cols) are fine, but with the **240 × 200** card the math changes:

| Viewport | Columns | Notes |
|----------|---------|-------|
| < 560 px | 1 | Mobile-style stack |
| 560–820 | 2 | |
| 820–1120 | 3 | |
| ≥ 1120 | 4 | |

Add a small **"compact mode"** toggle in the filter bar that swaps to a **list view** (rows with the same data, no ring — useful when the user has 40+ projects). One toggle, two layouts, same data.

### 12.8 Empty states

Instead of hiding the scroll area, keep the filter bar in place and put the empty-state widget inside the grid container. Two phrasings:

- *No projects yet* → primary CTA "Add Your First Project" (current behavior, just keep filter bar visible).
- *No projects match these filters* → secondary CTA "Clear filters", **not** "Add new" (current behavior shows the add CTA, which is wrong in this case).

### 12.9 Visual polish checklist

- Unify card border-radius (`12 px`), padding (`16 px`), spacing (`8 / 12 / 24`).
- One shared `card.qss` block — never per-card stylesheets — so future theme changes are one edit.
- Use the `#10b981` accent only for: primary CTAs, hover stripe, progress ring fill. Don't sprinkle it elsewhere.
- Deadline / priority colors come from a fixed palette (red `#ef4444`, amber `#f59e0b`, gray `#6b7280`) — already used in `ProjectCard`; promote them to QSS variables / a `Theme` namespace.
- Replace all emoji-as-icon usage with SVG.

### 12.10 Suggested phasing (UX)

| Phase | Change | Effort |
|-------|--------|--------|
| A | Add subtitle + sort dropdown + right-click context menu to both pages | small |
| B | Replace ProjectCard with a circular-ring layout that matches EntityCard family | medium |
| C | Unify FilterBar component (delete `ProjectsFilterBar`, parameterize `CoursesFilterBar`) | medium |
| D | Add "last activity" + accurate task counts to ProjectCard (requires DB query batching) | medium |
| E | Add list-view toggle, keyboard nav, inline progress edit | larger |

Phases A and C are pure UI work, no schema changes. Phase D requires fixing the `setTaskCount(0,0)` placeholder by computing real counts via JOIN in `fetchAllProjects()`.

---

## 13. Rethinking the Project Detail Page — Tasks, not Sessions

> The Projects detail page reuses `EntityDetailView` (built for Courses), so a "project" is just `Project → Unit → SessionTask` with a 0–100 progress slider on every leaf. That works for studying ("I'm 60% through Lecture 3"), but it is the wrong shape for project work — and it's the deeper reason the page feels awkward.

### 13.1 Why "Sessions with a slider" fails for projects

- **A task is a binary thing**, not a continuum. You don't drag "Set up CI" to 47%. It is `todo`, `in progress`, or `done` (sometimes `blocked` / `review`).
- **Real project work has fields a slider can't express**: due date, assignee, dependencies, attachments, labels, estimate vs. actual time.
- **Progress is computed**, not entered. A project is X% complete because Y/Z tasks are done — not because the user moved a slider.
- **"Units"** as the only sub-grouping is restrictive. Some users want phases (Design → Build → Ship); others want columns (Backlog → Doing → Done); others want sprints/milestones.
- **The activity log is wrong for tasks**: it records "progress slid from 30 → 45". For tasks the meaningful events are *status changed*, *assigned*, *commented*, *due-date moved*.

### 13.2 Proposed project data model

Keep the existing schema **for courses untouched**. Add a parallel set of tables that only Projects use. Both still hang off `CoursesProjects` so cross-cutting features (categories, status, heatmap aggregation) keep working.

```
CoursesProjects (existing, unchanged)
  ├── ProjectMetaData (existing — description, priority, deadline, team, links)
  │
  └── Milestones (NEW — replaces "Units" for projects)
        id, projectId (FK → CoursesProjects),
        name, description,
        dueDate, order, color, completed

        └── Tasks (NEW — replaces "SessionsTasks" for projects)
              id, projectId, milestoneId (nullable — task can live outside a milestone),
              title, description,
              status TEXT     -- 'todo' | 'in_progress' | 'blocked' | 'review' | 'done'
              priority TEXT   -- 'low' | 'medium' | 'high' | 'critical'
              assignee TEXT   -- single-user app, but keep the column for future multi-user
              dueDate DATE,
              estimateMinutes INTEGER,
              actualMinutes INTEGER,         -- accumulated from time entries
              parentTaskId INTEGER NULL,     -- enables subtasks (self-FK)
              order REAL,                    -- fractional order for cheap drag-reorder
              createdAt, updatedAt, completedAt

              ├── TaskLabels (many-to-many)
              │     taskId, labelId
              │
              ├── TaskDependencies (many-to-many, self-referencing on Tasks)
              │     taskId, dependsOnTaskId
              │
              ├── TaskComments (free-form notes / journal)
              │     id, taskId, body, createdAt
              │
              └── TaskTimeEntries (optional — tie Pomodoro to a task)
                    id, taskId, startedAt, durationMinutes, source ('manual' | 'pomodoro')

Labels (NEW, project-scoped or global)
  id, projectId NULL, name, color
```

**Progress for a project is now computed**:
`overallProgress = doneTasks / (totalTasks - cancelledTasks) * 100`
weighted optionally by estimate (a 4-hour task counts more than a 5-minute task). Whichever is chosen, the formula is **derived**, not stored; the existing `ActivityLog` no longer fires on Project rows — instead `TaskHistory` records status transitions for analytics.

> Optional but recommended: a `TaskHistory` table (taskId, fromStatus, toStatus, changedAt) so the project gets its own activity timeline and the heatmap can still aggregate "project days that mattered."

### 13.3 What changes in `DataStructures.h`

Add these structs (no logic — same POD style as the rest of the file):

- `MilestoneData { id, projectId, name, description, dueDate, order, color, completed }`
- `TaskData { id, projectId, milestoneId, title, description, status, priority, assignee, dueDate, estimateMinutes, actualMinutes, parentTaskId, order, createdAt, updatedAt, completedAt, labelIds }`
- `LabelData { id, name, color, projectId }`
- `TaskCommentData { id, taskId, body, createdAt }`
- `TaskTimeEntryData { id, taskId, startedAt, durationMinutes, source }`
- A `TaskStatus` and `TaskPriority` enum (or string constants) shared by the view layer.

Leave `UnitData` and `SessionTaskData` exactly as they are — courses still use them.

### 13.4 Project Detail page — proposed layout

Replace the current "unit list with sliders" with a **multi-view tasks board**. Same data, three ways to look at it; user picks via a small segmented control at the top of the right pane.

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ ←  ProjectName    🟢 Active   ⚑ High   ⏳ 12d left           [ ⚙ Edit ]    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                            ╭─────────────╮  │
│  ┌─ Tabs ──────────────────────────────────────────────┐   │ Project     │  │
│  │  Overview │ Board │ List │ Timeline │ Notes        │   │ Info        │  │
│  └────────────────────────────────────────────────────-┘   │             │  │
│                                                            │ Description │  │
│  ╭── Filter / quick add ──────────────────────────────╮    │ Deadline    │  │
│  │ [+ Add task]   Milestone: All ▾   Label: All ▾     │    │ Team        │  │
│  │ Status: All ▾  Sort: Due ▾   [🔍 search…]          │    │ Links       │  │
│  ╰────────────────────────────────────────────────────╯    │ Progress 64%│  │
│                                                            │             │  │
│   <Selected view body — Board / List / Timeline / etc.>    │ Stats:      │  │
│                                                            │  18 tasks   │  │
│                                                            │  12 done    │  │
│                                                            │  3 overdue  │  │
│                                                            ╰─────────────╯  │
└─────────────────────────────────────────────────────────────────────────────┘
```

The right-side **Project Info panel stays** (description, deadline, team, links) — it's the project's *what*. The left side becomes the project's *how* — actual work.

### 13.5 Five view modes — what each is for

| Tab | Best for | What you see |
|-----|----------|--------------|
| **Overview** | First-thing-on-open glance | Progress ring, "next 3 due" list, "blocked" list, recent activity, milestone progress bars |
| **Board** | "What am I working on?" — daily driver | Kanban columns by status: `Todo · In Progress · Blocked · Review · Done`. Cards drag between columns. Limit-WIP indicator. |
| **List** | Bulk edit / search / sort by anything | Dense table: checkbox, title, milestone, status, priority, due, assignee. Click column header to sort. Multi-select for bulk status/label changes. |
| **Timeline** | Deadline-driven projects | A simple Gantt-lite: rows = tasks, x-axis = days. Milestone diamonds. Dependency arrows if dependencies are set. |
| **Notes** | Brain-dump per project | Plain rich-text scratch area saved into `ProjectMetaData.description` *or* a new `ProjectNotes` table for longer notes. |

Ship **Overview + Board + List first**; Timeline and Notes are phase 2.

### 13.6 Task card / row — exact fields

A Task should be the same object visually whether shown as a kanban card, a list row, or a timeline bar — just different chrome.

Minimum visible fields on a card:

- **Title** (single-line elided)
- **Status badge** (color from a 5-color palette)
- **Priority dot or stripe** (red/amber/blue/gray)
- **Due date pill** (red/amber/gray by urgency, hidden when no date)
- **Labels** (up to 3 colored chips, "+2" overflow)
- **Subtask progress** (`3/5` if it has subtasks; hidden otherwise)
- **Comment indicator** (small bubble + count if >0)

Click → opens a **Task Detail dialog** (modal or right-side slide-over) showing description, comments, dependencies, time entries, history.

### 13.7 Quick-add and inline-edit (the friction killers)

These are where most task apps win or lose:

- **Quick add bar** at the top: typing `Deploy to staging #infra !high due:fri` parses inline tokens — labels (`#`), priority (`!`), due date (`due:`) — and creates a task in one keystroke. Stretch goal but huge UX win.
- **Inline status toggle**: clicking the status badge on any card cycles through statuses without opening detail view.
- **Drag-to-reschedule** in Timeline; drag column in Board; drag row in List for reordering.
- **Keyboard shortcuts** (project-detail-scoped): `N` new task, `B/L/T` switch to Board/List/Timeline, `/` focus search, `Enter` open task, `J/K` move selection, `1-5` set priority, `X` complete.

### 13.8 Pomodoro integration becomes meaningful

Today the Pomodoro session can be linked to a *course*. Extend the same `courseId` column (rename to `entityId`) so a Pomodoro can be tied to a **task** as well. On finishing a Pomodoro:

- A `TaskTimeEntries` row is inserted (source=`pomodoro`).
- The task's `actualMinutes` increments.
- The user is asked once: *"Mark task as in-progress?"* if it was `todo`.

This finally makes the Pomodoro feature pay off for project users — the timer is what feeds project progress.

### 13.9 Migration strategy from the current model

You have existing project data sitting in `Units` + `SessionsTasks` (with progress sliders). Don't throw it away — convert:

1. For every Project (entities with `Type='Project'`):
   - Each `Unit` becomes a `Milestone` (copy name, set `order` by current insertion order).
   - Each `SessionTask` becomes a `Task` with:
     - `status = 'done'` if `progress == 100`, `'in_progress'` if `0 < progress < 100`, `'todo'` if `progress == 0`.
     - `milestoneId` = corresponding new milestone.
     - `actualMinutes = 0` (we don't have historical time).
2. ActivityLog rows for project tasks are left as-is for history; new project edits write `TaskHistory` instead.
3. Once migrated, the `ProjectDetailView` no longer instantiates the base-class slider scaffold — it instantiates the new tasks board.

The migration is one-time, runs on first launch after the schema bump, and is reversible if we keep the old rows untouched (just stop writing to them for projects).

### 13.10 Boundaries — what NOT to add yet

To stop this becoming "rebuild Jira":

- **No real-time multi-user.** Assignee is a single string field; multi-user comes with cloud sync from §3.
- **No custom fields / custom workflows.** Fixed statuses, fixed priorities. Maybe add per-project status renaming later, nothing more.
- **No sub-projects.** A `Project` cannot contain another `Project`. Use Milestones for grouping.
- **No fancy formulas.** Progress is one of two: count-based or estimate-weighted. User picks once in project settings.
- **No notifications beyond local toasts.** Email/push live in §3 once cloud lands.

### 13.11 Suggested phasing

| Phase | Deliverable |
|-------|-------------|
| 1 | Add `Milestones` + `Tasks` tables (additive). Write a one-shot migration from existing Units/SessionsTasks. App still renders the old slider view. |
| 2 | New `TasksBoardView` widget (kanban only) shown for Projects instead of the slider view. Old `UnitExpandableWidget` stays for Courses. |
| 3 | Add `TasksListView` (table with sort + bulk select). |
| 4 | Quick-add parser + inline status cycle + keyboard shortcuts. |
| 5 | Overview tab (progress ring + "next due" + "blocked" + milestone bars). |
| 6 | Pomodoro→Task wiring (`TaskTimeEntries`). |
| 7 | Timeline / Gantt-lite. |
| 8 | Notes tab + TaskHistory-driven project heatmap. |

Phases 1–2 alone already make the Projects page feel like a real task tracker. Everything after that is iterative polish.

### 13.12 What stays the same

- The right-side **Project Info panel** (description, deadline, team, links). No reason to touch it.
- **Title-bar badges** (status, priority, deadline). Same place, same colors.
- **Categories** (color tag, filter chip on the Projects grid). Cross-cutting feature — unaffected.
- The **Projects grid card** redesign from §12 still applies; it now also shows `12/18 tasks` correctly because counts are real (fixes the `setTaskCount(0, 0)` bug).

---

## 14. TL;DR

- **Backend**: Supabase (Postgres + Auth + RLS), accessed via `QNetworkAccessManager` over HTTPS — no extra C++ SDK.
- **Offline-first**: SQLite stays the UI's source of truth. A background `CloudSyncManager` reconciles via `uuid` + `updated_at` + `SyncQueue`, with last-write-wins conflict resolution.
- **Achievements**: static catalog + per-user unlocks table; evaluated from existing DB signals; synced like any other table.
- **Developer analytics**: SQL views in Supabase, gated by admin RLS, surfaced in a separate web dashboard. Anonymize and respect an opt-in toggle.
- **Phasing**: schema migration → auth → one-table round-trip → expand to all tables → polish → achievements → dev dashboard. Risk stays low because each phase is independently shippable.
