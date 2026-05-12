import { useState } from 'react';
import { ArrowLeft, ChevronDown } from 'lucide-react';
import { Button } from '../Button';
import { Badge } from '../Badge';
import * as Accordion from '@radix-ui/react-accordion';
import * as Slider from '@radix-ui/react-slider';

interface Session {
  id: string;
  name: string;
  progress: number;
}

interface Unit {
  id: string;
  name: string;
  sessions: Session[];
}

interface Course {
  id: string;
  name: string;
  type: 'course' | 'project';
  status: 'active' | 'paused';
  units: Unit[];
}

interface CourseDetailPageProps {
  courseId: string;
  onBack: () => void;
}

// Mock course data
const mockCourses: Record<string, Course> = {
  '1': {
    id: '1',
    name: 'Advanced Algorithms & Data Structures',
    type: 'course',
    status: 'active',
    units: [
      {
        id: 'u1',
        name: 'Unit 1: Introduction to Algorithms',
        sessions: [
          { id: 's1', name: 'Session 1: Big O Notation', progress: 100 },
          { id: 's2', name: 'Session 2: Time Complexity Analysis', progress: 100 },
          { id: 's3', name: 'Session 3: Space Complexity', progress: 75 },
        ],
      },
      {
        id: 'u2',
        name: 'Unit 2: Sorting Algorithms',
        sessions: [
          { id: 's4', name: 'Session 1: Bubble Sort & Selection Sort', progress: 100 },
          { id: 's5', name: 'Session 2: Merge Sort', progress: 60 },
          { id: 's6', name: 'Session 3: Quick Sort', progress: 40 },
          { id: 's7', name: 'Session 4: Heap Sort', progress: 0 },
        ],
      },
      {
        id: 'u3',
        name: 'Unit 3: Dynamic Programming',
        sessions: [
          { id: 's8', name: 'Session 1: Introduction to DP', progress: 0 },
          { id: 's9', name: 'Session 2: Memoization Techniques', progress: 0 },
          { id: 's10', name: 'Session 3: Classic DP Problems', progress: 0 },
        ],
      },
    ],
  },
};

