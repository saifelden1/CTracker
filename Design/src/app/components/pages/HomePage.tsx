import { Card, CardHeader, CardTitle, CardContent } from '../Card';
import { Calendar } from '../Calendar';
import { TrendingUp, Pause } from 'lucide-react';

// Sample course data with status
const courses = [
  { id: '1', name: 'Advanced Algorithms', status: 'active', progress: 75 },
  { id: '2', name: 'Database Systems', status: 'active', progress: 88 },
  { id: '3', name: 'Machine Learning', status: 'active', progress: 34 },
  { id: '4', name: 'Operating Systems', status: 'active', progress: 91 },
  { id: '5', name: 'Computer Networks', status: 'active', progress: 56 },
  { id: '6', name: 'Cybersecurity', status: 'active', progress: 62 },
  { id: '7', name: 'Web Development', status: 'paused', progress: 45 },
  { id: '8', name: 'Mobile Dev', status: 'paused', progress: 23 },
];

export function HomePage() {
  // Calculate stats for active courses only
  const activeCourses = courses.filter(c => c.status === 'active');
  const pausedCourses = courses.filter(c => c.status === 'paused');
  const activeCount = activeCourses.length;
  const averageCompletion = activeCount > 0
    ? Math.round(activeCourses.reduce((sum, c) => sum + c.progress, 0) / activeCount)
    : 0;

  return (
    <div className="h-full overflow-auto">
      <div className="p-8">
        <div className="max-w-7xl mx-auto">
          {/* Header */}
          <div className="mb-8">
            <h2 className="mb-2">Welcome to CTracker</h2>
            <p className="text-foreground-muted">
              Track your engineering courses and projects in one place.
            </p>
          </div>

          {/* Stats Cards */}
          <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
            <Card>
              <CardHeader>
                <CardTitle>Active Courses</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="flex items-end justify-between mb-3">
                  <div className="text-3xl text-primary">{activeCount}</div>
                  {pausedCourses.length > 0 && (
                    <div className="flex items-center gap-1 text-foreground-muted text-sm">
                      <Pause className="w-3 h-3" />
                      <span>{pausedCourses.length} paused</span>
                    </div>
                  )}
                </div>
                <p className="text-foreground-subtle text-sm">Currently active</p>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Projects</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="flex items-end justify-between mb-3">
                  <div className="text-3xl text-primary">8</div>
                  <div className="text-foreground-muted text-sm">3 due soon</div>
                </div>
                <p className="text-foreground-subtle text-sm">In progress</p>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Completion Rate</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="flex items-end justify-between mb-3">
                  <div className="text-3xl text-primary">{averageCompletion}%</div>
                  <TrendingUp className="w-5 h-5 text-primary" />
                </div>
                <p className="text-foreground-subtle text-sm">Active courses average</p>
              </CardContent>
            </Card>
          </div>

          {/* Calendar */}
          <Calendar />
        </div>
      </div>
    </div>
  );
}
