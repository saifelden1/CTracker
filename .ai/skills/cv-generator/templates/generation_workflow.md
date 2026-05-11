# CV Generation Workflow for Kiro

Quick reference for generating CVs using the template system.

## Step-by-Step Process

### 1. Load Template Files
```
Read: templates/cv_template.tex
Read: templates/spacing_guide.md
```

### 2. Conduct Interview
Follow the interview structure in SKILL.md to collect:
- Personal information
- Professional summary
- Education
- Work experience (optional)
- Projects (optional)
- Skills
- Volunteering (optional)
- Courses (optional)

### 3. Enhance Content
Apply enhancement principles:
- Start bullets with action verbs
- Add quantification (numbers, percentages)
- Include technical details
- Keep concise (1-2 lines per bullet)

### 4. Count Content
```
total_bullets = count all bullet points across sections
total_sections = count included sections
```

### 5. Determine Spacing Level
```
IF total_bullets < 15 AND total_sections < 5:
    spacing_level = "SHORT"
    # Use default template values
    
ELIF total_bullets <= 25 AND total_sections <= 7:
    spacing_level = "MEDIUM"
    # Adjust: top=0.30in, topsep=-1pt, section: 0.15/0.08
    
ELSE:
    spacing_level = "LONG"
    # Adjust: top=0.25in, topsep=-2pt, parsep=-1pt, section: 0.12/0.05
```

### 6. Build Content Strings

**Header placeholders:**
```python
{{NAME}} = user's full name
{{TITLE}} = professional title
{{LOCATION}} = city, country
{{EMAIL}} = email address
{{PHONE}} = phone number
{{LINKEDIN_LINE}} = "LinkedIn: \href{URL}{URL}\\\\" or ""
{{GITHUB_LINE}} = "GitHub: \href{URL}{URL}" or ""
```

**Summary:**
```python
{{SUMMARY}} = enhanced professional summary paragraph
```

**Education:**
```python
{{EDUCATION_ENTRIES}} = """
\textbf{Degree Name} \hfill Start -- End\\
Institution Name \hfill GPA: X.X / 4.0

\textbf{Another Degree} \hfill Start -- End\\
Another Institution
"""
```

**Experience Section:**
```python
IF user has experience:
    {{EXPERIENCE_SECTION}} = """
%==================== EXPERIENCE ====================
\section{Professional Experience}

\textbf{Job Title} \hfill Start -- End\\
Company Name
\begin{itemize}
  \item Enhanced bullet point 1
  \item Enhanced bullet point 2
\end{itemize}

\textbf{Another Job} \hfill Start -- End\\
Another Company
\begin{itemize}
  \item Bullet 1
  \item Bullet 2
\end{itemize}
"""
ELSE:
    {{EXPERIENCE_SECTION}} = ""
```

**Projects Section:**
```python
IF user has projects:
    {{PROJECTS_SECTION}} = """
%==================== PROJECTS ====================
\section{Projects}

\textbf{Project Name} \hfill Date
\begin{itemize}
  \item Enhanced description with technologies
  \item Key achievement or result
\end{itemize}
"""
ELSE:
    {{PROJECTS_SECTION}} = ""
```

**Skills:**
```python
{{SKILLS}} = """
\textbf{Programming:} Language1, Language2, Language3\\
\textbf{Frameworks:} Framework1, Framework2\\
\textbf{Tools:} Tool1, Tool2, Tool3
"""
```

**Volunteering Section:**
```python
IF user has volunteering:
    {{VOLUNTEERING_SECTION}} = """
%==================== VOLUNTEERING ====================
\section{Volunteering Experience}

\textbf{Role} \hfill Date
\begin{itemize}
  \item Contribution 1
  \item Contribution 2
\end{itemize}
"""
ELSE:
    {{VOLUNTEERING_SECTION}} = ""
```

**Courses Section:**
```python
IF user has courses:
    {{COURSES_SECTION}} = """
%==================== COURSES ====================
\section{Courses \& Training}

\textbf{Course Name (Duration)} \hfill Date\\
Provider
\begin{itemize}
  \item Key topic 1
  \item Key topic 2
\end{itemize}
"""
ELSE:
    {{COURSES_SECTION}} = ""
```

### 7. Apply Spacing Adjustments

**If MEDIUM spacing:**
```latex
Replace: top=0.35in → top=0.30in
Replace: topsep=0pt → topsep=-1pt
Replace: {0pt}{0.2\baselineskip}{0.1\baselineskip} → {0pt}{0.15\baselineskip}{0.08\baselineskip}
Replace: \vspace{-5pt} → \vspace{-8pt}
```

**If LONG spacing:**
```latex
Replace: top=0.35in → top=0.25in
Replace: topsep=0pt, parsep=0pt → topsep=-2pt, parsep=-1pt
Replace: {0pt}{0.2\baselineskip}{0.1\baselineskip} → {0pt}{0.12\baselineskip}{0.05\baselineskip}
Replace: \vspace{-5pt} → \vspace{-10pt}
```

### 8. Replace All Placeholders

Load template, replace all `{{PLACEHOLDER}}` with actual content.

### 9. Generate File

Save as: `{firstname}_{lastname}_cv.tex`

### 10. Inform User

```
✅ CV generated: {filename}

Spacing level: {SHORT/MEDIUM/LONG}
Estimated pages: {1-2}
Total sections: {count}
Total bullet points: {count}

Next step: Compile to PDF
- Easy: Upload to Overleaf.com
- Local: Run `pdflatex {filename}`
```

## Example Output Message

```
All done! 🎉

I've generated your professional CV: **sarah_johnson_cv.tex**

📊 CV Statistics:
- Spacing level: MEDIUM (content fits tightly on 1 page)
- Sections included: 6 (Summary, Education, Experience, Projects, Skills, Courses)
- Total bullet points: 18

✅ What I created:
- Professional header with contact info
- Enhanced summary highlighting your 5 years of experience
- Education from MIT with GPA
- 2 work experiences with 8 enhanced bullet points
- 2 projects with technical details
- Skills organized by category
- 1 course with key topics

📄 Next step: Compile to PDF
- **Easy way:** Upload to Overleaf.com
- **Local way:** Run `pdflatex sarah_johnson_cv.tex`

Need help compiling? I can guide you through it!
```

## Common Issues & Solutions

**Issue: Content overflows to 2 pages**
- Solution: Use LONG spacing, or suggest removing optional sections

**Issue: Too much white space**
- Solution: Use SHORT spacing (default)

**Issue: User wants 2-page CV**
- Solution: Use SHORT spacing, add more content or expand descriptions

**Issue: LaTeX compilation errors**
- Solution: Check for special characters that need escaping: `& % $ # _ { } ~ ^`
