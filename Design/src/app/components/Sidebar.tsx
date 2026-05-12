import { Home, BookOpen, FolderKanban, BarChart3, Settings, CheckSquare, Timer } from 'lucide-react';

interface NavItem {
  id: string;
  label: string;
  icon: React.ComponentType<{ className?: string }>;
}

const navItems: NavItem[] = [
  { id: 'home', label: 'Home', icon: Home },
  { id: 'courses', label: 'Courses', icon: BookOpen },
  { id: 'projects', label: 'Projects', icon: FolderKanban },
  { id: 'todos', label: 'To-Do', icon: CheckSquare },
  { id: 'pomodoro', label: 'Pomodoro', icon: Timer },
  { id: 'analytics', label: 'Analytics', icon: BarChart3 },
  { id: 'settings', label: 'Settings', icon: Settings },
];

interface SidebarProps {
  activeItem: string;
  onNavigate: (itemId: string) => void;
}

export function Sidebar({ activeItem, onNavigate }: SidebarProps) {
  return (
    <aside className="w-64 h-screen bg-sidebar border-r border-sidebar-border flex flex-col">
      {/* Header */}
      <div className="px-6 py-5 border-b border-sidebar-border">
        <h1 className="flex items-center gap-2">
          <div className="w-8 h-8 rounded-md bg-primary flex items-center justify-center">
            <span className="text-primary-foreground">CT</span>
          </div>
          <span className="text-sidebar-foreground">CTracker</span>
        </h1>
      </div>

      {/* Navigation */}
      <nav className="flex-1 px-3 py-4">
        <ul className="space-y-1">
          {navItems.map((item) => {
            const Icon = item.icon;
            const isActive = activeItem === item.id;

            return (
              <li key={item.id}>
                <button
                  onClick={() => onNavigate(item.id)}
                  className={`
                    w-full flex items-center gap-3 px-3 py-2.5 rounded-md
                    transition-colors duration-150
                    ${
                      isActive
                        ? 'bg-sidebar-active text-sidebar-active-foreground'
                        : 'text-sidebar-foreground hover:bg-sidebar-accent hover:text-sidebar-accent-foreground'
                    }
                  `}
                >
                  <Icon className="w-5 h-5" />
                  <span>{item.label}</span>
                </button>
              </li>
            );
          })}
        </ul>
      </nav>

      {/* Footer */}
      <div className="px-6 py-4 border-t border-sidebar-border">
        <div className="flex items-center gap-3">
          <div className="w-8 h-8 rounded-full bg-muted flex items-center justify-center">
            <span className="text-muted-foreground">U</span>
          </div>
          <div className="flex-1 min-w-0">
            <p className="text-sidebar-foreground truncate">User</p>
            <p className="text-foreground-subtle text-xs truncate">user@example.com</p>
          </div>
        </div>
      </div>
    </aside>
  );
}
