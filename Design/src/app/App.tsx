import { useState } from 'react';
import { Sidebar } from './components/Sidebar';
import { HomePage } from './components/pages/HomePage';
import { CoursesPage } from './components/pages/CoursesPage';
import { ProjectsPage } from './components/pages/ProjectsPage';
import { TodoPage } from './components/pages/TodoPage';
import { PomodoroPage } from './components/pages/PomodoroPage';
import { AnalyticsPage } from './components/pages/AnalyticsPage';
import { SettingsPage } from './components/pages/SettingsPage';

export default function App() {
  const [activePage, setActivePage] = useState('home');

  const renderPage = () => {
    switch (activePage) {
      case 'home':
        return <HomePage />;
      case 'courses':
        return <CoursesPage />;
      case 'projects':
        return <ProjectsPage />;
      case 'todos':
        return <TodoPage />;
      case 'pomodoro':
        return <PomodoroPage />;
      case 'analytics':
        return <AnalyticsPage />;
      case 'settings':
        return <SettingsPage />;
      default:
        return <HomePage />;
    }
  };

  return (
    <div className="size-full flex bg-background">
      {/* Sidebar Navigation */}
      <Sidebar activeItem={activePage} onNavigate={setActivePage} />

      {/* Main Content Area */}
      <main className="flex-1 overflow-hidden">{renderPage()}</main>
    </div>
  );
}