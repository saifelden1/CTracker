import { useState } from 'react';
import { Button } from '../Button';
import { Input } from '../Input';
import { Badge } from '../Badge';
import { EntityCard } from '../EntityCard';
import { EmptyState } from '../EmptyState';
import { CourseDetailPage } from './CourseDetailPage';
import { Search, Plus, Filter } from 'lucide-react';

interface Category {
  id: string;
  name: string;
  color: string;
}

interface Entity {
  id: string;
  name: string;
  type: 'course' | 'project';
  progress: number;
  status: 'active' | 'paused';
  categoryId?: string;
}

// Categories
const categories: Category[] = [
  { id: 'cat1', name: 'Algorithms', color: '#10b981' },
  { id: 'cat2', name: 'Web Development', color: '#3b82f6' },
  { id: 'cat3', name: 'Machine Learning', color: '#8b5cf6' },
  { id: 'cat4', name: 'Systems', color: '#f59e0b' },
  { id: 'cat5', name: 'Security', color: '#ec4899' },
];

// Sample data
const sampleEntities: Entity[] = [
  { id: '1', name: 'Advanced Algorithms & Data Structures', type: 'course', progress: 75, status: 'active', categoryId: 'cat1' },
  { id: '2', name: 'Web Development Project', type: 'project', progress: 42, status: 'active', categoryId: 'cat2' },
  { id: '3', name: 'Database Systems', type: 'course', progress: 88, status: 'active', categoryId: 'cat4' },
  { id: '4', name: 'Machine Learning Fundamentals', type: 'course', progress: 34, status: 'active', categoryId: 'cat3' },
  { id: '5', name: 'E-Commerce Platform', type: 'project', progress: 67, status: 'active', categoryId: 'cat2' },
  { id: '6', name: 'Operating Systems', type: 'course', progress: 91, status: 'active', categoryId: 'cat4' },
  { id: '7', name: 'Mobile App Development', type: 'project', progress: 23, status: 'paused', categoryId: 'cat2' },
  { id: '8', name: 'Computer Networks', type: 'course', progress: 56, status: 'active', categoryId: 'cat4' },
  { id: '9', name: 'API Integration Project', type: 'project', progress: 78, status: 'active', categoryId: 'cat2' },
  { id: '10', name: 'Software Engineering Principles', type: 'course', progress: 45, status: 'paused', categoryId: 'cat1' },
  { id: '11', name: 'Cloud Infrastructure Setup', type: 'project', progress: 89, status: 'active', categoryId: 'cat4' },
  { id: '12', name: 'Cybersecurity Fundamentals', type: 'course', progress: 62, status: 'active', categoryId: 'cat5' },
];

