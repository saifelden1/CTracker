import { ChevronLeft, ChevronRight, X, Check, Circle, StickyNote } from 'lucide-react';
import { useState } from 'react';
import { Badge } from './Badge';
import { ActivityHeatmap } from './ActivityHeatmap';

interface CalendarEvent {
  date: Date;
  title: string;
  type: 'deadline' | 'exam' | 'event';
}

interface DayDetail {
  day: number;
  month: number;
  year: number;
  toDo: string[];
  completed: string[];
  notes: string;
}

const sampleEvents: CalendarEvent[] = [
  { date: new Date(2026, 4, 15), title: 'Database Assignment Due', type: 'deadline' },
  { date: new Date(2026, 4, 18), title: 'Algorithms Midterm', type: 'exam' },
  { date: new Date(2026, 4, 22), title: 'Project Presentation', type: 'event' },
  { date: new Date(2026, 4, 25), title: 'ML Project Due', type: 'deadline' },
];

// Mock day details - in real app this would come from database
const mockDayDetails: Record<string, DayDetail> = {
  '2026-4-15': {
    day: 15,
    month: 4,
    year: 2026,
    toDo: ['Database Assignment Due', 'Review SQL queries', 'Test application'],
    completed: ['Read chapter 8', 'Complete practice problems'],
    notes: 'Focus on normalization and indexing for the assignment'
  },
  '2026-4-18': {
    day: 18,
    month: 4,
    year: 2026,
    toDo: ['Algorithms Midterm', 'Review sorting algorithms', 'Practice graph problems'],
    completed: ['Study dynamic programming', 'Complete mock exam'],
    notes: 'Exam covers chapters 1-6. Bring calculator and ID.'
  },
  '2026-4-12': {
    day: 12,
    month: 4,
    year: 2026,
    toDo: ['Work on ML project', 'Attend office hours'],
    completed: ['Morning lecture attended', 'Completed lab 3'],
    notes: 'Today went well, made good progress on the project'
  }
};

