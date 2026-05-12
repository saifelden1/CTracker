import { useState } from 'react';

interface HeatmapData {
  date: Date;
  count: number;
}

// Generate mock data for the last 12 weeks
const generateMockData = (): HeatmapData[] => {
  const data: HeatmapData[] = [];
  const today = new Date(2026, 4, 12); // May 12, 2026

  for (let i = 83; i >= 0; i--) {
    const date = new Date(today);
    date.setDate(date.getDate() - i);

    // Random activity count (0-4 for intensity levels)
    const count = Math.floor(Math.random() * 5);
    data.push({ date, count });
  }

  return data;
};

export function ActivityHeatmap() {
  const [hoveredCell, setHoveredCell] = useState<HeatmapData | null>(null);
  const heatmapData = generateMockData();

  // Get intensity color based on count
  const getIntensityClass = (count: number) => {
    if (count === 0) return 'bg-muted';
    if (count === 1) return 'bg-primary/20';
    if (count === 2) return 'bg-primary/40';
    if (count === 3) return 'bg-primary/60';
    return 'bg-primary';
  };

  // Group data by weeks (7 days per week)
  const weeks: HeatmapData[][] = [];
  for (let i = 0; i < heatmapData.length; i += 7) {
    weeks.push(heatmapData.slice(i, i + 7));
  }

  const monthNames = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
  const dayLabels = ['Mon', 'Wed', 'Fri'];

  // Get month labels for the heatmap
  const getMonthLabels = () => {
    const labels: { month: string; index: number }[] = [];
    let lastMonth = -1;

    weeks.forEach((week, weekIndex) => {
      const firstDay = week[0];
      if (firstDay) {
        const month = firstDay.date.getMonth();
        if (month !== lastMonth && weekIndex > 0) {
          labels.push({ month: monthNames[month], index: weekIndex });
          lastMonth = month;
        }
      }
    });

    return labels;
  };

  const monthLabels = getMonthLabels();

  return (
    <div className="bg-card border border-card-border rounded-lg p-5">
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-card-foreground text-sm">Activity Overview</h3>
        <div className="flex items-center gap-2">
          <span className="text-foreground-subtle text-xs">Less</span>
          <div className="flex gap-1">
            {[0, 1, 2, 3, 4].map((level) => (
              <div
                key={level}
                className={`w-2.5 h-2.5 rounded-sm ${getIntensityClass(level)}`}
              />
            ))}
          </div>
          <span className="text-foreground-subtle text-xs">More</span>
        </div>
      </div>

      <div className="relative">
        {/* Month labels */}
        <div className="flex mb-1 ml-6">
          {monthLabels.map((label, index) => (
            <div
              key={index}
              style={{ marginLeft: index === 0 ? `${label.index * 10}px` : '0' }}
              className="text-foreground-muted text-xs"
            >
              {label.month}
            </div>
          ))}
        </div>

        <div className="flex gap-1">
          {/* Day labels */}
          <div className="flex flex-col gap-1 justify-between py-0.5">
            {dayLabels.map((day, index) => (
              <div key={index} className="text-foreground-muted text-xs h-2.5 flex items-center">
                {day}
              </div>
            ))}
          </div>

          {/* Heatmap grid */}
          <div className="flex gap-1 flex-1 overflow-x-auto">
            {weeks.map((week, weekIndex) => (
              <div key={weekIndex} className="flex flex-col gap-1">
                {week.map((day, dayIndex) => (
                  <div
                    key={dayIndex}
                    className={`
                      w-2.5 h-2.5 rounded-sm transition-all cursor-pointer
                      ${getIntensityClass(day.count)}
                      hover:ring-1 hover:ring-primary
                    `}
                    onMouseEnter={() => setHoveredCell(day)}
                    onMouseLeave={() => setHoveredCell(null)}
                  />
                ))}
              </div>
            ))}
          </div>
        </div>

        {/* Tooltip */}
        {hoveredCell && (
          <div className="absolute bottom-full left-1/2 -translate-x-1/2 mb-2 bg-popover border border-popover-border rounded px-2 py-1 text-xs text-popover-foreground whitespace-nowrap shadow-lg z-10">
            <div className="font-medium">{hoveredCell.count} tasks completed</div>
            <div className="text-foreground-subtle">
              {hoveredCell.date.toLocaleDateString('en-US', {
                month: 'short',
                day: 'numeric',
                year: 'numeric'
              })}
            </div>
          </div>
        )}
      </div>

      {/* Summary stats */}
      <div className="mt-4 pt-4 border-t border-border flex items-center justify-between">
        <div>
          <div className="text-foreground text-sm">84 days tracked</div>
          <div className="text-foreground-subtle text-xs">Last 12 weeks</div>
        </div>
        <div className="text-right">
          <div className="text-primary text-sm">
            {heatmapData.reduce((sum, day) => sum + day.count, 0)} tasks
          </div>
          <div className="text-foreground-subtle text-xs">Total completed</div>
        </div>
      </div>
    </div>
  );
}
