---
name: "cv-generator"
description: "AI-powered interactive CV builder that creates professional LaTeX resumes through conversational interview"
keywords: ["cv", "resume", "latex", "career", "job", "interview", "professional", "curriculum vitae"]
---

# CV Generator Skill

AI-powered interactive CV builder that creates professional LaTeX resumes through conversational interview.

## Overview

This skill provides an **AI-interactive workflow** where Kiro guides you through building your CV:

1. **Kiro asks questions** - One section at a time
2. **You provide raw answers** - Brief, casual responses are fine
3. **Kiro enhances content** - Transforms your answers into professional CV language
4. **Kiro generates LaTeX** - Creates formatted `.tex` file ready for PDF compilation

## Activation

Type in chat:
```
/cv-generator
```

Or simply say:
- "Create my CV"
- "Help me build my resume"
- "Generate my CV"

## Features

- **Conversational interface**: Natural question-and-answer flow
- **AI enhancement**: Raw answers → professional CV language
- **Action verbs**: Automatically adds strong verbs (Developed, Led, Implemented)
- **Quantification**: Prompts for numbers and metrics
- **Optional sections**: Skip volunteering, projects, courses if not applicable
- **Instant generation**: LaTeX file created immediately after interview
- **Professional format**: Optimized spacing, clean layout, ATS-friendly

## When to Use

Use this skill when:
- Creating a CV from scratch
- You have experience but struggle with professional wording
- You want AI to polish your descriptions
- You need a quick, guided CV creation process
- You prefer conversation over filling forms

## Complete Workflow

### Phase 1: Interview (Kiro asks, you answer)

**Section 1: Personal Information**
- Full name
- Professional title/field
- Location
- Email, phone
- LinkedIn, GitHub (optional)

**Section 2: Professional Summary**
- Background and specialization
- Key skills
- Career goals

**Section 3: Education**
- Degree(s)
- Institution(s)
- Duration, GPA

**Section 4: Work Experience** (optional)
- Job title, company, duration
- 3-5 responsibilities per job
- Kiro prompts for specifics and numbers

**Section 5: Projects** (optional)
- Project name, duration
- Technologies used
- Key achievements

**Section 6: Skills**
- Programming languages
- Tools & frameworks
- Technical skills

**Section 7: Volunteering** (optional)
- Role, organization, duration
- Key contributions

**Section 8: Courses & Training** (optional)
- Course name, provider
- Key topics covered

### Phase 2: Enhancement (Kiro transforms your answers)

**Enhancement Principles:**

1. **Action Verbs**: Developed, Implemented, Led, Designed, Optimized, Built
2. **Quantification**: Add numbers ("improved by 40%", "team of 5", "10K+ users")
3. **Specificity**: Replace vague terms with concrete details
4. **Professional Tone**: Formal but readable
5. **Conciseness**: 1-2 lines per bullet point

**Enhancement Examples:**

| Your Answer | Kiro's Enhancement |
|-------------|-------------------|
| "I made websites for a company" | "Developed responsive web applications using React and Node.js, serving 5,000+ daily users" |
| "I was team leader for a project" | "Led cross-functional team of 6 members to deliver project 2 weeks ahead of schedule" |
| "I know Python and can code" | "Proficient in Python with experience in data analysis, automation scripting, and backend development" |
| "Helped students learn coding" | "Mentored 15+ students in programming fundamentals, conducting weekly workshops and code review sessions" |

### Phase 3: Generation (Kiro creates LaTeX file)

Kiro automatically generates:
- Professional header with contact info
- Enhanced summary section
- Formatted education entries
- Work experience with bullet points
- Projects with technical details
- Skills organized by category
- Optional sections as provided

Output: `your_name_cv.tex`

### Phase 4: Compilation (You create PDF)