export function Calendar() {
  const [currentDate, setCurrentDate] = useState(new Date(2026, 4, 12));
  const [selectedDay, setSelectedDay] = useState<number | null>(null);

  const year = currentDate.getFullYear();
  const month = currentDate.getMonth();

  const firstDay = new Date(year, month, 1).getDay();
  const daysInMonth = new Date(year, month + 1, 0).getDate();

  const monthNames = [
    'January', 'February', 'March', 'April', 'May', 'June',
    'July', 'August', 'September', 'October', 'November', 'December'
  ];

  const dayNames = ['S', 'M', 'T', 'W', 'T', 'F', 'S'];

  const calendarDays: (number | null)[] = [];
  for (let i = 0; i < firstDay; i++) {
    calendarDays.push(null);
  }
  for (let day = 1; day <= daysInMonth; day++) {
    calendarDays.push(day);
  }

  const hasEvent = (day: number) => {
    return sampleEvents.some(
      event => event.date.getDate() === day &&
               event.date.getMonth() === month &&
               event.date.getFullYear() === year
    );
  };

  const isToday = (day: number) => {
    const today = new Date(2026, 4, 12);
    return day === today.getDate() &&
           month === today.getMonth() &&
           year === today.getFullYear();
  };

  const previousMonth = () => {
    setCurrentDate(new Date(year, month - 1, 1));
    setSelectedDay(null);
  };

  const nextMonth = () => {
    setCurrentDate(new Date(year, month + 1, 1));
    setSelectedDay(null);
  };

  const handleDayClick = (day: number | null) => {
    if (day) {
      setSelectedDay(day === selectedDay ? null : day);
    }
  };

  const getDayKey = (day: number) => `${year}-${month}-${day}`;
  const selectedDayDetails = selectedDay ? mockDayDetails[getDayKey(selectedDay)] : null;

  return (
    <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
      {/* Left Column - Calendar */}
      <div className="bg-card border border-card-border rounded-lg p-5">
        {/* Header */}
        <div className="flex items-center justify-between mb-4">
          <h3 className="text-card-foreground">
            {monthNames[month]} {year}
          </h3>
          <div className="flex items-center gap-2">
            <button
              onClick={previousMonth}
              className="p-1 hover:bg-surface rounded transition-colors"
            >
              <ChevronLeft className="w-4 h-4 text-foreground-muted" />
            </button>
            <button
              onClick={nextMonth}
              className="p-1 hover:bg-surface rounded transition-colors"
            >
              <ChevronRight className="w-4 h-4 text-foreground-muted" />
            </button>
          </div>
        </div>

        {/* Day names */}
        <div className="grid grid-cols-7 gap-1 mb-1">
          {dayNames.map((day, index) => (
            <div key={index} className="text-center text-foreground-muted text-xs py-1">
              {day}
            </div>
          ))}
        </div>

        {/* Calendar grid */}
        <div className="grid grid-cols-7 gap-1">
          {calendarDays.map((day, index) => (
            <button
              key={index}
              onClick={() => handleDayClick(day)}
              disabled={day === null}
              className={`
                aspect-square flex items-center justify-center rounded relative
                transition-all text-sm
                ${day === null ? 'cursor-default' : 'cursor-pointer hover:bg-surface'}
                ${isToday(day || 0) ? 'bg-primary text-primary-foreground' : ''}
                ${selectedDay === day && !isToday(day || 0) ? 'bg-surface ring-2 ring-primary' : ''}
                ${!isToday(day || 0) && selectedDay !== day && day !== null ? 'text-foreground' : ''}
              `}
            >
              {day && (
                <>
                  <span>{day}</span>
                  {hasEvent(day) && !isToday(day) && selectedDay !== day && (
                    <div className="absolute bottom-0.5 left-1/2 -translate-x-1/2 w-1 h-1 rounded-full bg-primary" />
                  )}
                </>
              )}
            </button>
          ))}
        </div>

        {/* Quick upcoming */}
        <div className="mt-4 pt-4 border-t border-border">
          <h4 className="text-foreground-muted text-xs mb-2">Upcoming</h4>
          <div className="space-y-1">
            {sampleEvents.slice(0, 2).map((event, index) => (
              <div key={index} className="flex items-center gap-2 text-xs">
                <div className={`
                  w-1.5 h-1.5 rounded-full shrink-0
                  ${event.type === 'deadline' ? 'bg-badge-warning' : ''}
                  ${event.type === 'exam' ? 'bg-badge-error' : ''}
                  ${event.type === 'event' ? 'bg-badge-info' : ''}
                `} />
                <div className="flex-1 truncate text-foreground">{event.title}</div>
              </div>
            ))}
          </div>
        </div>
      </div>

      {/* Right Column - Day Details & Heatmap */}
      <div className="flex flex-col gap-6">
        {/* Day Details */}
        <div className="bg-card border border-card-border rounded-lg p-5">
          {selectedDay ? (
            <>
              {/* Header */}
              <div className="flex items-center justify-between mb-4">
                <h3 className="text-card-foreground">
                  {monthNames[month]} {selectedDay}, {year}
                </h3>
                <button
                  onClick={() => setSelectedDay(null)}
                  className="p-1 hover:bg-surface rounded transition-colors"
                >
                  <X className="w-4 h-4 text-foreground-muted" />
                </button>
              </div>

              {selectedDayDetails ? (
                <div className="space-y-4">
                  {/* To Do */}
                  <div>
                    <div className="flex items-center gap-2 mb-2">
                      <Circle className="w-4 h-4 text-foreground-muted" />
                      <h4 className="text-foreground text-sm">To Do</h4>
                      <Badge variant="default">{selectedDayDetails.toDo.length}</Badge>
                    </div>
                    <div className="space-y-1.5 ml-6">
                      {selectedDayDetails.toDo.map((task, index) => (
                        <div key={index} className="flex items-start gap-2 text-sm">
                          <div className="w-1 h-1 rounded-full bg-foreground-muted mt-1.5 shrink-0" />
                          <span className="text-foreground-muted">{task}</span>
                        </div>
                      ))}
                    </div>
                  </div>

                  {/* Completed */}
                  <div>
                    <div className="flex items-center gap-2 mb-2">
                      <Check className="w-4 h-4 text-primary" />
                      <h4 className="text-foreground text-sm">Completed</h4>
                      <Badge variant="success">{selectedDayDetails.completed.length}</Badge>
                    </div>
                    <div className="space-y-1.5 ml-6">
                      {selectedDayDetails.completed.map((task, index) => (
                        <div key={index} className="flex items-start gap-2 text-sm">
                          <div className="w-1 h-1 rounded-full bg-primary mt-1.5 shrink-0" />
                          <span className="text-primary line-through">{task}</span>
                        </div>
                      ))}
                    </div>
                  </div>

                  {/* Notes */}
                  <div>
                    <div className="flex items-center gap-2 mb-2">
                      <StickyNote className="w-4 h-4 text-foreground-muted" />
                      <h4 className="text-foreground text-sm">Notes</h4>
                    </div>
                    <div className="ml-6 bg-surface border border-border rounded p-3">
                      <p className="text-foreground-muted text-sm">{selectedDayDetails.notes}</p>
                    </div>
                  </div>
                </div>
              ) : (
                <div className="text-center py-8">
                  <p className="text-foreground-muted text-sm">No details for this day</p>
                  <p className="text-foreground-subtle text-xs mt-1">
                    Tasks and notes will appear here automatically
                  </p>
                </div>
              )}
            </>
          ) : (
            <div className="flex items-center justify-center py-12">
              <div className="text-center">
                <div className="w-12 h-12 rounded-full bg-muted flex items-center justify-center mx-auto mb-3">
                  <Circle className="w-6 h-6 text-muted-foreground" />
                </div>
                <p className="text-foreground-muted text-sm">Select a day to view details</p>
                <p className="text-foreground-subtle text-xs mt-1">
                  Click on any date in the calendar
                </p>
              </div>
            </div>
          )}
        </div>

        {/* Activity Heatmap */}
        <ActivityHeatmap />
      </div>
    </div>
  );
}
