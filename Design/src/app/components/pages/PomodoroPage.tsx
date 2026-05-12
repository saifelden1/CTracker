import { useState, useEffect, useRef } from 'react';
import { Play, Pause, RotateCcw, Coffee, BookOpen } from 'lucide-react';
import { Button } from '../Button';
import { Badge } from '../Badge';
import { Card, CardHeader, CardTitle, CardContent } from '../Card';
import * as Select from '@radix-ui/react-select';

type TimerMode = 'work' | 'break';
type TimerStatus = 'idle' | 'running' | 'paused';

interface PomodoroSession {
  id: string;
  courseName: string;
  duration: number;
  completedAt: Date;
}

const mockSessions: PomodoroSession[] = [
  { id: '1', courseName: 'Advanced Algorithms', duration: 25, completedAt: new Date(2026, 4, 12, 9, 30) },
  { id: '2', courseName: 'Database Systems', duration: 25, completedAt: new Date(2026, 4, 12, 10, 30) },
  { id: '3', courseName: 'Advanced Algorithms', duration: 25, completedAt: new Date(2026, 4, 12, 14, 15) },
];

const courses = [
  'Advanced Algorithms & Data Structures',
  'Database Systems',
  'Machine Learning Fundamentals',
  'Operating Systems',
  'Computer Networks',
];

