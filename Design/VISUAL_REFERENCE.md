# CTracker Visual Reference (React Prototype)

> **CRITICAL**: This folder contains a React+TypeScript+Tailwind prototype for **VISUAL REFERENCE ONLY**.
> **NO React/TypeScript/Tailwind code is ported to the Qt implementation.**
> The Qt app reproduces layouts, colors, spacing, and interactions using native Qt widgets + QSS.

## Design Tokens (for Qt QSS)

### Colors
```css
/* Backgrounds */
--background: #1a1d24
--elevated: #1f2229
--surface: #252932
--surface-hover: #2d323d
--sidebar: #16181d

/* Text */
--text: #e4e6eb
--text-muted: #9ca3af
--text-subtle: #6b7280

/* Primary (Green) */
--primary: #10b981
--primary-hover: #059669
--primary-muted: #064e3b

/* Borders */
--border: #2d323d
--border-strong: #404854

/* Status */
--success: #10b981
--warning: #f59e0b
--error: #ef4444
--info: #3b82f6
```

### Spacing (px)
xs=4, sm=8, md=16, lg=24, xl=32, 2xl=48

### Border Radius (px)
sm=4, md=6, lg=8, xl=12

### Typography
Base: 14px | Weights: 400/500/600

## Layout Structure

```
┌─────────────────────────────────────────┐
│ Sidebar (256px)  │  Main Content        │
│ ─────────────    │  ──────────────      │
│ • Home           │  [Page Content]      │
│ • Courses        │                      │
│ • Projects       │                      │
│ • To-Do          │                      │
│ • Pomodoro       │                      │
│ • Analytics      │                      │
│ • Settings       │                      │
│ ─────────────    │                      │
│ [User Profile]   │                      │
└─────────────────────────────────────────┘
```

## Key UI Patterns

### EntityCard (Course/Project Card)
- 160×180px fixed size
- CircularProgressBar (center)
- CategoryPill (top-left, if assigned)
- Status badge (top-right, if paused)
- Hover: border highlight + shadow

### ProjectCard
- Name + 2-line description
- Priority badge (red/amber/gray)
- Deadline badge with countdown
- Horizontal progress bar
- Task count + team size

### Filter Pattern
- Search input (debounced 200ms)
- Collapsible filter panel
- Active filter badges with × remove
- "Clear all" link

### Progress Controls
- Circular: QPainter arc, 0-360°
- Slider: 0-100 range, press/release events
- Horizontal bar: QProgressBar

## Component Mapping (React → Qt)

| React Component | Qt Equivalent |
|----------------|---------------|
| `<Button>` | `QPushButton` |
| `<Input>` | `QLineEdit` |
| `<Card>` | `QFrame` + QSS |
| `<Badge>` | `QLabel` + styled |
| `<Slider>` | `QSlider` |
| `<Checkbox>` | `QCheckBox` |
| `<Combobox>` | `QComboBox` |
| `<Dialog>` | `QDialog` |
| Recharts | `Qt6::Charts` |
| Lucide icons | SVG via `Qt6::Svg` |

## Running the Prototype (Optional)

```bash
cd Design
npm install
npm run dev
```

**Purpose**: Visual reference for layouts, spacing, colors, and interactions.
**Not Used**: For code generation, bundling, or shipping.

---

**For full implementation details, see the Qt C++ codebase in `CTracker/`.**