**Option A: Online (Easiest)**
1. Go to [Overleaf.com](https://www.overleaf.com)
2. Upload the `.tex` file
3. Download PDF

**Option B: Local**
```bash
pdflatex your_name_cv.tex
```

## Example Conversation

**User:** `/cv-generator`

**Kiro:** "Let's create your professional CV! I'll ask questions, you provide answers (brief is fine), and I'll enhance them professionally.

Starting with personal information...

**What's your full name?**"

**User:** "Sarah Johnson"

**Kiro:** "**What's your professional title or field?**"

**User:** "I'm a software engineer, I work with web technologies"

**Kiro:** "Great! I'll format that as: **'Software Engineer -- Web Development'**

**Where are you located?**"

**User:** "Boston, MA"

**Kiro:** "**What's your email?**"

**User:** "sarah.j@email.com"

**Kiro:** "**Phone number?**"

**User:** "+1-555-0123"

**Kiro:** "**LinkedIn profile URL?** (optional)"

**User:** "linkedin.com/in/sarahjohnson"

**Kiro:** "I'll format that as: **https://www.linkedin.com/in/sarahjohnson**

**GitHub profile?** (optional)"

**User:** "github.com/sarahj"

**Kiro:** "Perfect! Now for your professional summary...

**Tell me about yourself professionally - your background, key skills, and what you're looking for. Just give me the main points, I'll make it sound professional.**"

**User:** "I've been coding for 5 years, mostly React and Node.js. I like building scalable apps. Looking for senior roles."

**Kiro:** "Excellent! I'll enhance that to:

> *'Software engineer with 5+ years of experience building scalable web applications. Specialized in React and Node.js with strong focus on performance and user experience. Proven track record of delivering high-quality solutions. Seeking senior engineering roles to drive technical innovation and mentor development teams.'*

Now let's talk about your education...

**What degree do you have?**"

**User:** "Bachelor's in Computer Science from MIT"

**Kiro:** "**When did you attend?**"

**User:** "2015 to 2019"

**Kiro:** "**GPA?** (optional)"

**User:** "3.7"

**Kiro:** "Great! Formatted as:

**Bachelor of Science in Computer Science** | Sep 2015 -- Jun 2019  
Massachusetts Institute of Technology | GPA: 3.7 / 4.0

**Do you have work experience to include?**"

**User:** "Yes"

**Kiro:** "**What was your job title?**"

**User:** "Senior Software Engineer"

**Kiro:** "**Company name?**"

**User:** "TechCorp"

**Kiro:** "**Duration?**"

**User:** "June 2021 to now"

**Kiro:** "**Tell me what you did there - just the key points:**"

**User:** "Built microservices, led a team, improved performance, worked on CI/CD"

**Kiro:** "Perfect! Let me get more details...

**How many microservices did you build? What technologies?**"

**User:** "About 8 services using Node.js and Docker"

**Kiro:** "**How many people on your team?**"

**User:** "4 developers"

**Kiro:** "**How much did you improve performance?**"

**User:** "Response time went from 2 seconds to 500ms"

**Kiro:** "Excellent! I'll enhance these to:

- Architected and developed 8+ microservices using Node.js and Docker, serving 50K+ daily active users
- Led team of 4 engineers in agile development, achieving 95% sprint completion rate
- Optimized application performance, reducing API response time by 75% (2s → 500ms)
- Implemented CI/CD pipelines using GitHub Actions, reducing deployment time by 60%

**Add another job?**"

**User:** "No, that's it"

**Kiro:** "**Do you have projects to showcase?**"

**User:** "Yes, I built an e-commerce platform"

[... continues through all sections ...]

**Kiro:** "All done! 🎉

I've generated your professional CV: **sarah_johnson_cv.tex**

Here's what I created:
✅ Professional header with contact info
✅ Enhanced summary highlighting your 5 years of experience
✅ Education from MIT with GPA
✅ Senior Software Engineer role with 4 enhanced bullet points
✅ E-commerce project with technical details
✅ Skills organized by category

**Next step:** Compile to PDF
- **Easy way:** Upload to Overleaf.com
- **Local way:** Run `pdflatex sarah_johnson_cv.tex`

Need help compiling? I can guide you through it!"

## AI Enhancement Guidelines (For Kiro)

When enhancing user responses:

1. **Identify key information** from casual answers
2. **Add action verbs** at the start of each bullet
3. **Quantify** whenever possible (ask follow-up questions for numbers)
4. **Add technical details** (technologies, tools, methods)
5. **Include impact/results** when mentioned
6. **Keep concise** (1-2 lines maximum per bullet)
7. **Maintain professional tone** without being overly formal
8. **Structure consistently** across all sections

### Action Verbs to Use
- **Development**: Developed, Built, Implemented, Designed, Created, Engineered
- **Leadership**: Led, Managed, Directed, Coordinated, Supervised, Mentored
- **Improvement**: Optimized, Enhanced, Improved, Streamlined, Reduced, Increased
- **Collaboration**: Collaborated, Partnered, Coordinated, Facilitated
- **Analysis**: Analyzed, Evaluated, Assessed, Investigated, Researched

### Follow-up Questions for Quantification
- "How many [users/items/projects]?"
- "What percentage improvement?"
- "How much time saved?"
- "What was the team size?"
- "What technologies/tools did you use?"

## Required vs Optional Sections

**Required (will prompt):**
- Personal Information (name, email, phone)
- Professional Summary
- Education
- Skills

**Optional (will ask if user wants to include):**
- Work Experience
- Projects
- Volunteering
- Courses & Training

## Template System

This skill uses a **LaTeX template** with **adaptive spacing** that adjusts based on CV content length.

### Template Files
- `templates/cv_template.tex` - Main LaTeX template with placeholders
- `templates/spacing_guide.md` - Spacing adjustment rules based on content length

### How Template Generation Works

1. **Collect user information** through conversational interview
2. **Enhance content** with professional language and action verbs
3. **Count content** (bullet points, sections) to determine spacing level
4. **Apply spacing adjustments** based on content length:
   - **SHORT CV** (< 15 bullets, < 5 sections): Default spacing
   - **MEDIUM CV** (15-25 bullets, 5-7 sections): Tighter spacing
   - **LONG CV** (> 25 bullets, > 7 sections): Maximum compression
5. **Fill template** with enhanced content and adjusted spacing
6. **Generate .tex file** ready for compilation

### Spacing Levels

**SHORT (Default):**
```latex
top=0.35in, topsep=0pt, section spacing: 0.2/0.1 baselineskip
```

**MEDIUM (Tighter):**
```latex
top=0.30in, topsep=-1pt, section spacing: 0.15/0.08 baselineskip
```

**LONG (Maximum Compression):**
```latex
top=0.25in, topsep=-2pt, parsep=-1pt, section spacing: 0.12/0.05 baselineskip
```

### Template Placeholders

The template uses these placeholders that get replaced with user content:

**Header:**
- `{{NAME}}` - Full name
- `{{TITLE}}` - Professional title/field
- `{{LOCATION}}` - City, country
- `{{EMAIL}}` - Email address
- `{{PHONE}}` - Phone number
- `{{LINKEDIN_LINE}}` - LinkedIn URL (or empty if not provided)
- `{{GITHUB_LINE}}` - GitHub URL (or empty if not provided)

**Content Sections:**
- `{{SUMMARY}}` - Professional summary paragraph
- `{{EDUCATION_ENTRIES}}` - All education entries
- `{{EXPERIENCE_SECTION}}` - Entire experience section (or empty if skipped)
- `{{VOLUNTEERING_SECTION}}` - Entire volunteering section (or empty if skipped)
- `{{PROJECTS_SECTION}}` - Entire projects section (or empty if skipped)
- `{{SKILLS}}` - Skills formatted as: `\textbf{Category:} skill1, skill2\\`
- `{{COURSES_SECTION}}` - Entire courses section (or empty if skipped)

### Generation Instructions for Kiro

When generating a CV:

1. **Read the template**: Load `templates/cv_template.tex`
2. **Read spacing guide**: Load `templates/spacing_guide.md`
3. **Conduct interview**: Collect all user information
4. **Enhance content**: Apply professional language and action verbs
5. **Count content**: 
   - Total bullet points across all sections
   - Number of sections included
6. **Determine spacing level**: Use decision tree from spacing guide
7. **Adjust template**: Modify spacing parameters if MEDIUM or LONG
8. **Replace placeholders**: Fill in all `{{PLACEHOLDER}}` values
9. **Handle optional sections**: 
   - If section is skipped, replace entire section placeholder with empty string
   - For LinkedIn/GitHub, format as: `LinkedIn: \href{URL}{URL}\\` or empty
10. **Generate file**: Save as `{firstname}_{lastname}_cv.tex`
11. **Inform user**: 
    - Which spacing level was used
    - Estimated page count
    - Next steps for compilation

### Example Section Formatting

**Education Entry:**
```latex
\textbf{Bachelor of Science in Computer Science} \hfill Sep 2015 -- Jun 2019\\
Massachusetts Institute of Technology \hfill GPA: 3.7 / 4.0
```

**Experience Entry:**
```latex
\textbf{Senior Software Engineer} \hfill Jun 2021 -- Present\\
TechCorp
\begin{itemize}
  \item Architected and developed 8+ microservices using Node.js and Docker, serving 50K+ daily active users
  \item Led team of 4 engineers in agile development, achieving 95% sprint completion rate
  \item Optimized application performance, reducing API response time by 75% (2s → 500ms)
\end{itemize}
```

**Skills:**
```latex
\textbf{Programming:} C, C++, Python, JavaScript\\
\textbf{Frameworks:} React, Node.js, Django\\
\textbf{Tools:} Git, Docker, AWS
```

## Files in This Skill

- `SKILL.md` - This documentation (AI workflow instructions)
- `templates/cv_template.tex` - LaTeX template with adaptive spacing
- `templates/spacing_guide.md` - Spacing adjustment rules
- `compilation-guide.html` - Visual PDF compilation guide
- `references/compilation-guide.md` - Detailed compilation instructions
