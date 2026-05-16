# Requirements Document: CTracker

## Introduction

CTracker is an offline Engineering Course & Project Management Suite built with C++17 and Qt 6. It provides hierarchical tracking for courses and projects with custom circular progress indicators, GitHub-style contribution heatmap analytics, and automatic time-series activity logging. The application includes integrated to-do management, a Pomodoro focus timer, an interactive calendar with per-day notes, course categorization with color tagging, and analytics charts (line/bar/pie/streaks). It follows strict Model-View separation, uses SQLite for persistence, and implements a dark industrial theme.

> **Visual Spec Source of Truth**: `design/` (React prototype). Qt widgets reproduce its layouts, palette, spacing, and interaction patterns.
>
> **Implementation Constraint (non-negotiable)**: The `design/` folder is a **visual and interaction reference only**. The shipped application is **Qt 6 + C++17 + QSS**. No React, TypeScript, Tailwind, shadcn/ui, Radix, or Recharts code is to be ported, embedded, executed, or shipped. Every component described below is implemented as a native Qt widget (or `QChartView` for analytics charts). The React project is never built or bundled into the Qt application; it only informs design decisions.

## Glossary

- **Application**: The CTracker desktop application
- **Entity**: A course or project being tracked
- **Course**: An academic or learning-related entity with hierarchical structure
- **Project**: An engineering or development-related entity with hierarchical structure
- **Unit**: A subdivision of a Course or Project containing Sessions/Tasks
- **Session**: An individual learning or work session within a Unit (for Courses)
- **Task**: An individual work item within a Unit (for Projects)
- **SessionTask**: A Session or Task item with associated progress
- **Progress**: A percentage value from 0 to 100 indicating completion
- **ActivityLog**: A time-series record of progress changes
- **Heatmap**: A GitHub-style 12-week grid visualization of recent activity intensity (84 days)
- **CircularProgressBar**: A custom widget displaying progress as a circular arc
- **SideNavigationBar**: A vertical 256px navigation bar with icon + label buttons, app header, and user footer
- **HomeDashboard**: The main view displaying stats cards, calendar, day-detail panel, and recent-activity heatmap
- **EntityCard**: A card widget displaying a course or project with progress indicator and category tag
- **UnitExpandableWidget**: An expandable/collapsible widget containing Sessions/Tasks
- **ProgressSlider**: A slider control for adjusting Session/Task progress
- **Category**: A user-defined tag with a name and color, attachable to courses/projects
- **Todo**: A standalone task with a title, completion state, and priority (high/medium/low)
- **PomodoroSession**: A completed focus session recording course, duration, and completion timestamp
- **CalendarDayDetail**: Per-day notes and todo/completed task lists attached to a calendar date
- **ProjectMeta**: Project-specific metadata — description, status, priority, deadline, team, links
- **DatabaseManager**: The centralized database access layer

## Requirements

### Requirement 1: Application Navigation Structure

**User Story:** As a user, I want a consistent navigation interface, so that I can easily switch between different views of the application.

#### Acceptance Criteria

1. THE Application SHALL provide a fixed 256px-wide side navigation bar with icon + label buttons for Home, Courses, Projects, To-Do, Pomodoro, Analytics, and Settings views
2. WHEN a user clicks a navigation button, THE Application SHALL switch the main content area to display the corresponding view
3. THE Application SHALL highlight the currently active navigation button in the side navigation bar using the primary accent color
4. THE Application SHALL use a QStackedWidget to manage the main content area for view switching
5. THE SideNavigationBar SHALL include a header section with the app name "CTracker" and logo, and a footer section with a user profile (avatar, name, email)

### Requirement 2: Home Dashboard Display

**User Story:** As a user, I want a dashboard that summarizes my activity, surfaces today's tasks via a calendar, and shows recent contribution intensity at a glance.

#### Acceptance Criteria