export function CoursesPage() {
  const [selectedId, setSelectedId] = useState<string | null>(null);
  const [viewingCourseId, setViewingCourseId] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [entities] = useState<Entity[]>(sampleEntities);
  const [selectedCategory, setSelectedCategory] = useState<string>('all');
  const [selectedStatus, setSelectedStatus] = useState<string>('all');
  const [showFilters, setShowFilters] = useState(false);

  // Filter entities based on search, category, and status
  const filteredEntities = entities.filter((entity) => {
    const matchesSearch = entity.name.toLowerCase().includes(searchQuery.toLowerCase());
    const matchesCategory = selectedCategory === 'all' || entity.categoryId === selectedCategory;
    const matchesStatus = selectedStatus === 'all' || entity.status === selectedStatus;

    return matchesSearch && matchesCategory && matchesStatus;
  });

  const hasEntities = entities.length > 0;
  const hasFilteredResults = filteredEntities.length > 0;

  const getCategoryById = (id?: string) => {
    return categories.find(cat => cat.id === id);
  };

  // If viewing a course, show detail page
  if (viewingCourseId) {
    return <CourseDetailPage courseId={viewingCourseId} onBack={() => setViewingCourseId(null)} />;
  }

  return (
    <div className="h-full flex flex-col">
      {/* Header - Fixed */}
      <div className="border-b border-border bg-background px-8 py-6">
        <div className="max-w-7xl mx-auto">
          <div className="flex items-center justify-between mb-6">
            <div>
              <h2 className="mb-2">Courses</h2>
              <p className="text-foreground-muted">
                {hasEntities
                  ? `${entities.length} courses and projects`
                  : 'Start tracking your courses and projects'}
              </p>
            </div>
            <div className="flex items-center gap-3">
              <Button
                variant={showFilters ? 'primary' : 'secondary'}
                size="md"
                onClick={() => setShowFilters(!showFilters)}
              >
                <Filter className="w-4 h-4" />
                Filter
              </Button>
              <Button variant="primary" size="md">
                <Plus className="w-4 h-4" />
                Add New
              </Button>
            </div>
          </div>

          {/* Search Bar */}
          {hasEntities && (
            <div className="max-w-md relative">
              <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-foreground-subtle" />
              <Input
                placeholder="Search courses and projects..."
                className="pl-10"
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
              />
            </div>
          )}

          {/* Filter Panel */}
          {showFilters && hasEntities && (
            <div className="mt-4 p-4 bg-card border border-card-border rounded-lg">
              <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                {/* Category Filter */}
                <div>
                  <label className="block text-foreground text-sm mb-2">Category</label>
                  <select
                    value={selectedCategory}
                    onChange={(e) => setSelectedCategory(e.target.value)}
                    className="w-full px-3 py-2 bg-input-background border border-input-border rounded-md text-foreground"
                  >
                    <option value="all">All Categories</option>
                    {categories.map((category) => (
                      <option key={category.id} value={category.id}>
                        {category.name}
                      </option>
                    ))}
                  </select>
                </div>

                {/* Status Filter */}
                <div>
                  <label className="block text-foreground text-sm mb-2">Status</label>
                  <select
                    value={selectedStatus}
                    onChange={(e) => setSelectedStatus(e.target.value)}
                    className="w-full px-3 py-2 bg-input-background border border-input-border rounded-md text-foreground"
                  >
                    <option value="all">All Status</option>
                    <option value="active">Active</option>
                    <option value="paused">Paused</option>
                  </select>
                </div>
              </div>

              {/* Active Filters Display */}
              {(selectedCategory !== 'all' || selectedStatus !== 'all') && (
                <div className="mt-4 flex items-center gap-2">
                  <span className="text-foreground-muted text-sm">Active filters:</span>
                  {selectedCategory !== 'all' && (
                    <Badge
                      variant="info"
                      className="cursor-pointer"
                      onClick={() => setSelectedCategory('all')}
                    >
                      {getCategoryById(selectedCategory)?.name} ×
                    </Badge>
                  )}
                  {selectedStatus !== 'all' && (
                    <Badge
                      variant="default"
                      className="cursor-pointer"
                      onClick={() => setSelectedStatus('all')}
                    >
                      {selectedStatus} ×
                    </Badge>
                  )}
                  <button
                    onClick={() => {
                      setSelectedCategory('all');
                      setSelectedStatus('all');
                    }}
                    className="text-primary text-sm hover:text-primary-hover ml-2"
                  >
                    Clear all
                  </button>
                </div>
              )}
            </div>
          )}
        </div>
      </div>

      {/* Content - Scrollable */}
      <div className="flex-1 overflow-auto px-8 py-6">
        <div className="max-w-7xl mx-auto">
          {!hasEntities ? (
            // Empty state - no entities at all
            <EmptyState
              title="No courses or projects yet"
              description="Get started by adding your first course or project to begin tracking your progress."
              actionLabel="Add Your First Item"
              onAction={() => console.log('Add new clicked')}
            />
          ) : !hasFilteredResults ? (
            // Empty state - search returned no results
            <EmptyState
              title="No results found"
              description={`No courses or projects match "${searchQuery}". Try a different search term.`}
            />
          ) : (
            // Grid of entity cards
            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-6">
              {filteredEntities.map((entity) => {
                const category = getCategoryById(entity.categoryId);
                return (
                  <EntityCard
                    key={entity.id}
                    name={entity.name}
                    type={entity.type}
                    progress={entity.progress}
                    isSelected={selectedId === entity.id}
                    categoryName={category?.name}
                    categoryColor={category?.color}
                    onClick={() => {
                      setSelectedId(entity.id);
                      setViewingCourseId(entity.id);
                    }}
                  />
                );
              })}
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
