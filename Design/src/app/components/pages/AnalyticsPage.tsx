import { Card, CardHeader, CardTitle, CardContent } from '../Card';
import { Badge } from '../Badge';
import { TrendingUp, TrendingDown, Award, Target } from 'lucide-react';
import {
  LineChart,
  Line,
  BarChart,
  Bar,
  PieChart,
  Pie,
  Cell,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  ResponsiveContainer,
} from 'recharts';

// Mock data for charts
const progressOverTime = [
  { date: 'Week 1', completion: 45 },
  { date: 'Week 2', completion: 52 },
  { date: 'Week 3', completion: 58 },
  { date: 'Week 4', completion: 65 },
  { date: 'Week 5', completion: 71 },
  { date: 'Week 6', completion: 78 },
  { date: 'Week 7', completion: 82 },
  { date: 'Week 8', completion: 87 },
];

const hoursPerWeek = [
  { week: 'W1', hours: 12 },
  { week: 'W2', hours: 15 },
  { week: 'W3', hours: 18 },
  { week: 'W4', hours: 14 },
  { week: 'W5', hours: 20 },
  { week: 'W6', hours: 22 },
  { week: 'W7', hours: 19 },
  { week: 'W8', hours: 24 },
];

const courseProgress = [
  { name: 'Algorithms', progress: 75, color: '#10b981' },
  { name: 'Database', progress: 88, color: '#3b82f6' },
  { name: 'ML', progress: 34, color: '#8b5cf6' },
  { name: 'OS', progress: 91, color: '#f59e0b' },
  { name: 'Networks', progress: 56, color: '#ec4899' },
];

const timePerCourse = [
  { name: 'Algorithms', value: 28, color: '#10b981' },
  { name: 'Database', value: 22, color: '#3b82f6' },
  { name: 'ML', value: 15, color: '#8b5cf6' },
  { name: 'OS', value: 20, color: '#f59e0b' },
  { name: 'Networks', value: 15, color: '#ec4899' },
];

const activityData = [
  { day: 'Mon', sessions: 3 },
  { day: 'Tue', sessions: 4 },
  { day: 'Wed', sessions: 2 },
  { day: 'Thu', sessions: 5 },
  { day: 'Fri', sessions: 3 },
  { day: 'Sat', sessions: 6 },
  { day: 'Sun', sessions: 4 },
];