1. THE HomeDashboard SHALL display a top row of three stats cards: Active Courses (count of `status = 'active'`, with paused count badge), Projects (in-progress count with "due soon" sub-count), and Completion Rate (average progress of active courses only, expressed as percentage)
2. THE HomeDashboard SHALL display a two-column lower section: an interactive monthly calendar (left) and a stacked panel containing Day Details (top) + Activity Heatmap (bottom) on the right
3. THE Calendar SHALL provide month/year navigation arrows, a 7-column day grid with single-letter day names, indicator dots on days that contain todos or completed items, and the current day highlighted in the primary accent color
4. WHEN a user clicks a day in the Calendar, THE DayDetails panel SHALL display the selected date along with three sections: To Do list, Completed list (rendered with strikethrough), and Notes textarea
5. THE Activity Heatmap on the Home page SHALL display the last 12 weeks (84 days) of activity, with month labels across the top and abbreviated day-of-week labels (Mon/Wed/Fri) on the left, plus a summary line showing total tracked days and total tasks completed

### Requirement 3: Circular Progress Visualization

**User Story:** As a user, I want to see my progress visualized as a circular progress bar, so that I can quickly understand my completion status at a glance.

#### Acceptance Criteria

1. THE CircularProgressBar SHALL render a circular arc using QPainter with antialiasing
2. WHEN the progress value is set, THE CircularProgressBar SHALL draw a progress arc spanning exactly `progress * 3.6` degrees
3. THE CircularProgressBar SHALL display the percentage value as centered text inside the circle
4. THE CircularProgressBar SHALL draw a background arc as a full circle in a muted color
5. THE CircularProgressBar SHALL support customizable line width, background color, and progress color properties

### Requirement 4: Hierarchical Entity Management

**User Story:** As a user, I want to organize my courses and projects into hierarchical structures with units and sessions/tasks, so that I can break down complex work into manageable components.

#### Acceptance Criteria

1. THE Application SHALL support a three-level hierarchy: Entity (Course/Project) → Units → Sessions/Tasks
2. THE Application SHALL allow users to create, rename, and delete Units within a Course or Project
3. THE Application SHALL allow users to create, rename, and delete Sessions/Tasks within a Unit
4. WHEN a Unit is deleted, THE Application SHALL cascade delete all associated Sessions/Tasks
5. WHEN an Entity is deleted, THE Application SHALL cascade delete all associated Units and Sessions/Tasks

### Requirement 5: Expandable Unit Display

**User Story:** As a user, I want to expand and collapse units to see their contained sessions/tasks, so that I can focus on the relevant parts of my hierarchy.

#### Acceptance Criteria

1. THE UnitExpandableWidget SHALL display the unit name with an expand/collapse button
2. WHEN a user clicks the expand button, THE UnitExpandableWidget SHALL reveal all child Session/Task rows
3. WHEN a user clicks the collapse button, THE UnitExpandableWidget SHALL hide all child Session/Task rows
4. THE UnitExpandableWidget SHALL display the overall unit progress calculated from its child Sessions/Tasks
5. THE UnitExpandableWidget SHALL support adding and removing Session/Task widgets dynamically

### Requirement 6: Progress Slider Control

**User Story:** As a user, I want to adjust the progress of my sessions/tasks using a slider, so that I can record my completion status with precision.

#### Acceptance Criteria

1. THE ProgressSlider SHALL provide a slider control with a range from 0 to 100
2. WHEN a user presses the slider, THE ProgressSlider SHALL record the current progress value as the old value
3. WHEN a user releases the slider, THE ProgressSlider SHALL emit a progressChanged signal with the old and new values
4. THE ProgressSlider SHALL display the current progress value as a label alongside the slider
5. IF the progress value is updated externally, THE ProgressSlider SHALL update its visual state to reflect the new value

### Requirement 7: Inline Name Editing

**User Story:** As a user, I want to rename my sessions/tasks by double-clicking their names, so that I can quickly update labels without additional dialogs.

#### Acceptance Criteria

