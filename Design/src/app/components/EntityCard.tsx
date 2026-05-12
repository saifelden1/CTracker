import { CircularProgress } from './CircularProgress';
import { Badge } from './Badge';
import { cn } from '../../lib/utils';

interface EntityCardProps {
  name: string;
  type: 'course' | 'project';
  progress: number;
  isSelected?: boolean;
  onClick?: () => void;
  categoryName?: string;
  categoryColor?: string;
}

export function EntityCard({
  name,
  type,
  progress,
  isSelected = false,
  onClick,
  categoryName,
  categoryColor
}: EntityCardProps) {
  return (
    <button
      onClick={onClick}
      className={cn(
        'w-full bg-card border rounded-lg p-6',
        'transition-all duration-150',
        'text-left cursor-pointer',
        'flex flex-col h-full min-h-[180px]',
        'hover:border-border-strong hover:bg-surface hover:shadow-md',
        'active:scale-[0.98]',
        isSelected
          ? 'border-primary bg-surface ring-2 ring-primary ring-offset-2 ring-offset-background shadow-lg'
          : 'border-card-border'
      )}
    >
      {/* Header with badges */}
      <div className="flex items-start justify-between mb-3">
        <h4 className="text-card-foreground pr-2 line-clamp-2 flex-1">
          {name}
        </h4>
        <Badge variant={type === 'course' ? 'info' : 'success'} className="shrink-0 mt-0.5">
          {type === 'course' ? 'Course' : 'Project'}
        </Badge>
      </div>

      {/* Category Badge */}
      {categoryName && (
        <div className="mb-4">
          <span
            className="inline-flex items-center gap-1.5 px-2 py-1 rounded text-xs"
            style={{
              backgroundColor: `${categoryColor}20`,
              color: categoryColor
            }}
          >
            <span
              className="w-2 h-2 rounded-full"
              style={{ backgroundColor: categoryColor }}
            />
            {categoryName}
          </span>
        </div>
      )}

      {/* Progress section */}
      <div className="mt-auto flex items-center gap-4">
        <CircularProgress percentage={progress} size={56} strokeWidth={5} />
        <div className="flex-1 min-w-0">
          <div className="text-foreground-muted text-sm mb-1">Progress</div>
          <div className="text-primary text-xl">{progress}%</div>
        </div>
      </div>
    </button>
  );
}
