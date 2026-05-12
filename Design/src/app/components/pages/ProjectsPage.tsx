import { useState } from 'react';
import { Button } from '../Button';
import { Input } from '../Input';
import { Badge } from '../Badge';
import { Card, CardHeader, CardTitle, CardContent } from '../Card';
import { Search, Plus, Filter, Calendar, ExternalLink, Users, CheckCircle2, Circle } from 'lucide-react';
import { CircularProgress } from '../CircularProgress';

interface Task {
  id: string;
  title: string;
  completed: boolean;
}

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

const sampleProjects: Project[] = [
  {
    id: '1',
    name: 'E-Commerce Platform',
    description: 'Full-stack web application with React, Node.js, and PostgreSQL',
    progress: 67,
    status: 'active',
    priority: 'high',
    deadline: new Date(2026, 4, 25),
    team: ['Alice', 'Bob'],
    links: [
      { label: 'GitHub', url: 'https://github.com/...' },
      { label: 'Figma', url: 'https://figma.com/...' },
    ],
    tasks: [
      { id: 't1', title: 'Setup backend API', completed: true },
      { id: 't2', title: 'Design database schema', completed: true },
      { id: 't3', title: 'Build product catalog', completed: false },
      { id: 't4', title: 'Implement checkout flow', completed: false },
      { id: 't5', title: 'Add payment integration', completed: false },
    ],
  },
  {
    id: '2',
    name: 'Mobile App Development',
    description: 'React Native fitness tracking app with social features',
    progress: 23,
    status: 'active',
    priority: 'medium',
    deadline: new Date(2026, 5, 15),
    tasks: [
      { id: 't6', title: 'Project setup and configuration', completed: true },
      { id: 't7', title: 'UI/UX design', completed: true },
      { id: 't8', title: 'Authentication system', completed: false },
      { id: 't9', title: 'Activity tracking features', completed: false },
    ],
  },
  {
    id: '3',
    name: 'API Integration Project',
    description: 'Integrate third-party APIs for data aggregation dashboard',
    progress: 78,
    status: 'active',
    priority: 'high',
    deadline: new Date(2026, 4, 18),
    team: ['Charlie'],
    links: [{ label: 'API Docs', url: 'https://docs.api.com' }],
    tasks: [
      { id: 't10', title: 'Research APIs', completed: true },
      { id: 't11', title: 'Setup authentication', completed: true },
      { id: 't12', title: 'Build data aggregation service', completed: true },
      { id: 't13', title: 'Create dashboard UI', completed: false },
      { id: 't14', title: 'Testing and optimization', completed: false },
    ],
  },
  {
    id: '4',
    name: 'Cloud Infrastructure Setup',
    description: 'AWS deployment pipeline with CI/CD automation',
    progress: 89,
    status: 'active',
    priority: 'medium',
    deadline: new Date(2026, 4, 20),
    tasks: [
      { id: 't15', title: 'Setup AWS account', completed: true },
      { id: 't16', title: 'Configure EC2 instances', completed: true },
      { id: 't17', title: 'Setup CI/CD pipeline', completed: true },
      { id: 't18', title: 'Documentation', completed: false },
    ],
  },
];

