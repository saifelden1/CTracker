# CTracker - Complete System Overview Prompt

Use this prompt to inform an AI about the complete CTracker system architecture, features, and implementation.

---

## 📋 SYSTEM DESCRIPTION

CTracker is a **dark industrial productivity application** for tracking engineering courses, projects, study sessions, and tasks. It features a desktop-optimized dark theme with green accents (#10b981) and is designed to be clean, practical, and professional.

**Target Platform**: Desktop (designed for Qt 6 conversion, currently built in React + Tailwind CSS)

**Tech Stack**:
- Frontend: React 18.3.1 + TypeScript
- Styling: Tailwind CSS v4
- Components: Radix UI (headless components)
- Charts: Recharts
- Icons: Lucide React

---

## 🎨 DESIGN SYSTEM

### Color Palette (Dark Industrial Theme)

**Background Colors**:
- `--background`: #1a1d24 (main dark gray background)
- `--background-elevated`: #1f2229 (slightly lighter for cards)
- `--surface`: #252932 (surface elements)
- `--surface-hover`: #2d323d (hover states)

**Text Colors**:
- `--foreground`: #e4e6eb (primary text - high contrast white)
- `--foreground-muted`: #9ca3af (secondary text - gray)
- `--foreground-subtle`: #6b7280 (tertiary text - darker gray)

**Primary (Green Accent)**:
- `--primary`: #10b981 (main green for progress/analytics)
- `--primary-hover`: #059669 (darker green for hover)
- `--primary-foreground`: #ffffff (text on green)
- `--primary-muted`: #064e3b (dark green background)

**Borders**:
- `--border`: #2d323d (subtle borders)
- `--border-strong`: #404854 (more visible borders)

**Status Colors**:
- Success: #10b981 (green)
- Warning: #f59e0b (amber)
- Error: #ef4444 (red)
- Info: #3b82f6 (blue)

### Spacing System
- xs: 4px
- sm: 8px
- md: 16px
- lg: 24px
- xl: 32px
- 2xl: 48px

### Border Radius
- sm: 4px
- md: 6px (default)
- lg: 8px
- xl: 12px

### Typography
- Base font size: 14px
- Font weights: 400 (normal), 500 (medium), 600 (semibold)
- Line height: 1.5

---

## 🏗️ APPLICATION STRUCTURE

### Navigation (Sidebar - Left Fixed)
Width: 256px (64 * 4px)
Background: #16181d (darker than main background)

**Navigation Items** (top to bottom):
1. Home - Overview dashboard
2. Courses - Course and project management
3. Projects - Dedicated project tracking
4. To-Do - Task management
5. Pomodoro - Focus timer
6. Analytics - Charts and insights
7. Settings - Preferences and configuration

**Sidebar Footer**:
- User profile section with avatar, name, email

### Main Content Area
Fills remaining width, contains page content with overflow scrolling.

---

## 📄 PAGE DESCRIPTIONS

### 1. HOME PAGE

**Layout**: Dashboard with stats and calendar

**Top Section - Stats Cards** (3 cards in a row):

1. **Active Courses Card**:
   - Shows count of courses with `status: 'active'`
   - Displays number of paused courses as secondary info
   - Badge showing paused count if any
   - Excludes paused courses from count

2. **Projects Card**:
   - Total number of in-progress projects
   - Shows count of due soon (3 or fewer)
   - Static for now (8 projects, 3 due soon)

3. **Completion Rate Card**:
   - Average progress of ONLY active courses
   - Excludes paused courses from calculation
   - Displays percentage with trending icon
   - Shows "Active courses average" as description

**Bottom Section - Calendar + Heatmap** (2 columns):

**Left Column - Interactive Calendar**:
- Month/year navigation with arrows
- Compact day names (S M T W T F S)
- Small cells with event indicators (colored dots)
- Today highlighted in green
- Click any day to view details

**Right Column** (stacked vertically):

**Top: Day Details Panel**:
- Shows when a day is selected
- Displays selected date
- Three sections:
  - **To Do**: List of pending tasks with count badge
  - **Completed**: List of finished tasks (green, strikethrough) with count badge
  - **Notes**: Text area with notes for the day
- Empty state: "Select a day to view details"
- X button to close

**Bottom: Activity Heatmap**:
- GitHub-style contribution grid
- Shows last 12 weeks (84 days)
- 5 intensity levels (0-4 tasks completed)
- Color scale from dark to bright green
- Hover tooltip shows date and task count
- Month labels across top
- Day labels (Mon, Wed, Fri) on left
- Summary stats: total days tracked, total tasks completed

---

### 2. COURSES PAGE

**Purpose**: Grid view of all courses and projects with filtering

**Header Section** (fixed at top):
- Title: "Courses"
- Subtitle: Count of total courses/projects
- Search bar with icon (left side)
- Two buttons (right side):
  - **Filter** button (toggles filter panel, turns green when active)
  - **Add New** button (primary green)

**Filter Panel** (appears when Filter button clicked):
- Two dropdowns side by side:
  1. **Category Filter**: 
     - Options: All Categories, Algorithms, Web Development, Machine Learning, Systems, Security
  2. **Status Filter**:
     - Options: All Status, Active, Paused

- **Active Filters Display**:
  - Shows applied filters as clickable badges
  - Each badge has × to remove
  - "Clear all" link to reset all filters

**Content Grid**:
- Responsive grid: 1-4 columns based on screen width
- Cards arranged with equal height
- Empty state if no entities or no search results

**Entity Cards** (Course/Project Cards):

Structure:
```
┌─────────────────────────────┐
│ [Title]          [Type Badge]│
│                               │
│ [Category Badge]              │
│                               │
│ ┌─────┐  Progress             │
│ │ 75% │  75%                  │
│ └─────┘                       │
└─────────────────────────────┘
```

Card Contents:
- **Header**:
  - Title (2 lines max, truncated)
  - Type badge: Blue "Course" or Green "Project"
  
- **Category Badge** (if assigned):
  - Small pill with category color
  - Colored dot + category name
  - Semi-transparent background

- **Progress Section** (bottom):
  - Circular progress ring (56px diameter)
  - Percentage text inside circle
  - "Progress" label
  - Large percentage display

**Interactions**:
- Hover: Border highlights, background lightens, shadow appears
- Click: Opens course/project detail view
- Selected: Green border ring

**Categories** (predefined):
1. Algorithms (#10b981 - green)
2. Web Development (#3b82f6 - blue)
3. Machine Learning (#8b5cf6 - purple)
4. Systems (#f59e0b - amber)
5. Security (#ec4899 - pink)

---

### 3. COURSE DETAIL PAGE

**Accessed by**: Clicking a course card

**Header**:
- Back button "← Back to Courses"
- Course title (large)
- Badges row:
  - Type badge (Course/Project)
  - Status badge (Active/Paused - green/gray)
  - Session count
- **Pause Course / Resume Course** button (top right)
  - Toggles between active/paused status
  - Updates badge in real-time

**Overall Progress Card**:
- Large display with:
  - Circular progress indicator (80px)
  - Progress percentage
  - Horizontal progress bar
  - Text: "X of Y sessions completed"

**Main Content - Units Accordion**:

Each unit is an expandable accordion item:

**Unit Header** (collapsed):
- Chevron icon (rotates on expand)
- Unit name: "Unit X: Title"
- Session count
- Progress percentage
- Mini progress bar (24px wide)

**Unit Content** (expanded):
- List of sessions in cards
- Each session card contains:
  
  **Session Card Structure**:
  ```
  ┌─────────────────────────────────┐
  │ Session 1: Title                 │
  │ 75%          [Completed badge]   │
  │                                   │
  │ [━━━━━━━━○──────] Progress Slider │
  │ 0%  25%  50%  75%  100%          │
  └─────────────────────────────────┘
  ```

  - Session name
  - Progress percentage + "Completed" badge if 100%
  - **Interactive Slider**:
    - Radix UI Slider component
    - Range: 0-100%
    - Step: 5%
    - Green track fill
    - Draggable thumb
    - Labels at 0%, 25%, 50%, 75%, 100%
  - Updates progress in real-time

**Data Structure**:
```typescript
interface Unit {
  id: string;
  name: string;
  sessions: Session[];
}

interface Session {
  id: string;
  name: string;
  progress: number; // 0-100
}
```

---

### 4. PROJECTS PAGE

**Similar to Courses but project-focused**

**Header**: Same as Courses page

**Project Cards** (grid view):
- Project name
- Description (2 lines, truncated)
- Priority badge: High (red), Medium (amber), Low (gray)
- Deadline badge: "Xd left" (color based on urgency)
  - Overdue: red
  - ≤3 days: red
  - ≤7 days: amber
  - >7 days: gray
- Progress bar
- Task count: "X/Y tasks"
- Team member count icon

**Project Detail View** (click card):

**Header**:
- Back button
- Project name
- Description
- Badges: Status, Priority, Deadline countdown

**Progress Card**:
- Circular progress (80px)
- Task completion ratio
- Progress bar

**Main Content**:

**Left Column - Tasks Checklist**:
- Interactive checklist items
- Click to toggle completion
- Checkbox (empty → green checkmark)
- Task text (normal → strikethrough when done)
- Green background when completed

**Right Column - Project Info Card**:
- **Deadline**: Full date display
- **Team Members**: List of names as badges
- **Links**: External links with icons
  - GitHub, Figma, API Docs, etc.
  - Click opens in new tab

**Data Structure**:
```typescript
interface Project {
  id: string;
  name: string;
  description: string;
  progress: number;
  status: 'active' | 'paused' | 'completed';
  priority: 'high' | 'medium' | 'low';
  deadline: Date;
  team?: string[];
  links?: { label: string; url: string }[];
  tasks: Task[];
}

interface Task {
  id: string;
  title: string;
  completed: boolean;
}
```

---

### 5. TO-DO PAGE

**Purpose**: Task management with priorities

**Header**:
- Title + description
- Stats: Active tasks count, Completed count

**Add Task Section**:
- Input field: "Add a new task..."
- Plus button
- Enter key to submit

**Stats Cards** (2 boxes):
1. Active tasks count
2. Completed count

**Active Tasks Section**:
- Header: "Active Tasks"
- Task items (cards):
  - Checkbox (empty, border)
  - Task text
  - Priority badge: High (red), Medium (amber), Low (gray)
  - Delete button (trash icon)
  - Hover: background highlights
  - Click checkbox: marks complete

**Completed Section**:
- Header: "Completed" (muted color)
- Task items (muted):
  - Checkbox (filled green with checkmark)
  - Task text (strikethrough)
  - Delete button
  - Opacity: 60%
  - Click checkbox: uncompletes task

**Task Structure**:
```typescript
interface TodoItem {
  id: string;
  title: string;
  completed: boolean;
  priority: 'high' | 'medium' | 'low';
}
```

---

### 6. POMODORO PAGE

**Purpose**: Focus timer with work/break modes

**Layout**: 2 columns (timer on left, stats on right)

**Left Column - Timer Section**:

**Mode Toggle** (2 buttons):
- Work Session (BookOpen icon)
- Break (Coffee icon)
- Active mode: green, inactive: gray
- Disabled during running timer

**Timer Display Card**:
- Mode indicator badge: "Focus Time" or "Break Time"
- **Circular Progress Ring**:
  - Large circle (256px)
  - Background ring (gray)
  - Progress ring (green)
  - Animates as timer counts down
  - Center: Large time display (MM:SS format)
  
- **Control Buttons**:
  - Start/Resume (when idle/paused)
  - Pause (when running)
  - Reset (always available)

**Settings Card**:
- **Course Selection**: Dropdown to select which course
- **Work Duration**: 15/20/25/30/45/50 minutes
- **Break Duration**: 5/10/15 minutes
- Disabled during running timer

**Right Column - Stats & History**:

**Today's Progress Card**:
- Sessions completed count (large number)
- Total minutes focused (large number)

**Recent Sessions Card**:
- List of last 5 completed sessions
- Each shows:
  - Course name
  - Completion time
  - Duration badge (e.g., "25m")

**Timer Logic**:
- Counts down from selected duration
- Updates every second
- On completion:
  - Plays notification (placeholder)
  - Logs session to history
  - Auto-switches mode (work → break → work)
  - Resets timer to new mode duration

**Data Structure**:
```typescript
interface PomodoroSession {
  id: string;
  courseName: string;
  duration: number; // minutes
  completedAt: Date;
}
```

---

### 7. ANALYTICS PAGE

**Purpose**: Visualize productivity data with charts

**Top Row - Key Metrics** (4 cards):

1. **Day Streak**:
   - Current streak count
   - Award icon
   - Longest streak shown below

2. **Total Hours**:
   - Hours studied this month
   - Trending icon
   
3. **Avg Sessions/Day**:
   - Average over last 7 days
   - Target icon

4. **Week Comparison**:
   - Percentage vs last week
   - "+12%" with trending icon

**Charts Row 1** (2 cards):

1. **Progress Over Time** (Line Chart):
   - X-axis: Weeks (Week 1-8)
   - Y-axis: Completion percentage
   - Green line showing trend
   - Shows improvement over 8 weeks

2. **Study Hours Per Week** (Bar Chart):
   - X-axis: Weeks (W1-W8)
   - Y-axis: Hours
   - Green bars
   - Shows weekly study time

**Charts Row 2** (2 cards):

1. **Course Progress Breakdown**:
   - List of courses with progress bars
   - Each course:
     - Name + percentage
     - Horizontal progress bar (colored by course)
   - Colors match categories

2. **Time Distribution by Course** (Pie Chart):
   - Shows percentage of time per course
   - Color-coded slices
   - Labels show course name + percentage

**Bottom Chart**:

**Weekly Activity Pattern** (Bar Chart):
- X-axis: Days (Mon-Sun)
- Y-axis: Session count
- Shows which days are most productive

**All charts**:
- Dark theme with custom tooltips
- Green primary color
- Responsive sizing
- Mock data for demonstration

---

### 8. SETTINGS PAGE

**Purpose**: App configuration and preferences

**Layout**: Single column, stacked cards

**1. Profile Card**:
- User icon
- **Fields**:
  - Name (text input)
  - Email (text input)
  - Study Goals (textarea)
- Save Profile button

**2. Preferences Card**:
- Bell icon

**Pomodoro Timer Settings**:
- Default Work Duration (dropdown)
- Default Break Duration (dropdown)

**Notifications & Sounds**:
- Enable Notifications (checkbox)
- Sound Effects (checkbox)

**Course Management**:
- Auto-pause inactive courses after: 7/14/30 days/Never (dropdown)

- Save Preferences button

**3. Course Categories Card**:
- Tag icon
- List of categories:
  - Color dot + name + course count
  - Edit button per category
- Add Category button

**4. Data Management Card**:
- Download icon
- **Buttons**:
  - Export Data
  - Import Data
- **Danger Zone**:
  - Clear All Data (red destructive button)

**5. About Card**:
- Version: 1.0.0
- Built with: React + Tailwind CSS
- License: MIT

---

## 🧩 SHARED COMPONENTS

### Button
**Variants**: primary, secondary, ghost, destructive
**Sizes**: sm, md, lg
**Props**: variant, size, children, onClick, disabled

### Badge
**Variants**: success, warning, error, info, default
**Visual**: Small pill, colored background + text
**Props**: variant, children

### Card / CardHeader / CardTitle / CardContent
**Purpose**: Consistent card container
**Structure**: Composable parts
**Styling**: Dark background, subtle border, rounded corners

### Input
**Purpose**: Text input field
**Features**: Dark background, border, focus ring, error state
**Props**: placeholder, value, onChange, error

### CircularProgress
**Purpose**: Circular progress indicator
**Props**: percentage (0-100), size, strokeWidth
**Visual**: SVG circle with green progress arc

### EntityCard
**Purpose**: Course/project card in grid
**Props**: name, type, progress, categoryName, categoryColor, isSelected, onClick
**Features**: Hover effects, category badge, progress ring

### EmptyState
**Purpose**: Empty state messaging
**Props**: title, description, actionLabel, onAction
**Visual**: Icon, text, optional CTA button

### Calendar
**Purpose**: Interactive monthly calendar
**Features**: 
- Month navigation
- Day selection
- Event indicators
- Day detail panel
- Integrated heatmap

### ActivityHeatmap
**Purpose**: GitHub-style contribution grid
**Features**: 
- 12 weeks of data
- 5 intensity levels
- Hover tooltips
- Summary stats

---

## 📊 DATA STRUCTURES

### Course
```typescript
{
  id: string;
  name: string;
  type: 'course' | 'project';
  progress: number; // 0-100
  status: 'active' | 'paused';
  categoryId?: string;
  units: Unit[];
}
```

### Unit
```typescript
{
  id: string;
  name: string;
  sessions: Session[];
}
```

### Session
```typescript
{
  id: string;
  name: string;
  progress: number; // 0-100
}
```

### Category
```typescript
{
  id: string;
  name: string;
  color: string; // hex color
}
```

### Project
```typescript
{
  id: string;
  name: string;
  description: string;
  progress: number;
  status: 'active' | 'paused' | 'completed';
  priority: 'high' | 'medium' | 'low';
  deadline: Date;
  team?: string[];
  links?: { label: string; url: string }[];
  tasks: Task[];
}
```

### Task (Project)
```typescript
{
  id: string;
  title: string;
  completed: boolean;
}
```

### TodoItem
```typescript
{
  id: string;
  title: string;
  completed: boolean;
  priority: 'high' | 'medium' | 'low';
}
```

### PomodoroSession
```typescript
{
  id: string;
  courseName: string;
  duration: number; // minutes
  completedAt: Date;
}
```

### CalendarDayDetail
```typescript
{
  day: number;
  month: number;
  year: number;
  toDo: string[];
  completed: string[];
  notes: string;
}
```

---

## 🔄 USER FLOWS

### Creating a Course
1. Navigate to Courses page
2. Click "Add New" button
3. (Not implemented - would show modal/form)
4. Enter course details
5. Select category
6. Save → appears in grid

### Tracking Progress
1. Click course card in grid
2. Opens course detail view
3. Expand unit accordion
4. Drag session progress slider
5. Progress updates immediately
6. Overall course progress recalculates

### Using Pomodoro Timer
1. Navigate to Pomodoro page
2. Select course from dropdown
3. Set work duration (default 25 min)
4. Click Start
5. Timer counts down
6. On completion:
   - Logs session
   - Switches to break mode
   - Shows in recent sessions

### Filtering Courses
1. Navigate to Courses page
2. Click Filter button
3. Select category and/or status
4. Grid updates automatically
5. Active filters shown as badges
6. Click × on badge to remove filter

### Managing Projects
1. Navigate to Projects page
2. Click project card
3. View task checklist
4. Click task to toggle completion
5. Progress auto-updates
6. View team members and links

---

## 🎯 KEY DESIGN PRINCIPLES

1. **Dark Industrial Theme**: Professional, serious productivity tool
2. **High Contrast**: White text on dark backgrounds for readability
3. **Green Accents**: Progress, success, and primary actions
4. **Minimal Clutter**: Clean layouts, plenty of whitespace
5. **Desktop-First**: Optimized for desktop workflows
6. **Consistent Spacing**: 8px grid system
7. **Smooth Transitions**: 150-300ms animations
8. **Clear Hierarchy**: Visual weight guides attention
9. **Qt-Ready**: Simple patterns for easy conversion

---

## 🚀 TECHNICAL NOTES

**State Management**: 
- React useState for local state
- Props for component communication
- No global state management (yet)

**Data Persistence**: 
- Currently in-memory only
- Resets on refresh
- Ready for localStorage/backend integration

**Routing**: 
- Conditional rendering based on page state
- No React Router (simple page switching)

**Charts**: 
- Recharts library
- Fully customized for dark theme
- Responsive containers

**Accessibility**: 
- Semantic HTML
- Focus states
- Keyboard navigation ready

---

## 📝 CURRENT STATE

**✅ Fully Implemented**:
- All 7 pages with full UI
- Component library
- Design system
- Navigation
- Interactive features (sliders, filters, timers)
- Category system with filtering
- Responsive layouts

**⏳ Partially Implemented**:
- Mock data (hardcoded)
- Timer logic (works but no persistence)
- Task toggle (updates state only)

**❌ Not Implemented**:
- Data persistence (localStorage/database)
- Backend API
- User authentication
- File export/import
- Real notifications
- Calendar event creation
- Settings save functionality

**🎯 Ready For**:
- Qt/QML conversion
- Backend integration
- Data persistence layer
- Production deployment

---

## 💡 USAGE INSTRUCTIONS

When working with this system:

1. **Understand the dark theme** - All components use the design tokens
2. **Follow component patterns** - Reuse existing components
3. **Maintain consistency** - Use spacing/radius/color constants
4. **Test interactions** - Hover, click, keyboard navigation
5. **Consider Qt conversion** - Keep patterns simple
6. **Document changes** - Update this overview when adding features

---

**End of System Overview**

This document fully describes the CTracker application as currently implemented. Use it as a complete reference for understanding the system architecture, features, and design patterns.