1. WHEN a user double-clicks a Session/Task name label, THE Application SHALL switch the label to an editable QLineEdit
2. WHEN the user finishes editing (presses Enter or focus leaves the edit field), THE Application SHALL save the new name and switch back to label display
3. IF the new name is empty or whitespace-only, THE Application SHALL revert to the previous name
4. THE Application SHALL emit a nameChanged signal when the name is successfully updated

### Requirement 8: Activity Logging

**User Story:** As a user, I want my progress changes to be automatically logged, so that I can track my activity history over time.

#### Acceptance Criteria

1. WHEN a ProgressSlider is released at a new position, THE Application SHALL create an ActivityLog entry
2. THE ActivityLog entry SHALL include ItemID, Timestamp, OldValue, NewValue, and Type fields
3. THE Timestamp SHALL be automatically set to the current date and time
4. IF the new progress value equals the old value, THE Application SHALL NOT create an ActivityLog entry
5. THE Application SHALL calculate and store the ProgressDelta as the absolute difference between NewValue and OldValue

### Requirement 9: GitHub-Style Contribution Heatmap

**User Story:** As a user, I want to see my activity visualized as a GitHub-style heatmap, so that I can understand my work patterns over time.

#### Acceptance Criteria

1. THE ContributionHeatmap SHALL render a configurable-range grid with 7 rows (days) and N columns (weeks), where N defaults to 12 on the Home page and is configurable up to 53 on the Analytics page
2. THE ContributionHeatmap SHALL map daily activity totals to a 5-level color gradient (intensities 0–4) using the dark-industrial green ramp
3. THE ContributionHeatmap SHALL calculate intensity as `floor((dailyTotal / maxDailyTotal) * 4)` bounded to [0, 4]
4. WHEN a user hovers over a day cell, THE ContributionHeatmap SHALL display a tooltip showing the date and total tasks/progress for that day
5. THE ContributionHeatmap SHALL support year/range navigation when displayed on the Analytics page
6. THE ContributionHeatmap SHALL draw month labels at the top and abbreviated day-of-week labels on the left side
7. THE ContributionHeatmap SHALL render a summary line below the grid showing total days tracked and total tasks completed in the visible window

### Requirement 10: JSON Data Import

**User Story:** As a user, I want to import course and project data from JSON files, so that I can quickly populate my tracking system.

#### Acceptance Criteria

1. WHEN a user selects an import file, THE Application SHALL validate that the file contains valid UTF-8 encoded JSON
2. THE Application SHALL require the JSON to include version, type, name, and units fields
3. IF the type field is not "course" or "project", THE Application SHALL reject the import with an error message
4. THE Application SHALL create the entity, units, and sessions/tasks in a single atomic transaction
5. IF any session progress value is outside the range [0, 100], THE Application SHALL clamp the value to the valid range
6. IF the JSON contains invalid entries, THE Application SHALL log warnings but continue importing valid entries

### Requirement 11: JSON Data Export

**User Story:** As a user, I want to export my course and project data to JSON files, so that I can backup or share my tracking data.

#### Acceptance Criteria

1. WHEN a user requests an export, THE Application SHALL generate a JSON document containing all courses and projects
2. THE JSON export SHALL include version, type, name, and units fields for each entity
3. THE JSON export SHALL preserve the hierarchical structure of entities, units, and sessions/tasks
4. THE Application SHALL write the JSON file using UTF-8 encoding
5. IF the export file cannot be written, THE Application SHALL display an error message and not modify any existing files

### Requirement 12: SQLite Data Persistence

**User Story:** As a user, I want my data automatically saved to a local database, so that I can work offline without losing progress.

#### Acceptance Criteria

1. THE Application SHALL use SQLite as the data persistence layer
2. THE DatabaseManager SHALL create the database schema automatically if tables do not exist
3. WHEN any data is modified, THE Application SHALL persist changes to SQLite immediately
4. THE Application SHALL support offline operation without requiring network connectivity
5. THE DatabaseManager SHALL use parameter binding for all SQL queries to prevent SQL injection

### Requirement 13: Database Schema Integrity

