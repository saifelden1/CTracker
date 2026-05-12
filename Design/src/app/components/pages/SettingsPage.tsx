import { useState } from 'react';
import { Card, CardHeader, CardTitle, CardContent } from '../Card';
import { Button } from '../Button';
import { Input } from '../Input';
import { Badge } from '../Badge';
import { User, Bell, Download, Upload, Trash2, Tag } from 'lucide-react';

export function SettingsPage() {
  const [name, setName] = useState('John Doe');
  const [email, setEmail] = useState('john.doe@example.com');
  const [workDuration, setWorkDuration] = useState(25);
  const [breakDuration, setBreakDuration] = useState(5);
  const [notifications, setNotifications] = useState(true);
  const [soundEffects, setSoundEffects] = useState(true);
  const [autoPauseDays, setAutoPauseDays] = useState(14);

  const categories = [
    { id: '1', name: 'Algorithms', color: '#10b981', count: 2 },
    { id: '2', name: 'Web Development', color: '#3b82f6', count: 3 },
    { id: '3', name: 'Machine Learning', color: '#8b5cf6', count: 1 },
    { id: '4', name: 'Systems', color: '#f59e0b', count: 4 },
  ];

  return (
    <div className="h-full overflow-auto">
      <div className="p-8">
        <div className="max-w-4xl mx-auto">
          {/* Header */}
          <div className="mb-8">
            <h2 className="mb-2">Settings</h2>
            <p className="text-foreground-muted">
              Manage your preferences and app configuration
            </p>
          </div>

          <div className="space-y-6">
            {/* Profile Section */}
            <Card>
              <CardHeader>
                <div className="flex items-center gap-2">
                  <User className="w-5 h-5 text-foreground-muted" />
                  <CardTitle>Profile</CardTitle>
                </div>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div>
                    <label className="block text-foreground mb-2">Name</label>
                    <Input
                      value={name}
                      onChange={(e) => setName(e.target.value)}
                      placeholder="Your name"
                    />
                  </div>
                  <div>
                    <label className="block text-foreground mb-2">Email</label>
                    <Input
                      type="email"
                      value={email}
                      onChange={(e) => setEmail(e.target.value)}
                      placeholder="your.email@example.com"
                    />
                  </div>
                  <div>
                    <label className="block text-foreground mb-2">Study Goals</label>
                    <textarea
                      className="w-full px-3 py-2 bg-input-background border border-input-border rounded-md text-foreground min-h-[100px] focus:outline-none focus:ring-2 focus:ring-primary focus:ring-offset-2 focus:ring-offset-background"
                      placeholder="What are your learning goals?"
                      defaultValue="Complete all advanced algorithms courses and build 3 projects by end of semester."
                    />
                  </div>
                  <Button>Save Profile</Button>
                </div>
              </CardContent>
            </Card>

            {/* Preferences Section */}
            <Card>
              <CardHeader>
                <div className="flex items-center gap-2">
                  <Bell className="w-5 h-5 text-foreground-muted" />
                  <CardTitle>Preferences</CardTitle>
                </div>
              </CardHeader>
              <CardContent>
                <div className="space-y-6">
                  {/* Pomodoro Settings */}
                  <div>
                    <h4 className="text-foreground mb-4">Pomodoro Timer</h4>
                    <div className="grid grid-cols-2 gap-4">
                      <div>
                        <label className="block text-foreground-muted text-sm mb-2">
                          Default Work Duration
                        </label>
                        <select
                          value={workDuration}
                          onChange={(e) => setWorkDuration(Number(e.target.value))}
                          className="w-full px-3 py-2 bg-input-background border border-input-border rounded-md text-foreground"
                        >
                          <option value={15}>15 minutes</option>
                          <option value={20}>20 minutes</option>
                          <option value={25}>25 minutes</option>
                          <option value={30}>30 minutes</option>
                          <option value={45}>45 minutes</option>
                          <option value={50}>50 minutes</option>
                        </select>
                      </div>
                      <div>
                        <label className="block text-foreground-muted text-sm mb-2">
                          Default Break Duration
                        </label>
                        <select
                          value={breakDuration}
                          onChange={(e) => setBreakDuration(Number(e.target.value))}
                          className="w-full px-3 py-2 bg-input-background border border-input-border rounded-md text-foreground"
                        >
                          <option value={5}>5 minutes</option>
                          <option value={10}>10 minutes</option>
                          <option value={15}>15 minutes</option>
                        </select>
                      </div>
                    </div>
                  </div>

                  {/* Notifications */}
                  <div>
                    <h4 className="text-foreground mb-4">Notifications & Sounds</h4>
                    <div className="space-y-3">
                      <label className="flex items-center justify-between">
                        <span className="text-foreground">Enable Notifications</span>
                        <input
                          type="checkbox"
                          checked={notifications}
                          onChange={(e) => setNotifications(e.target.checked)}
                          className="w-5 h-5 rounded border-input-border bg-input-background text-primary focus:ring-primary"
                        />
                      </label>
                      <label className="flex items-center justify-between">
                        <span className="text-foreground">Sound Effects</span>
                        <input
                          type="checkbox"
                          checked={soundEffects}
                          onChange={(e) => setSoundEffects(e.target.checked)}
                          className="w-5 h-5 rounded border-input-border bg-input-background text-primary focus:ring-primary"
                        />
                      </label>
                    </div>
                  </div>

                  {/* Auto-pause */}
                  <div>
                    <h4 className="text-foreground mb-4">Course Management</h4>
                    <div>
                      <label className="block text-foreground-muted text-sm mb-2">
                        Auto-pause inactive courses after
                      </label>
                      <select
                        value={autoPauseDays}
                        onChange={(e) => setAutoPauseDays(Number(e.target.value))}
                        className="w-full px-3 py-2 bg-input-background border border-input-border rounded-md text-foreground"
                      >
                        <option value={7}>7 days</option>
                        <option value={14}>14 days</option>
                        <option value={30}>30 days</option>
                        <option value={0}>Never</option>
                      </select>
                    </div>
                  </div>

                  <Button>Save Preferences</Button>
                </div>
              </CardContent>
            </Card>

            {/* Categories Section */}
            <Card>
              <CardHeader>
                <div className="flex items-center gap-2">
                  <Tag className="w-5 h-5 text-foreground-muted" />
                  <CardTitle>Course Categories</CardTitle>
                </div>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div className="space-y-2">
                    {categories.map((category) => (
                      <div
                        key={category.id}
                        className="flex items-center justify-between p-3 bg-surface border border-border rounded-lg"
                      >
                        <div className="flex items-center gap-3">
                          <div
                            className="w-4 h-4 rounded"
                            style={{ backgroundColor: category.color }}
                          />
                          <span className="text-foreground">{category.name}</span>
                          <Badge variant="default">{category.count} courses</Badge>
                        </div>
                        <Button variant="ghost" size="sm">
                          Edit
                        </Button>
                      </div>
                    ))}
                  </div>
                  <Button variant="secondary">
                    <Tag className="w-4 h-4" />
                    Add Category
                  </Button>
                </div>
              </CardContent>
            </Card>

            {/* Data Management Section */}
            <Card>
              <CardHeader>
                <div className="flex items-center gap-2">
                  <Download className="w-5 h-5 text-foreground-muted" />
                  <CardTitle>Data Management</CardTitle>
                </div>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  <div className="flex gap-3">
                    <Button variant="secondary">
                      <Download className="w-4 h-4" />
                      Export Data
                    </Button>
                    <Button variant="secondary">
                      <Upload className="w-4 h-4" />
                      Import Data
                    </Button>
                  </div>
                  <div className="pt-4 border-t border-border">
                    <p className="text-foreground-muted text-sm mb-3">
                      Danger Zone: This action cannot be undone
                    </p>
                    <Button variant="destructive">
                      <Trash2 className="w-4 h-4" />
                      Clear All Data
                    </Button>
                  </div>
                </div>
              </CardContent>
            </Card>

            {/* About Section */}
            <Card>
              <CardHeader>
                <CardTitle>About</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="space-y-3 text-sm">
                  <div className="flex justify-between">
                    <span className="text-foreground-muted">Version</span>
                    <span className="text-foreground">1.0.0</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-foreground-muted">Built with</span>
                    <span className="text-foreground">React + Tailwind CSS</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-foreground-muted">License</span>
                    <span className="text-foreground">MIT</span>
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>
        </div>
      </div>
    </div>
  );
}