export function ProjectsPage() {
  const [selectedId, setSelectedId] = useState<string | null>(null);
  const [viewingProjectId, setViewingProjectId] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [projects, setProjects] = useState<Project[]>(sampleProjects);

  const filteredProjects = projects.filter((project) =>
    project.name.toLowerCase().includes(searchQuery.toLowerCase()) ||
    project.description.toLowerCase().includes(searchQuery.toLowerCase())
  );

  const toggleTask = (projectId: string, taskId: string) => {
    setProjects(
      projects.map((project) =>
        project.id === projectId
          ? {
              ...project,
              tasks: project.tasks.map((task) =>
                task.id === taskId ? { ...task, completed: !task.completed } : task
              ),
            }
          : project
      )
    );
  };

  const getDaysUntilDeadline = (deadline: Date) => {
    const today = new Date(2026, 4, 12);
    const diffTime = deadline.getTime() - today.getTime();
    const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24));
    return diffDays;
  };

  const getDeadlineBadge = (deadline: Date) => {
    const days = getDaysUntilDeadline(deadline);
    if (days < 0) return { variant: 'error' as const, text: 'Overdue' };
    if (days <= 3) return { variant: 'error' as const, text: `${days}d left` };
    if (days <= 7) return { variant: 'warning' as const, text: `${days}d left` };
    return { variant: 'default' as const, text: `${days}d left` };
  };

  // If viewing a project detail
  if (viewingProjectId) {
    const project = projects.find((p) => p.id === viewingProjectId);
    if (!project) return null;

    const completedTasks = project.tasks.filter((t) => t.completed).length;

    return (
      <div className="h-full overflow-auto">
        <div className="p-8">
          <div className="max-w-5xl mx-auto">
            <Button variant="ghost" onClick={() => setViewingProjectId(null)} className="mb-4 -ml-3">
              ← Back to Projects
            </Button>

            {/* Project Header */}
            <div className="mb-8">
              <div className="flex items-start justify-between mb-4">
                <div className="flex-1">
                  <h2 className="mb-2">{project.name}</h2>
                  <p className="text-foreground-muted mb-4">{project.description}</p>
                  <div className="flex items-center gap-3 flex-wrap">
                    <Badge variant={project.status === 'active' ? 'success' : 'default'}>
                      {project.status}
                    </Badge>
                    <Badge
                      variant={
                        project.priority === 'high'
                          ? 'error'
                          : project.priority === 'medium'
                          ? 'warning'
                          : 'default'
                      }
                    >
                      {project.priority} priority
                    </Badge>
                    <Badge {...getDeadlineBadge(project.deadline)}>
                      <Calendar className="w-3 h-3 mr-1" />
                      {getDeadlineBadge(project.deadline).text}
                    </Badge>
                  </div>
                </div>
              </div>

              {/* Progress Card */}
              <Card>
                <CardContent className="p-6">
                  <div className="flex items-center gap-6">
                    <CircularProgress percentage={project.progress} size={80} strokeWidth={8} />
                    <div className="flex-1">
                      <h3 className="mb-2">Overall Progress</h3>
                      <div className="text-foreground-muted text-sm mb-2">
                        {completedTasks} of {project.tasks.length} tasks completed
                      </div>
                      <div className="w-full h-2 bg-muted rounded-full overflow-hidden">
                        <div
                          className="h-full bg-primary transition-all duration-300"
                          style={{ width: `${project.progress}%` }}
                        />
                      </div>
                    </div>
                  </div>
                </CardContent>
              </Card>
            </div>

            <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
              {/* Main Content */}
              <div className="lg:col-span-2 space-y-6">
                {/* Tasks */}
                <Card>
                  <CardHeader>
                    <CardTitle>Tasks Checklist</CardTitle>
                  </CardHeader>
                  <CardContent>
                    <div className="space-y-2">
                      {project.tasks.map((task) => (
                        <button
                          key={task.id}
                          onClick={() => toggleTask(project.id, task.id)}
                          className="w-full flex items-center gap-3 p-3 rounded-lg hover:bg-surface transition-colors text-left"
                        >
                          <div
                            className={`w-5 h-5 rounded border-2 flex items-center justify-center shrink-0 ${
                              task.completed
                                ? 'bg-primary border-primary'
                                : 'border-muted'
                            }`}
                          >
                            {task.completed && <CheckCircle2 className="w-3 h-3 text-primary-foreground" />}
                          </div>
                          <span
                            className={`flex-1 ${
                              task.completed
                                ? 'text-foreground-muted line-through'
                                : 'text-foreground'
                            }`}
                          >
                            {task.title}
                          </span>
                        </button>
                      ))}
                    </div>
                  </CardContent>
                </Card>
              </div>

              {/* Sidebar */}
              <div className="space-y-6">
                {/* Project Info */}
                <Card>
                  <CardHeader>
                    <CardTitle>Project Info</CardTitle>
                  </CardHeader>
                  <CardContent>
                    <div className="space-y-4">
                      <div>
                        <div className="text-foreground-muted text-sm mb-1">Deadline</div>
                        <div className="text-foreground">
                          {project.deadline.toLocaleDateString('en-US', {
                            month: 'long',
                            day: 'numeric',
                            year: 'numeric',
                          })}
                        </div>
                      </div>

                      {project.team && project.team.length > 0 && (
                        <div>
                          <div className="text-foreground-muted text-sm mb-2 flex items-center gap-2">
                            <Users className="w-4 h-4" />
                            Team Members
                          </div>
                          <div className="flex flex-wrap gap-2">
                            {project.team.map((member, index) => (
                              <Badge key={index} variant="default">
                                {member}
                              </Badge>
                            ))}
                          </div>
                        </div>
                      )}

                      {project.links && project.links.length > 0 && (
                        <div>
                          <div className="text-foreground-muted text-sm mb-2">Links</div>
                          <div className="space-y-2">
                            {project.links.map((link, index) => (
                              <a
                                key={index}
                                href={link.url}
                                target="_blank"
                                rel="noopener noreferrer"
                                className="flex items-center gap-2 text-primary hover:text-primary-hover text-sm transition-colors"
                              >
                                <ExternalLink className="w-4 h-4" />
                                {link.label}
                              </a>
                            ))}
                          </div>
                        </div>
                      )}
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

  // Project Grid View
  return (
    <div className="h-full flex flex-col">
      {/* Header */}
      <div className="border-b border-border bg-background px-8 py-6">
        <div className="max-w-7xl mx-auto">
          <div className="flex items-center justify-between mb-6">
            <div>
              <h2 className="mb-2">Projects</h2>
              <p className="text-foreground-muted">
                {projects.length} active projects
              </p>
            </div>
            <div className="flex items-center gap-3">
              <Button variant="secondary" size="md">
                <Filter className="w-4 h-4" />
                Filter
              </Button>
              <Button variant="primary" size="md">
                <Plus className="w-4 h-4" />
                New Project
              </Button>
            </div>
          </div>

          <div className="max-w-md relative">
            <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-foreground-subtle" />
            <Input
              placeholder="Search projects..."
              className="pl-10"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
          </div>
        </div>
      </div>

      {/* Projects Grid */}
      <div className="flex-1 overflow-auto px-8 py-6">
        <div className="max-w-7xl mx-auto">
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
            {filteredProjects.map((project) => {
              const completedTasks = project.tasks.filter((t) => t.completed).length;
              const deadlineBadge = getDeadlineBadge(project.deadline);

              return (
                <button
                  key={project.id}
                  onClick={() => {
                    setSelectedId(project.id);
                    setViewingProjectId(project.id);
                  }}
                  className={`bg-card border rounded-lg p-6 text-left cursor-pointer transition-all duration-150 hover:border-border-strong hover:bg-surface hover:shadow-md ${
                    selectedId === project.id
                      ? 'border-primary ring-2 ring-primary ring-offset-2 ring-offset-background shadow-lg'
                      : 'border-card-border'
                  }`}
                >
                  {/* Header */}
                  <div className="mb-4">
                    <h4 className="text-card-foreground mb-2">{project.name}</h4>
                    <p className="text-foreground-muted text-sm line-clamp-2 mb-3">
                      {project.description}
                    </p>
                    <div className="flex items-center gap-2 flex-wrap">
                      <Badge
                        variant={
                          project.priority === 'high'
                            ? 'error'
                            : project.priority === 'medium'
                            ? 'warning'
                            : 'default'
                        }
                      >
                        {project.priority}
                      </Badge>
                      <Badge {...deadlineBadge}>{deadlineBadge.text}</Badge>
                    </div>
                  </div>

                  {/* Progress */}
                  <div className="mb-4">
                    <div className="flex items-center justify-between mb-2">
                      <span className="text-foreground-muted text-sm">Progress</span>
                      <span className="text-primary text-sm">{project.progress}%</span>
                    </div>
                    <div className="w-full h-2 bg-muted rounded-full overflow-hidden">
                      <div
                        className="h-full bg-primary transition-all duration-300"
                        style={{ width: `${project.progress}%` }}
                      />
                    </div>
                  </div>

                  {/* Tasks Count */}
                  <div className="flex items-center justify-between text-sm">
                    <span className="text-foreground-muted">
                      {completedTasks}/{project.tasks.length} tasks
                    </span>
                    {project.team && (
                      <div className="flex items-center gap-1 text-foreground-muted">
                        <Users className="w-3 h-3" />
                        {project.team.length}
                      </div>
                    )}
                  </div>
                </button>
              );
            })}
          </div>
        </div>
      </div>
    </div>
  );
}