**User Story:** As a system, I want to maintain referential integrity in the database, so that the data remains consistent and valid.

#### Acceptance Criteria

1. THE CoursesProjects table SHALL store both courses and projects with a Type discriminator column
2. THE Units table SHALL reference the CoursesProjects table via a foreign key with CASCADE DELETE
3. THE SessionsTasks table SHALL reference the Units table via a foreign key with CASCADE DELETE
4. THE ActivityLog table SHALL reference the SessionsTasks table via a foreign key with CASCADE DELETE
5. THE SessionsTasks.CurrentProgress column SHALL have a CHECK constraint ensuring values are in [0, 100]

### Requirement 14: Overall Progress Calculation

**User Story:** As a user, I want to see the overall progress of my courses and projects, so that I can understand my completion status at a glance.

#### Acceptance Criteria

1. WHEN calculating entity overall progress, THE Application SHALL compute the arithmetic mean of all Session/Task progress values
2. IF an entity has no Sessions/Tasks, THE Application SHALL return 0 as the overall progress
3. WHEN a Session/Task progress changes, THE Application SHALL recalculate the parent Unit's overall progress
4. WHEN a Unit's progress changes, THE Application SHALL recalculate the parent Entity's overall progress
5. THE overall progress SHALL be displayed as a rounded integer percentage

### Requirement 15: Dark Industrial Theme

**User Story:** As a user, I want a visually consistent dark theme, so that I can use the application comfortably in low-light environments.

#### Acceptance Criteria

1. THE Application SHALL load and apply a QSS (Qt Style Sheet) for dark industrial styling
2. THE Application SHALL apply the theme to all widgets consistently
3. THE ContributionHeatmap SHALL use a green gradient (5 stops from dark to bright) for activity intensity colors
4. THE CircularProgressBar SHALL use colors compatible with the dark theme
5. THE Application SHALL use the following canonical palette, matching the React design source of truth:
   - Background: `#1a1d24` (main), `#1f2229` (elevated), `#252932` (surface), `#2d323d` (surface hover)
   - Sidebar: `#16181d` (background)
   - Text: `#e4e6eb` (primary), `#9ca3af` (muted), `#6b7280` (subtle)
   - Primary accent: `#10b981` (green), `#059669` (hover), `#064e3b` (muted)
   - Borders: `#2d323d` (subtle), `#404854` (strong)
   - Status: `#10b981` success, `#f59e0b` warning, `#ef4444` error, `#3b82f6` info
6. THE Application SHALL use the following spacing scale: xs=4, sm=8, md=16, lg=24, xl=32, 2xl=48 pixels
7. THE Application SHALL use the following border-radius scale: sm=4, md=6, lg=8, xl=12 pixels
8. THE Application SHALL set base font size to 14px with weights 400/500/600 available

### Requirement 16: Error Handling and Recovery

**User Story:** As a user, I want the application to handle errors gracefully, so that I can recover from issues without data loss.

#### Acceptance Criteria

1. IF the database connection fails, THE Application SHALL display an error dialog and exit with an error code
2. IF the database schema is corrupted, THE Application SHALL offer to recreate the database from scratch or restore from a backup.
3. IF an import file has parse errors, THE Application SHALL log warnings with line numbers and continue importing valid entries
4. IF a progress update fails, THE Application SHALL revert the slider to the previous position and show an error notification
5. IF a rendering failure occurs in a custom widget, THE Application SHALL catch the exception and render a fallback placeholder

### Requirement 17: Course/Project Categories

**User Story:** As a user, I want to tag my courses and projects with color-coded categories, so that I can group and filter related work.

#### Acceptance Criteria

