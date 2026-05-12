import { HTMLAttributes, forwardRef } from 'react';
import { cn } from '../../lib/utils';

interface BadgeProps extends HTMLAttributes<HTMLSpanElement> {
  variant?: 'success' | 'warning' | 'error' | 'info' | 'default';
}

const Badge = forwardRef<HTMLSpanElement, BadgeProps>(
  ({ className, variant = 'default', children, ...props }, ref) => {
    return (
      <span
        ref={ref}
        className={cn(
          'inline-flex items-center px-2.5 py-0.5 rounded-md text-xs',
          {
            'bg-badge-success-bg text-badge-success': variant === 'success',
            'bg-badge-warning-bg text-badge-warning': variant === 'warning',
            'bg-badge-error-bg text-badge-error': variant === 'error',
            'bg-badge-info-bg text-badge-info': variant === 'info',
            'bg-muted text-muted-foreground': variant === 'default',
          },
          className
        )}
        {...props}
      >
        {children}
      </span>
    );
  }
);

Badge.displayName = 'Badge';

export { Badge };
