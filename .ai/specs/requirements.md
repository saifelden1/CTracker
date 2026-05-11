# Requirements Document: CTracker

## Introduction

CTracker is an offline Engineering Course & Project Management Suite built with C++17 and Qt 6. It provides hierarchical tracking for courses and projects with custom circular progress indicators, GitHub-style contribution heatmap analytics, and automatic time-series activity logging. The application follows strict Model-View separation, uses SQLite for persistence, and implements a dark industrial theme.

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
- **Heatmap**: A GitHub-style 52-week grid visualization of activity intensity
- **CircularProgressBar**: A custom widget displaying progress as a circular arc
- **SideNavigationBar**: A vertical navigation bar with icon buttons
- **HomeDashboard**: The main view displaying all entities as interactive cards
- **EntityCard**: A card widget displaying a course or project with progress indicator
- **UnitExpandableWidget**: An expandable/collapsible widget containing Sessions/Tasks
- **ProgressSlider**: A slider control for adjusting Session/Task progress
- **DatabaseManager**: The centralized database access layer

## Requirements

### Requirement 1: Application Navigation Structure

**User Story:** As a user, I want a consistent navigation interface, so that I can easily switch between different views of the application.

#### Acceptance Criteria

1. THE Application SHALL provide a side navigation bar with icon buttons for Home, Courses, Projects, Analytics, and Settings views
2. WHEN a user clicks a navigation button, THE Application SHALL switch the main content area to display the corresponding view
3. THE Application SHALL highlight the currently active navigation button in the side navigation bar
4. THE Application SHALL use a QStackedWidget to manage the main content area for view switching

### Requirement 2: Home Dashboard Display

**User Story:** As a user, I want to see all my courses and projects on the home dashboard, so that I can quickly assess my overall progress and navigate to specific items.

#### Acceptance Criteria

1. THE HomeDashboard SHALL display all courses and projects as EntityCard widgets in a scrollable grid layout
2. THE EntityCard SHALL display the entity name, type (Course or Project), and overall progress percentage
3. THE EntityCard SHALL include a CircularProgressBar showing the overall completion percentage
4. WHEN a user clicks an EntityCard, THE Application SHALL navigate to the corresponding Course or Project detail view
5. THE HomeDashboard SHALL emit courseSelected or projectSelected signals when a card is clicked

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

1. THE ContributionHeatmap SHALL render a 52-week grid with 7 rows (days) and 53 columns (weeks)
2. THE ContributionHeatmap SHALL map daily activity totals to a 5-level color gradient (0-4 intensity)
3. THE ContributionHeatmap SHALL calculate intensity as `floor((dailyTotal / maxDailyTotal) * 4)` bounded to [0, 4]
4. WHEN a user hovers over a day cell, THE ContributionHeatmap SHALL display a tooltip showing the date and total progress
5. THE ContributionHeatmap SHALL support year navigation to view historical activity
6. THE ContributionHeatmap SHALL draw month labels at the top and day labels on the left side

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
3. THE ContributionHeatmap SHALL use a GitHub-style green gradient for activity intensity colors
4. THE CircularProgressBar SHALL use colors compatible with the dark theme

### Requirement 16: Error Handling and Recovery

**User Story:** As a user, I want the application to handle errors gracefully, so that I can recover from issues without data loss.

#### Acceptance Criteria

1. IF the database connection fails, THE Application SHALL display an error dialog and exit with an error code
2. IF the database schema is corrupted, THE Application SHALL attempt automatic migration or offer to recreate the database
3. IF an import file has parse errors, THE Application SHALL log warnings with line numbers and continue importing valid entries
4. IF a progress update fails, THE Application SHALL revert the slider to the previous position and show an error notification
5. IF a rendering failure occurs in a custom widget, THE Application SHALL catch the exception and render a fallback placeholder
