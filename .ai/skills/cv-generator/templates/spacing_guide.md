# Adaptive Spacing Guide for CV Template

This guide helps Kiro adjust spacing dynamically based on CV content length.

## Spacing Parameters

### 1. Top Margin (`geometry` package)
```latex
\usepackage[margin=0.65in, top=VALUE, bottom=0.4in]{geometry}
```

**Adjustment Rules:**
- **SHORT CV** (< 0.8 pages): `top=0.35in` (default)
- **MEDIUM CV** (0.8-1.0 pages): `top=0.30in`
- **LONG CV** (> 1.0 pages): `top=0.25in`

### 2. List Spacing (`enumitem` package)
```latex
\setlist[itemize]{noitemsep, topsep=VALUE, parsep=VALUE, partopsep=0pt}
```

**Adjustment Rules:**
- **SHORT CV**: `topsep=0pt, parsep=0pt` (default)
- **MEDIUM CV**: `topsep=-1pt, parsep=0pt`
- **LONG CV**: `topsep=-2pt, parsep=-1pt`

### 3. Section Spacing (`titlesec` package)
```latex
\titlespacing*{\section}{0pt}{BEFORE}{AFTER}
```

**Adjustment Rules:**
- **SHORT CV**: `{0pt}{0.2\baselineskip}{0.1\baselineskip}` (default)
- **MEDIUM CV**: `{0pt}{0.15\baselineskip}{0.08\baselineskip}`
- **LONG CV**: `{0pt}{0.12\baselineskip}{0.05\baselineskip}`

### 4. Header Spacing
```latex
\vspace{VALUE}
```

**Adjustment Rules:**
- **SHORT CV**: `\vspace{-5pt}` (default, after header)
- **MEDIUM CV**: `\vspace{-8pt}`
- **LONG CV**: `\vspace{-10pt}`

## Content Length Estimation

**How to estimate CV length:**

1. Count total bullet points across all sections
2. Count number of sections included
3. Estimate page usage:
   - **SHORT**: < 15 bullet points, < 5 sections
   - **MEDIUM**: 15-25 bullet points, 5-7 sections
   - **LONG**: > 25 bullet points, > 7 sections

## Decision Tree for Kiro

```
IF total_bullets < 15 AND sections < 5:
    Use SHORT spacing (default values)
    
ELIF total_bullets <= 25 AND sections <= 7:
    Use MEDIUM spacing
    - top=0.30in
    - topsep=-1pt
    - section spacing: 0.15/0.08
    - header vspace: -8pt
    
ELSE:
    Use LONG spacing
    - top=0.25in
    - topsep=-2pt, parsep=-1pt
    - section spacing: 0.12/0.05
    - header vspace: -10pt
```

## Additional Compression Techniques

If CV still doesn't fit on desired pages:

1. **Reduce font size**: Change `10pt` to `9pt` in `\documentclass`
2. **Compress bullet points**: Combine related points
3. **Remove optional sections**: Volunteering, Courses (if not critical)
4. **Shorten descriptions**: Keep only most impactful bullets
5. **Use abbreviations**: "Bachelor of Science" → "B.Sc."

## Example Adjustments

### Medium CV Example
```latex
\usepackage[margin=0.65in, top=0.30in, bottom=0.4in]{geometry}
\setlist[itemize]{noitemsep, topsep=-1pt, parsep=0pt, partopsep=0pt}
\titlespacing*{\section}{0pt}{0.15\baselineskip}{0.08\baselineskip}
% After header:
\vspace{-8pt}
```

### Long CV Example
```latex
\usepackage[margin=0.65in, top=0.25in, bottom=0.4in]{geometry}
\setlist[itemize]{noitemsep, topsep=-2pt, parsep=-1pt, partopsep=0pt}
\titlespacing*{\section}{0pt}{0.12\baselineskip}{0.05\baselineskip}
% After header:
\vspace{-10pt}
```

## Notes for Kiro

- **Always start with SHORT spacing** (default template values)
- **After generating content**, count bullets and sections
- **Apply adjustments** if needed based on decision tree
- **Inform user** which spacing level was used and why
- **Suggest compression** if LONG spacing still doesn't fit content on desired pages
