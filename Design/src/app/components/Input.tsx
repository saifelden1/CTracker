import { InputHTMLAttributes, forwardRef } from 'react';
import { cn } from '../../lib/utils';

interface InputProps extends InputHTMLAttributes<HTMLInputElement> {
  error?: boolean;
}

const Input = forwardRef<HTMLInputElement, InputProps>(
  ({ className, error, ...props }, ref) => {
    return (
      <input
        ref={ref}
        className={cn(
          'w-full px-3 py-2 rounded-md',
          'bg-input-background border',
          'text-foreground placeholder:text-foreground-subtle',
          'transition-colors duration-150',
          'focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-offset-background',
          error
            ? 'border-destructive focus:ring-destructive'
            : 'border-input-border focus:ring-input-focus',
          'disabled:opacity-50 disabled:cursor-not-allowed',
          className
        )}
        {...props}
      />
    );
  }
);

Input.displayName = 'Input';

export { Input };