export function CourseDetailPage({ courseId, onBack }: CourseDetailPageProps) {
  const course = mockCourses[courseId] || mockCourses['1'];
  const [units, setUnits] = useState(course.units);
  const [courseStatus, setCourseStatus] = useState<'active' | 'paused'>(course.status);

  const handleProgressChange = (unitId: string, sessionId: string, value: number[]) => {
    setUnits(
      units.map((unit) =>
        unit.id === unitId
          ? {
              ...unit,
              sessions: unit.sessions.map((session) =>
                session.id === sessionId ? { ...session, progress: value[0] } : session
              ),
            }
          : unit
      )
    );
  };

  // Calculate overall course progress
  const totalSessions = units.reduce((sum, unit) => sum + unit.sessions.length, 0);
  const totalProgress = units.reduce(
    (sum, unit) => sum + unit.sessions.reduce((s, session) => s + session.progress, 0),
    0
  );
  const overallProgress = totalSessions > 0 ? Math.round(totalProgress / totalSessions) : 0;

  return (
    <div className="h-full overflow-auto">
      <div className="p-8">
        <div className="max-w-5xl mx-auto">
          {/* Header */}
          <div className="mb-8">
            <Button variant="ghost" onClick={onBack} className="mb-4 -ml-3">
              <ArrowLeft className="w-4 h-4" />
              Back to Courses
            </Button>

            <div className="flex items-start justify-between mb-4">
              <div className="flex-1">
                <h2 className="mb-2">{course.name}</h2>
                <div className="flex items-center gap-3">
                  <Badge variant={course.type === 'course' ? 'info' : 'success'}>
                    {course.type === 'course' ? 'Course' : 'Project'}
                  </Badge>
                  <Badge variant={courseStatus === 'active' ? 'success' : 'default'}>
                    {courseStatus === 'active' ? 'Active' : 'Paused'}
                  </Badge>
                  <span className="text-foreground-muted text-sm">
                    {totalSessions} sessions
                  </span>
                </div>
              </div>

              {/* Status Toggle Button */}
              <Button
                variant={courseStatus === 'active' ? 'secondary' : 'primary'}
                onClick={() => setCourseStatus(courseStatus === 'active' ? 'paused' : 'active')}
              >
                {courseStatus === 'active' ? 'Pause Course' : 'Resume Course'}
              </Button>
            </div>

            {/* Overall Progress */}
            <div className="bg-card border border-card-border rounded-lg p-6">
              <div className="flex items-center justify-between mb-3">
                <h3 className="text-card-foreground">Overall Progress</h3>
                <span className="text-primary text-xl">{overallProgress}%</span>
              </div>
              <div className="w-full h-2 bg-muted rounded-full overflow-hidden">
                <div
                  className="h-full bg-primary transition-all duration-300"
                  style={{ width: `${overallProgress}%` }}
                />
              </div>
            </div>
          </div>

          {/* Units Accordion */}
          <Accordion.Root type="multiple" className="space-y-4">
            {units.map((unit) => {
              const unitProgress =
                unit.sessions.length > 0
                  ? Math.round(
                      unit.sessions.reduce((sum, s) => sum + s.progress, 0) / unit.sessions.length
                    )
                  : 0;

              return (
                <Accordion.Item
                  key={unit.id}
                  value={unit.id}
                  className="bg-card border border-card-border rounded-lg overflow-hidden"
                >
                  <Accordion.Header>
                    <Accordion.Trigger className="w-full px-6 py-4 flex items-center justify-between hover:bg-surface transition-colors group">
                      <div className="flex items-center gap-4 flex-1 text-left">
                        <ChevronDown className="w-5 h-5 text-foreground-muted transition-transform group-data-[state=open]:rotate-180" />
                        <div className="flex-1">
                          <h4 className="text-card-foreground mb-1">{unit.name}</h4>
                          <div className="flex items-center gap-3">
                            <span className="text-foreground-muted text-sm">
                              {unit.sessions.length} sessions
                            </span>
                            <span className="text-primary text-sm">{unitProgress}% complete</span>
                          </div>
                        </div>
                      </div>
                      <div className="w-24 h-1.5 bg-muted rounded-full overflow-hidden ml-4">
                        <div
                          className="h-full bg-primary transition-all duration-300"
                          style={{ width: `${unitProgress}%` }}
                        />
                      </div>
                    </Accordion.Trigger>
                  </Accordion.Header>

                  <Accordion.Content className="overflow-hidden data-[state=open]:animate-slideDown data-[state=closed]:animate-slideUp">
                    <div className="px-6 pb-4">
                      <div className="space-y-4 pt-2">
                        {unit.sessions.map((session) => (
                          <div
                            key={session.id}
                            className="bg-surface border border-border rounded-lg p-4"
                          >
                            <div className="flex items-start justify-between mb-3">
                              <div className="flex-1">
                                <h5 className="text-foreground mb-1">{session.name}</h5>
                                <div className="flex items-center gap-2">
                                  <span className="text-primary text-sm">
                                    {session.progress}%
                                  </span>
                                  {session.progress === 100 && (
                                    <Badge variant="success" className="text-xs">
                                      Completed
                                    </Badge>
                                  )}
                                </div>
                              </div>
                            </div>

                            {/* Progress Slider */}
                            <div className="space-y-2">
                              <Slider.Root
                                className="relative flex items-center select-none touch-none w-full h-5"
                                value={[session.progress]}
                                onValueChange={(value) =>
                                  handleProgressChange(unit.id, session.id, value)
                                }
                                max={100}
                                step={5}
                              >
                                <Slider.Track className="bg-muted relative grow rounded-full h-2">
                                  <Slider.Range className="absolute bg-primary rounded-full h-full" />
                                </Slider.Track>
                                <Slider.Thumb
                                  className="block w-5 h-5 bg-primary rounded-full hover:bg-primary-hover focus:outline-none focus:ring-2 focus:ring-primary focus:ring-offset-2 focus:ring-offset-background transition-colors cursor-grab active:cursor-grabbing"
                                  aria-label="Progress"
                                />
                              </Slider.Root>
                              <div className="flex justify-between text-xs text-foreground-subtle">
                                <span>0%</span>
                                <span>25%</span>
                                <span>50%</span>
                                <span>75%</span>
                                <span>100%</span>
                              </div>
                            </div>
                          </div>
                        ))}
                      </div>
                    </div>
                  </Accordion.Content>
                </Accordion.Item>
              );
            })}
          </Accordion.Root>
        </div>
      </div>
    </div>
  );
}