1. THE Application SHALL support user-defined Categories, each with a name and hex color
2. THE Application SHALL ship with five default categories: Algorithms (#10b981), Web Development (#3b82f6), Machine Learning (#8b5cf6), Systems (#f59e0b), Security (#ec4899)
3. THE EntityCard SHALL display a small colored pill (colored dot + category name) when the entity has a category assigned
4. THE Application SHALL allow assigning, changing, or removing the category of any course or project
5. THE SettingsView SHALL provide a Category Management section listing all categories with their color, name, and a count of entities using each category, plus controls to Add and Edit categories
6. WHEN a category is deleted, THE Application SHALL set the categoryId of affected entities to NULL without deleting the entities

### Requirement 18: Course Pause/Resume

**User Story:** As a user, I want to pause inactive courses, so that they stop counting toward my active workload and completion stats.

#### Acceptance Criteria

1. THE Application SHALL store an `active` or `paused` status on each course (defaulting to `active`)
2. THE CourseDetailView SHALL display a "Pause Course" / "Resume Course" toggle button in the top-right header
3. WHEN a course is paused, THE Application SHALL exclude it from the Active Courses count and the Completion Rate calculation on the Home dashboard
4. THE EntityCard for a paused course SHALL render a "Paused" status badge in a muted color
5. THE Application SHALL allow filtering courses by status (All / Active / Paused) on the CoursesView

### Requirement 19: Courses View Filtering & Search

**User Story:** As a user, I want to search and filter my courses to find what I'm looking for quickly.

#### Acceptance Criteria

1. THE CoursesView SHALL display a header with a title, an entity count subtitle, a search input, a Filter toggle button, and an "Add New" primary button
2. WHEN the Filter button is clicked, THE CoursesView SHALL toggle a filter panel containing two dropdowns: Category (All + each category) and Status (All / Active / Paused)
3. THE Filter button SHALL render in the primary accent color when any filter is active
4. WHEN a filter is applied, THE CoursesView SHALL display each active filter as a removable badge with a × icon, plus a "Clear all" link
5. THE search input SHALL filter the displayed cards in real time by case-insensitive substring match on the entity name
6. THE CoursesView SHALL display entity cards in a responsive grid of 1 to 4 columns based on available width
7. IF the filter or search yields no results, THE CoursesView SHALL render an EmptyState widget with a relevant message

### Requirement 20: Projects View with Metadata

**User Story:** As a user, I want richer project tracking with priority, deadline, team, and links so I can manage real engineering work.

#### Acceptance Criteria

1. THE Application SHALL store the following per-project metadata: description, status (`active` / `paused` / `completed`), priority (`high` / `medium` / `low`), deadline date, team members (string list), and external links (list of `{label, url}`)
2. THE Project card SHALL display: name, truncated description (max 2 lines), priority badge (red/amber/gray), deadline badge with countdown ("Xd left" / "Overdue") whose color is red ≤ 3 days, amber ≤ 7 days, gray otherwise, a progress bar, "X/Y tasks" count, and a team icon with member count
3. THE ProjectDetailView SHALL display project name, description, status/priority/deadline badges, a Back button, a circular progress card, a left column with the task checklist, and a right column with project info (deadline, team, links)
4. WHEN a user clicks a checklist task, THE Application SHALL toggle its completion state and re-render the row with a strikethrough and a muted green background when complete
5. THE Project info links SHALL open the target URL in the system default browser when clicked

### Requirement 21: To-Do List View

**User Story:** As a user, I want a dedicated to-do list with priorities, separate from course/project sessions.

#### Acceptance Criteria

1. THE TodoView SHALL provide a top-of-page input field with placeholder "Add a new task..." and a "+" button that creates a Todo when the user presses Enter or clicks the button
2. THE Application SHALL store each Todo with: id, title, completed flag, priority (`high` / `medium` / `low`), and createdAt timestamp
3. THE TodoView SHALL display two stat boxes summarising counts: Active and Completed
4. THE TodoView SHALL render two sections: "Active Tasks" (incomplete todos) and "Completed" (completed todos, rendered at 60% opacity with strikethrough)
5. EACH todo row SHALL display: a checkbox, the title, a priority badge (red/amber/gray), and a trash icon
6. WHEN a user clicks the checkbox, THE Application SHALL toggle the completion state and persist the change
7. WHEN a user clicks the trash icon, THE Application SHALL delete the todo without further confirmation

### Requirement 22: Pomodoro Timer View

**User Story:** As a user, I want an integrated Pomodoro timer to track focused study sessions per course.

#### Acceptance Criteria

1. THE PomodoroView SHALL provide a two-column layout: timer + settings on the left, stats + history on the right
2. THE PomodoroView SHALL provide a mode toggle with two buttons: "Work Session" and "Break" (active in primary accent, inactive in muted gray, disabled while a timer is running)
3. THE PomodoroView SHALL render a large (≈256 px) circular ring around the time display, animating from 100% to 0% as the timer counts down, with MM:SS centered text
4. THE PomodoroView SHALL provide Start / Pause / Resume / Reset control buttons whose visibility/state depends on the timer state (idle / running / paused)
5. THE PomodoroView SHALL provide a Settings card with: Course selector (dropdown of active courses), Work Duration (15/20/25/30/45/50 minutes), Break Duration (5/10/15 minutes); all disabled while a timer is running
6. WHEN a work session completes, THE Application SHALL insert a PomodoroSession row recording `courseId`, `durationMinutes`, and `completedAt`, then auto-switch the timer mode to Break and reset the timer to the break duration
7. WHEN a break completes, THE Application SHALL auto-switch the timer mode back to Work and reset to the configured work duration
8. THE right column SHALL display a "Today's Progress" card (sessions completed + total minutes focused) and a "Recent Sessions" card listing the last 5 sessions with course name, completion time, and duration badge

### Requirement 23: Analytics Charts

**User Story:** As a user, I want detailed charts of my productivity so I can identify trends.

#### Acceptance Criteria

1. THE AnalyticsView SHALL render a top row of four key-metric cards: Day Streak (current + longest), Total Hours (this month), Avg Sessions/Day (last 7 days), Week Comparison (% vs last week)
2. THE AnalyticsView SHALL render a Progress-Over-Time line chart showing completion percentage across 8 weeks
3. THE AnalyticsView SHALL render a Study Hours Per Week bar chart for the last 8 weeks
4. THE AnalyticsView SHALL render a Course Progress Breakdown panel listing each course with a colored horizontal progress bar (using the assigned category color)
5. THE AnalyticsView SHALL render a Time Distribution pie chart of minutes per course (from Pomodoro sessions)
6. THE AnalyticsView SHALL render a Weekly Activity Pattern bar chart showing total sessions per day of week (Mon–Sun)
7. ALL charts SHALL use the dark theme palette, with green primary, custom tooltips, and resize responsively with the view
8. THE AnalyticsView SHALL be implemented using Qt 6 Charts (`Qt6::Charts`)

### Requirement 24: User Profile & Preferences

**User Story:** As a user, I want to configure my profile and app preferences in one place.

#### Acceptance Criteria

1. THE SettingsView SHALL display a Profile card with editable Name, Email, and Study Goals fields, plus a "Save Profile" button
2. THE SettingsView SHALL display a Preferences card with: Default Work Duration dropdown, Default Break Duration dropdown, Enable Notifications checkbox, Sound Effects checkbox, and an "Auto-pause inactive courses after" dropdown (7 / 14 / 30 days / Never)
3. THE SettingsView SHALL display a Data Management card with Export Data, Import Data, and Clear All Data (destructive red) buttons
4. THE SettingsView SHALL display an About card showing version, build info, and license
5. WHEN the user clicks "Clear All Data", THE Application SHALL show a confirmation dialog before performing any destructive action
6. THE Application SHALL persist all profile and preference values in a `Settings` key/value table

### Requirement 25: Calendar Day Details

**User Story:** As a user, I want to attach notes and per-day todos to a calendar date, so I can capture daily planning context.

#### Acceptance Criteria

1. THE Application SHALL store CalendarDayDetail rows keyed by date, containing: notes text, todo list (JSON array of strings), completed list (JSON array of strings)
2. WHEN the user selects a day in the Calendar, THE DayDetails panel SHALL load and display the existing record, or empty fields if none exists
3. WHEN the user adds a todo string to a day's To Do list, THE Application SHALL persist the change immediately
4. WHEN the user moves a todo from To Do to Completed (or back), THE Application SHALL update both lists and persist immediately
5. WHEN the user types into the Notes textarea, THE Application SHALL persist the text on focus loss
6. THE Calendar SHALL render an indicator dot beneath any day that has any non-empty `toDo`, `completed`, or `notes` field

### Requirement 26: Activity Heatmap Source Data

**User Story:** As a user, I want my heatmap to reflect real work — both progress changes and completed Pomodoros and todos.

#### Acceptance Criteria

1. THE Activity Heatmap on the Home page SHALL aggregate daily counts of: completed todos, completed Pomodoro sessions, and ActivityLog entries (one entry per slider release)
2. THE Activity Heatmap intensity formula on the Home page SHALL bucket the daily total into 5 levels: 0 (no activity), 1 (1 item), 2 (2–3 items), 3 (4–6 items), 4 (7+ items)
3. THE Analytics Heatmap SHALL continue to use the normalized formula `floor((dailyTotal / maxDailyTotal) * 4)` for larger time windows
4. THE Application SHALL recompute the heatmap data on any change to ActivityLog, Todos, or PomodoroSessions

### Requirement 27: Application Window & Layout

**User Story:** As a user, I want the app to fit comfortably on a desktop monitor without breaking the layout.

#### Acceptance Criteria

1. THE MainWindow SHALL set a minimum size of 1280×800 pixels
2. THE MainWindow SHALL preserve the SideNavigationBar at a fixed 256px width when the window is resized
3. THE MainWindow SHALL allow the central QStackedWidget area to scroll vertically when content exceeds the viewport
4. THE Application SHALL persist the last window geometry (size and position) across launches using `QSettings`

### Requirement 28: Inline Entity Creation

**User Story:** As a user, I want to create a course or project from the Courses page without navigating away.

#### Acceptance Criteria

1. WHEN the user clicks the "Add New" button on the CoursesView, THE Application SHALL display a modal dialog with fields: Name (required), Type (Course / Project radio), Category (dropdown, optional), and OK / Cancel buttons
2. WHEN the user clicks "Add New" on the ProjectsView, THE Application SHALL display a similar dialog with the Type field hidden and additional fields for Description, Priority, and Deadline
3. WHEN the user clicks OK in either dialog, THE Application SHALL create the entity, persist it, and navigate to its detail view
4. IF the Name field is empty or whitespace-only, THE OK button SHALL be disabled

### Requirement 29: Keyboard Navigation & Accessibility

**User Story:** As a user, I want to navigate the app efficiently via keyboard.

#### Acceptance Criteria

1. THE Application SHALL set logical tab order on every view (header controls → primary content → secondary content)
2. WHEN focus is on a SideNavigationBar button, the Enter or Space key SHALL trigger navigation
3. ALL primary action buttons SHALL display a visible focus ring using the primary accent color at 25% opacity
4. THE TodoView "Add a new task" input SHALL submit when the user presses Enter

### Requirement 30: Data Schema Initialization

**User Story:** As a developer, I want a well-defined initial schema, so that the database is created uniformly on first launch.

#### Acceptance Criteria

1. THE DatabaseManager SHALL define a comprehensive unified schema containing all 10 core tables (`CoursesProjects`, `Units`, `SessionsTasks`, `ActivityLog`, `Categories`, `Todos`, `PomodoroSessions`, `ProjectMeta`, `CalendarDayDetails`, `Settings`) straight away.
2. ON startup, THE DatabaseManager SHALL execute idempotent `CREATE TABLE IF NOT EXISTS` statements for all tables to ensure the database is fully initialized.
3. IF any error occurs during schema creation, THE Application SHALL log the failure and present an error dialog offering to restore from backup or recreate the database.
