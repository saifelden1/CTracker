# CTracker Design System

## Overview
Dark industrial design system for CTracker - an offline engineering course and project management app.

## Design Principles
- **Dark & Industrial**: Professional productivity tool aesthetic
- **High Contrast**: Strong visual hierarchy for readability
- **Minimal Clutter**: Clean, focused interface
- **Green Accents**: Progress and analytics highlighting
- **Desktop-First**: Optimized for desktop workflows

## Color Palette

### Background Colors
- `--background`: #1a1d24 - Main dark gray background
- `--background-elevated`: #1f2229 - Elevated surfaces
- `--surface`: #252932 - Surface elements
- `--surface-hover`: #2d323d - Hover states

### Text Colors
- `--foreground`: #e4e6eb - Primary text (high contrast)
- `--foreground-muted`: #9ca3af - Secondary text
- `--foreground-subtle`: #6b7280 - Tertiary text

### Green Accent (Primary)
- `--primary`: #10b981 - Main green for progress/analytics
- `--primary-hover`: #059669 - Hover state
- `--primary-foreground`: #ffffff - Text on green
- `--primary-muted`: #064e3b - Dark green background

### Borders
- `--border`: #2d323d - Subtle borders
- `--border-strong`: #404854 - More visible borders

### Status Colors
- Success: #10b981 (green)
- Warning: #f59e0b (amber)
- Error: #ef4444 (red)
- Info: #3b82f6 (blue)

## Typography

### Font Sizes
- Base: 14px
- Designed for desktop readability
- Uses system default font stack

### Font Weights
- Normal: 400
- Medium: 500
- Semibold: 600

## Spacing System
- xs: 4px (0.25rem)
- sm: 8px (0.5rem)
- md: 16px (1rem)
- lg: 24px (1.5rem)
- xl: 32px (2rem)
- 2xl: 48px (3rem)

## Border Radius
- sm: 4px - Small elements
- md: 6px - Default (buttons, inputs)
- lg: 8px - Cards
- xl: 12px - Large containers

## Components

### Button
**Variants:**
- `primary`: Green background for main actions
- `secondary`: Gray background for secondary actions
- `ghost`: Transparent, hover state only
- `destructive`: Red for dangerous actions

**Sizes:**
- `sm`: Compact (px-3 py-1.5)
- `md`: Default (px-4 py-2)
- `lg`: Large (px-6 py-3)

### Input
- Dark background with border
- Green focus ring
- Error state with red border

### Badge
**Variants:**
- `success`: Green - completed items
- `warning`: Amber - pending/due soon
- `error`: Red - overdue/failed
- `info`: Blue - general info
- `default`: Gray - neutral status

### Card
- Dark surface with subtle border
- Elevated appearance
- Composed of: Card, CardHeader, CardTitle, CardContent

### Sidebar
- Fixed left navigation
- Width: 256px (64 * 4px)
- Darker than main background
- Active state highlighting with green accent
- User profile section at bottom

## Shadows
- sm: Subtle depth
- md: Standard elevation
- lg: Modal/popover depth

## Usage Examples

```tsx
// Button
<Button variant="primary" size="md">
  <Plus className="w-4 h-4" />
  New Course
</Button>

// Input
<Input placeholder="Search..." error={false} />

// Badge
<Badge variant="success">Completed</Badge>
<Badge variant="warning">Due Soon</Badge>

// Card
<Card>
  <CardHeader>
    <CardTitle>Title</CardTitle>
  </CardHeader>
  <CardContent>
    Content here
  </CardContent>
</Card>
```

## Implementation Notes

- All design tokens are defined in `/src/styles/theme.css`
- Components use Tailwind classes mapped to design tokens
- Built for Qt 6 portability - simple, consistent patterns
- No complex animations or transitions (150ms duration standard)
- Focus on functionality over decoration