export function PomodoroPage() {
  const [mode, setMode] = useState<TimerMode>('work');
  const [status, setStatus] = useState<TimerStatus>('idle');
  const [timeLeft, setTimeLeft] = useState(25 * 60); // 25 minutes in seconds
  const [workDuration, setWorkDuration] = useState(25);
  const [breakDuration, setBreakDuration] = useState(5);
  const [selectedCourse, setSelectedCourse] = useState<string>(courses[0]);
  const [sessions, setSessions] = useState<PomodoroSession[]>(mockSessions);
  const [sessionsToday, setSessionsToday] = useState(3);

  const intervalRef = useRef<number | null>(null);

  useEffect(() => {
    if (status === 'running') {
      intervalRef.current = window.setInterval(() => {
        setTimeLeft((prev) => {
          if (prev <= 1) {
            handleTimerComplete();
            return 0;
          }
          return prev - 1;
        });
      }, 1000);
    } else {
      if (intervalRef.current) {
        clearInterval(intervalRef.current);
      }
    }

    return () => {
      if (intervalRef.current) {
        clearInterval(intervalRef.current);
      }
    };
  }, [status]);

  const handleTimerComplete = () => {
    setStatus('idle');

    // Play notification sound (in real app)
    // new Audio('/notification.mp3').play();

    if (mode === 'work') {
      // Log completed session
      const newSession: PomodoroSession = {
        id: Date.now().toString(),
        courseName: selectedCourse,
        duration: workDuration,
        completedAt: new Date(),
      };
      setSessions([newSession, ...sessions]);
      setSessionsToday(sessionsToday + 1);

      // Switch to break
      setMode('break');
      setTimeLeft(breakDuration * 60);
    } else {
      // Switch back to work
      setMode('work');
      setTimeLeft(workDuration * 60);
    }
  };

  const handleStart = () => {
    setStatus('running');
  };

  const handlePause = () => {
    setStatus('paused');
  };

  const handleReset = () => {
    setStatus('idle');
    setTimeLeft(mode === 'work' ? workDuration * 60 : breakDuration * 60);
  };

  const handleModeSwitch = (newMode: TimerMode) => {
    setMode(newMode);
    setStatus('idle');
    setTimeLeft(newMode === 'work' ? workDuration * 60 : breakDuration * 60);
  };

  const formatTime = (seconds: number) => {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
  };

  const progress = mode === 'work'
    ? ((workDuration * 60 - timeLeft) / (workDuration * 60)) * 100
    : ((breakDuration * 60 - timeLeft) / (breakDuration * 60)) * 100;

  const totalMinutesToday = sessions
    .filter(s => {
      const today = new Date(2026, 4, 12);
      return s.completedAt.toDateString() === today.toDateString();
    })
    .reduce((sum, s) => sum + s.duration, 0);

  return (
    <div className="h-full overflow-auto">
      <div className="p-8">
        <div className="max-w-5xl mx-auto">
          {/* Header */}
          <div className="mb-8">
            <h2 className="mb-2">Pomodoro Timer</h2>
            <p className="text-foreground-muted">
              Stay focused with timed study sessions
            </p>
          </div>

          <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
            {/* Timer Section */}
            <div className="lg:col-span-2 space-y-6">
              {/* Mode Toggle */}
              <div className="flex gap-3">
                <Button
                  variant={mode === 'work' ? 'primary' : 'secondary'}
                  className="flex-1"
                  onClick={() => handleModeSwitch('work')}
                  disabled={status === 'running'}
                >
                  <BookOpen className="w-4 h-4" />
                  Work Session
                </Button>
                <Button
                  variant={mode === 'break' ? 'primary' : 'secondary'}
                  className="flex-1"
                  onClick={() => handleModeSwitch('break')}
                  disabled={status === 'running'}
                >
                  <Coffee className="w-4 h-4" />
                  Break
                </Button>
              </div>

              {/* Timer Display */}
              <Card>
                <CardContent className="p-8">
                  <div className="text-center">
                    {/* Mode Indicator */}
                    <div className="mb-6">
                      <Badge variant={mode === 'work' ? 'info' : 'success'} className="text-sm">
                        {mode === 'work' ? 'Focus Time' : 'Break Time'}
                      </Badge>
                    </div>

                    {/* Circular Progress */}
                    <div className="relative w-64 h-64 mx-auto mb-8">
                      <svg className="w-full h-full transform -rotate-90" viewBox="0 0 256 256">
                        {/* Background circle */}
                        <circle
                          cx="128"
                          cy="128"
                          r="112"
                          stroke="currentColor"
                          strokeWidth="12"
                          fill="none"
                          className="text-muted"
                        />
                        {/* Progress circle */}
                        <circle
                          cx="128"
                          cy="128"
                          r="112"
                          stroke="currentColor"
                          strokeWidth="12"
                          fill="none"
                          strokeDasharray={2 * Math.PI * 112}
                          strokeDashoffset={2 * Math.PI * 112 * (1 - progress / 100)}
                          className="text-primary transition-all duration-300"
                          strokeLinecap="round"
                        />
                      </svg>
                      <div className="absolute inset-0 flex items-center justify-center">
                        <div className="text-6xl text-foreground font-mono">
                          {formatTime(timeLeft)}
                        </div>
                      </div>
                    </div>

                    {/* Controls */}
                    <div className="flex justify-center gap-4">
                      {status === 'idle' || status === 'paused' ? (
                        <Button size="lg" onClick={handleStart}>
                          <Play className="w-5 h-5" />
                          {status === 'paused' ? 'Resume' : 'Start'}
                        </Button>
                      ) : (
                        <Button size="lg" variant="secondary" onClick={handlePause}>
                          <Pause className="w-5 h-5" />
                          Pause
                        </Button>
                      )}
                      <Button size="lg" variant="secondary" onClick={handleReset}>
                        <RotateCcw className="w-5 h-5" />
                        Reset
                      </Button>
                    </div>
                  </div>
                </CardContent>
              </Card>

              {/* Settings */}
              <Card>
                <CardHeader>
                  <CardTitle>Session Settings</CardTitle>
                </CardHeader>
                <CardContent>
                  <div className="space-y-4">
                    {/* Course Selection */}
                    <div>
                      <label className="block text-foreground mb-2">
                        Current Course
                      </label>
                      <select
                        value={selectedCourse}
                        onChange={(e) => setSelectedCourse(e.target.value)}
                        className="w-full px-3 py-2 bg-input-background border border-input-border rounded-md text-foreground"
                        disabled={status === 'running'}
                      >
                        {courses.map((course) => (
                          <option key={course} value={course}>
                            {course}
                          </option>
                        ))}
                      </select>
                    </div>

                    {/* Duration Settings */}
                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <label className="block text-foreground mb-2">
                          Work Duration (min)
                        </label>
                        <select
                          value={workDuration}
                          onChange={(e) => {
                            setWorkDuration(Number(e.target.value));
                            if (mode === 'work' && status === 'idle') {
                              setTimeLeft(Number(e.target.value) * 60);
                            }
                          }}
                          className="w-full px-3 py-2 bg-input-background border border-input-border rounded-md text-foreground"
                          disabled={status === 'running'}
                        >
                          <option value={15}>15</option>
                          <option value={20}>20</option>
                          <option value={25}>25</option>
                          <option value={30}>30</option>
                          <option value={45}>45</option>
                          <option value={50}>50</option>
                        </select>
                      </div>
                      <div>
                        <label className="block text-foreground mb-2">
                          Break Duration (min)
                        </label>
                        <select
                          value={breakDuration}
                          onChange={(e) => {
                            setBreakDuration(Number(e.target.value));
                            if (mode === 'break' && status === 'idle') {
                              setTimeLeft(Number(e.target.value) * 60);
                            }
                          }}
                          className="w-full px-3 py-2 bg-input-background border border-input-border rounded-md text-foreground"
                          disabled={status === 'running'}
                        >
                          <option value={5}>5</option>
                          <option value={10}>10</option>
                          <option value={15}>15</option>
                        </select>
                      </div>
                    </div>
                  </div>
                </CardContent>
              </Card>
            </div>

            {/* Stats & History */}
            <div className="space-y-6">
              {/* Today's Stats */}
              <Card>
                <CardHeader>
                  <CardTitle>Today's Progress</CardTitle>
                </CardHeader>
                <CardContent>
                  <div className="space-y-4">
                    <div>
                      <div className="text-3xl text-primary mb-1">{sessionsToday}</div>
                      <div className="text-foreground-muted text-sm">Sessions completed</div>
                    </div>
                    <div>
                      <div className="text-3xl text-primary mb-1">{totalMinutesToday}</div>
                      <div className="text-foreground-muted text-sm">Minutes focused</div>
                    </div>
                  </div>
                </CardContent>
              </Card>

              {/* Recent Sessions */}
              <Card>
                <CardHeader>
                  <CardTitle>Recent Sessions</CardTitle>
                </CardHeader>
                <CardContent>
                  <div className="space-y-3">
                    {sessions.slice(0, 5).map((session) => (
                      <div
                        key={session.id}
                        className="flex items-center justify-between py-2 border-b border-border last:border-0"
                      >
                        <div className="flex-1 min-w-0">
                          <div className="text-foreground text-sm truncate">
                            {session.courseName}
                          </div>
                          <div className="text-foreground-subtle text-xs">
                            {session.completedAt.toLocaleTimeString('en-US', {
                              hour: 'numeric',
                              minute: '2-digit',
                            })}
                          </div>
                        </div>
                        <Badge variant="success" className="ml-2">
                          {session.duration}m
                        </Badge>
                      </div>
                    ))}
                  </div>
                </CardContent>
              </Card>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
