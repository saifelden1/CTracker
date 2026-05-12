import { useState } from 'react';
import { Button } from '../Button';
import { Input } from '../Input';
import { Badge } from '../Badge';
import { Plus, Trash2, Check } from 'lucide-react';

interface TodoItem {
  id: string;
  title: string;
  completed: boolean;
  priority: 'high' | 'medium' | 'low';
}

const sampleTodos: TodoItem[] = [
  { id: '1', title: 'Complete Database Assignment 2', completed: false, priority: 'high' },
  { id: '2', title: 'Study for Algorithms Midterm', completed: false, priority: 'high' },
  { id: '3', title: 'Review lecture notes - Chapter 5', completed: true, priority: 'medium' },
  { id: '4', title: 'Start ML Project proposal', completed: false, priority: 'medium' },
  { id: '5', title: 'Read OS textbook pages 120-150', completed: false, priority: 'low' },
];

export function TodoPage() {
  const [todos, setTodos] = useState<TodoItem[]>(sampleTodos);
  const [newTodo, setNewTodo] = useState('');

  const toggleTodo = (id: string) => {
    setTodos(todos.map(todo =>
      todo.id === id ? { ...todo, completed: !todo.completed } : todo
    ));
  };

  const deleteTodo = (id: string) => {
    setTodos(todos.filter(todo => todo.id !== id));
  };

  const addTodo = () => {
    if (newTodo.trim()) {
      const todo: TodoItem = {
        id: Date.now().toString(),
        title: newTodo,
        completed: false,
        priority: 'medium'
      };
      setTodos([todo, ...todos]);
      setNewTodo('');
    }
  };

  const activeTodos = todos.filter(t => !t.completed);
  const completedTodos = todos.filter(t => t.completed);

  return (
    <div className="h-full overflow-auto">
      <div className="p-8">
        <div className="max-w-4xl mx-auto">
          {/* Header */}
          <div className="mb-8">
            <h2 className="mb-2">To-Do List</h2>
            <p className="text-foreground-muted">
              Manage your tasks and assignments
            </p>
          </div>

          {/* Add new todo */}
          <div className="mb-8 flex gap-3">
            <Input
              placeholder="Add a new task..."
              value={newTodo}
              onChange={(e) => setNewTodo(e.target.value)}
              onKeyDown={(e) => e.key === 'Enter' && addTodo()}
              className="flex-1"
            />
            <Button onClick={addTodo}>
              <Plus className="w-4 h-4" />
              Add
            </Button>
          </div>

          {/* Stats */}
          <div className="grid grid-cols-2 gap-4 mb-8">
            <div className="bg-card border border-card-border rounded-lg p-4">
              <div className="text-2xl text-primary mb-1">{activeTodos.length}</div>
              <div className="text-foreground-muted text-sm">Active tasks</div>
            </div>
            <div className="bg-card border border-card-border rounded-lg p-4">
              <div className="text-2xl text-primary mb-1">{completedTodos.length}</div>
              <div className="text-foreground-muted text-sm">Completed</div>
            </div>
          </div>

          {/* Active Todos */}
          {activeTodos.length > 0 && (
            <div className="mb-8">
              <h3 className="mb-4 text-foreground">Active Tasks</h3>
              <div className="space-y-2">
                {activeTodos.map(todo => (
                  <div
                    key={todo.id}
                    className="bg-card border border-card-border rounded-lg p-4 flex items-center gap-4 hover:border-border-strong transition-colors"
                  >
                    <button
                      onClick={() => toggleTodo(todo.id)}
                      className="w-5 h-5 rounded border-2 border-muted flex items-center justify-center shrink-0 hover:border-primary transition-colors"
                    >
                      {todo.completed && <Check className="w-3 h-3 text-primary" />}
                    </button>
                    <div className="flex-1">
                      <p className="text-foreground">{todo.title}</p>
                    </div>
                    <Badge
                      variant={
                        todo.priority === 'high' ? 'error' :
                        todo.priority === 'medium' ? 'warning' : 'default'
                      }
                    >
                      {todo.priority}
                    </Badge>
                    <button
                      onClick={() => deleteTodo(todo.id)}
                      className="p-1 hover:bg-surface rounded transition-colors"
                    >
                      <Trash2 className="w-4 h-4 text-foreground-muted hover:text-destructive" />
                    </button>
                  </div>
                ))}
              </div>
            </div>
          )}

          {/* Completed Todos */}
          {completedTodos.length > 0 && (
            <div>
              <h3 className="mb-4 text-foreground-muted">Completed</h3>
              <div className="space-y-2">
                {completedTodos.map(todo => (
                  <div
                    key={todo.id}
                    className="bg-surface border border-border rounded-lg p-4 flex items-center gap-4 opacity-60"
                  >
                    <button
                      onClick={() => toggleTodo(todo.id)}
                      className="w-5 h-5 rounded border-2 border-primary flex items-center justify-center shrink-0 bg-primary"
                    >
                      <Check className="w-3 h-3 text-primary-foreground" />
                    </button>
                    <div className="flex-1">
                      <p className="text-foreground line-through">{todo.title}</p>
                    </div>
                    <button
                      onClick={() => deleteTodo(todo.id)}
                      className="p-1 hover:bg-muted rounded transition-colors"
                    >
                      <Trash2 className="w-4 h-4 text-foreground-muted hover:text-destructive" />
                    </button>
                  </div>
                ))}
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