export function AnalyticsPage() {
  const currentStreak = 7;
  const longestStreak = 12;
  const totalHours = 156;
  const avgSessionsPerDay = 3.8;

  return (
    <div className="h-full overflow-auto">
      <div className="p-8">
        <div className="max-w-7xl mx-auto">
          {/* Header */}
          <div className="mb-8">
            <h2 className="mb-2">Analytics</h2>
            <p className="text-foreground-muted">
              Track your progress and productivity insights
            </p>
          </div>

          {/* Key Metrics */}
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
            <Card>
              <CardContent className="p-6">
                <div className="flex items-center gap-3 mb-3">
                  <div className="w-10 h-10 rounded-lg bg-primary/10 flex items-center justify-center">
                    <Award className="w-5 h-5 text-primary" />
                  </div>
                  <div>
                    <div className="text-2xl text-primary">{currentStreak}</div>
                    <div className="text-foreground-muted text-sm">Day Streak</div>
                  </div>
                </div>
                <div className="text-foreground-subtle text-xs">
                  Longest: {longestStreak} days
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardContent className="p-6">
                <div className="flex items-center gap-3 mb-3">
                  <div className="w-10 h-10 rounded-lg bg-primary/10 flex items-center justify-center">
                    <TrendingUp className="w-5 h-5 text-primary" />
                  </div>
                  <div>
                    <div className="text-2xl text-primary">{totalHours}h</div>
                    <div className="text-foreground-muted text-sm">Total Hours</div>
                  </div>
                </div>
                <div className="text-foreground-subtle text-xs">
                  This month
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardContent className="p-6">
                <div className="flex items-center gap-3 mb-3">
                  <div className="w-10 h-10 rounded-lg bg-primary/10 flex items-center justify-center">
                    <Target className="w-5 h-5 text-primary" />
                  </div>
                  <div>
                    <div className="text-2xl text-primary">{avgSessionsPerDay}</div>
                    <div className="text-foreground-muted text-sm">Avg Sessions/Day</div>
                  </div>
                </div>
                <div className="text-foreground-subtle text-xs">
                  Last 7 days
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardContent className="p-6">
                <div className="flex items-center gap-3 mb-3">
                  <div className="w-10 h-10 rounded-lg bg-primary/10 flex items-center justify-center">
                    <TrendingUp className="w-5 h-5 text-primary" />
                  </div>
                  <div>
                    <div className="text-2xl text-primary">+12%</div>
                    <div className="text-foreground-muted text-sm">vs Last Week</div>
                  </div>
                </div>
                <div className="text-foreground-subtle text-xs">
                  Productivity increase
                </div>
              </CardContent>
            </Card>
          </div>

          {/* Charts Row 1 */}
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-6">
            {/* Progress Over Time */}
            <Card>
              <CardHeader>
                <CardTitle>Progress Over Time</CardTitle>
              </CardHeader>
              <CardContent>
                <ResponsiveContainer width="100%" height={250}>
                  <LineChart data={progressOverTime}>
                    <CartesianGrid strokeDasharray="3 3" stroke="#2d323d" />
                    <XAxis dataKey="date" stroke="#9ca3af" style={{ fontSize: '12px' }} />
                    <YAxis stroke="#9ca3af" style={{ fontSize: '12px' }} />
                    <Tooltip
                      contentStyle={{
                        backgroundColor: '#1f2229',
                        border: '1px solid #2d323d',
                        borderRadius: '6px',
                        color: '#e4e6eb',
                      }}
                    />
                    <Line
                      type="monotone"
                      dataKey="completion"
                      stroke="#10b981"
                      strokeWidth={2}
                      dot={{ fill: '#10b981', r: 4 }}
                    />
                  </LineChart>
                </ResponsiveContainer>
              </CardContent>
            </Card>

            {/* Hours Per Week */}
            <Card>
              <CardHeader>
                <CardTitle>Study Hours Per Week</CardTitle>
              </CardHeader>
              <CardContent>
                <ResponsiveContainer width="100%" height={250}>
                  <BarChart data={hoursPerWeek}>
                    <CartesianGrid strokeDasharray="3 3" stroke="#2d323d" />
                    <XAxis dataKey="week" stroke="#9ca3af" style={{ fontSize: '12px' }} />
                    <YAxis stroke="#9ca3af" style={{ fontSize: '12px' }} />
                    <Tooltip
                      contentStyle={{
                        backgroundColor: '#1f2229',
                        border: '1px solid #2d323d',
                        borderRadius: '6px',
                        color: '#e4e6eb',
                      }}
                    />
                    <Bar dataKey="hours" fill="#10b981" radius={[4, 4, 0, 0]} />
                  </BarChart>
                </ResponsiveContainer>
              </CardContent>
            </Card>
          </div>

          {/* Charts Row 2 */}
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-6">
            {/* Course Progress Breakdown */}
            <Card>
              <CardHeader>
                <CardTitle>Course Progress Breakdown</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  {courseProgress.map((course) => (
                    <div key={course.name}>
                      <div className="flex items-center justify-between mb-2">
                        <span className="text-foreground text-sm">{course.name}</span>
                        <span className="text-primary text-sm">{course.progress}%</span>
                      </div>
                      <div className="w-full h-2 bg-muted rounded-full overflow-hidden">
                        <div
                          className="h-full transition-all duration-300"
                          style={{
                            width: `${course.progress}%`,
                            backgroundColor: course.color,
                          }}
                        />
                      </div>
                    </div>
                  ))}
                </div>
              </CardContent>
            </Card>

            {/* Time Distribution */}
            <Card>
              <CardHeader>
                <CardTitle>Time Distribution by Course</CardTitle>
              </CardHeader>
              <CardContent>
                <ResponsiveContainer width="100%" height={250}>
                  <PieChart>
                    <Pie
                      data={timePerCourse}
                      cx="50%"
                      cy="50%"
                      outerRadius={80}
                      dataKey="value"
                      label={(entry) => `${entry.name}: ${entry.value}%`}
                      labelLine={false}
                    >
                      {timePerCourse.map((entry) => (
                        <Cell key={`pie-cell-${entry.name}`} fill={entry.color} />
                      ))}
                    </Pie>
                    <Tooltip
                      contentStyle={{
                        backgroundColor: '#1f2229',
                        border: '1px solid #2d323d',
                        borderRadius: '6px',
                        color: '#e4e6eb',
                      }}
                    />
                  </PieChart>
                </ResponsiveContainer>
              </CardContent>
            </Card>
          </div>

          {/* Weekly Activity */}
          <Card>
            <CardHeader>
              <CardTitle>Weekly Activity Pattern</CardTitle>
            </CardHeader>
            <CardContent>
              <ResponsiveContainer width="100%" height={200}>
                <BarChart data={activityData}>
                  <CartesianGrid strokeDasharray="3 3" stroke="#2d323d" />
                  <XAxis dataKey="day" stroke="#9ca3af" style={{ fontSize: '12px' }} />
                  <YAxis stroke="#9ca3af" style={{ fontSize: '12px' }} />
                  <Tooltip
                    contentStyle={{
                      backgroundColor: '#1f2229',
                      border: '1px solid #2d323d',
                      borderRadius: '6px',
                      color: '#e4e6eb',
                    }}
                  />
                  <Bar dataKey="sessions" fill="#10b981" radius={[4, 4, 0, 0]} />
                </BarChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  );
}
