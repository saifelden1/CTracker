# CV Generator Skill - Setup Complete ✅

## What Was Created

This skill now has a complete template-based CV generation system with adaptive spacing.

### Files Structure

```
SKILLS/cv-generator/
├── SKILL.md                              # Main skill documentation (with front-matter metadata)
├── README.md                             # This file
├── HOW_TO_USE.md                         # User guide
├── compilation-guide.html                # Visual compilation guide
├── templates/
│   ├── cv_template.tex                   # LaTeX template with placeholders
│   ├── spacing_guide.md                  # Spacing adjustment rules
│   └── generation_workflow.md            # Step-by-step generation process for Kiro
└── references/
    └── compilation-guide.md              # Detailed compilation instructions
```

## How It Works

### 1. Template-Based Generation
- Uses `cv_template.tex` with your exact formatting from `resume1.tex`
- Placeholders like `{{NAME}}`, `{{EMAIL}}` get replaced with user content
- Maintains consistent LaTeX structure and styling

### 2. Adaptive Spacing
The template automatically adjusts spacing based on content length:

| Content Level | Criteria | Spacing Adjustments |
|--------------|----------|---------------------|
| **SHORT** | < 15 bullets, < 5 sections | Default (top=0.35in, topsep=0pt) |
| **MEDIUM** | 15-25 bullets, 5-7 sections | Tighter (top=0.30in, topsep=-1pt) |
| **LONG** | > 25 bullets, > 7 sections | Maximum compression (top=0.25in, topsep=-2pt) |

### 3. AI Enhancement
- Kiro interviews you conversationally
- Transforms casual answers into professional CV language
- Adds action verbs, quantification, and technical details
- Maintains your exact LaTeX formatting

## How to Use

### Activate the Skill

In Kiro chat, type:
```
/cv-generator
```

Or say:
- "Create my CV"
- "Help me build my resume"
- "Generate my CV"

### What Happens

1. **Interview**: Kiro asks questions section by section
2. **Enhancement**: Your answers are transformed into professional language
3. **Generation**: LaTeX file is created with adaptive spacing
4. **Compilation**: You compile to PDF (Overleaf or local)

## Example Flow

```
You: /cv-generator

Kiro: Let's create your professional CV! I'll ask questions, you provide 
      answers (brief is fine), and I'll enhance them professionally.
      
      What's your full name?

You: John Smith

Kiro: What's your professional title or field?

You: I'm a software engineer

[... interview continues ...]

Kiro: All done! 🎉

      I've generated your professional CV: john_smith_cv.tex
      
      📊 CV Statistics:
      - Spacing level: MEDIUM (content fits tightly on 1 page)
      - Sections included: 6
      - Total bullet points: 18
      
      📄 Next step: Compile to PDF
      - Easy way: Upload to Overleaf.com
      - Local way: Run `pdflatex john_smith_cv.tex`
```

## Key Features

✅ **Exact format match** - Uses your `resume1.tex` formatting  
✅ **Adaptive spacing** - Automatically adjusts based on content length  
✅ **AI enhancement** - Professional wording with action verbs  
✅ **Conversational** - Natural question-and-answer flow  
✅ **Optional sections** - Skip what you don't need  
✅ **Consistent output** - Same format every time  

## Technical Details

### Template Placeholders

The template uses these placeholders that get replaced:

**Header:**
- `{{NAME}}`, `{{TITLE}}`, `{{LOCATION}}`
- `{{EMAIL}}`, `{{PHONE}}`
- `{{LINKEDIN_LINE}}`, `{{GITHUB_LINE}}`

**Content:**
- `{{SUMMARY}}` - Professional summary
- `{{EDUCATION_ENTRIES}}` - All education entries
- `{{EXPERIENCE_SECTION}}` - Work experience (or empty)
- `{{PROJECTS_SECTION}}` - Projects (or empty)
- `{{VOLUNTEERING_SECTION}}` - Volunteering (or empty)
- `{{SKILLS}}` - Skills by category
- `{{COURSES_SECTION}}` - Courses & training (or empty)

### Spacing Parameters

The template adjusts these LaTeX parameters:

1. **Top margin**: `geometry` package `top` value
2. **List spacing**: `enumitem` package `topsep`, `parsep` values
3. **Section spacing**: `titlesec` package before/after values
4. **Header spacing**: `\vspace` after header

## Next Steps

1. **Test the skill**: Try generating a CV with `/cv-generator`
2. **Verify output**: Check that generated `.tex` matches your format
3. **Compile**: Test PDF generation on Overleaf or locally
4. **Iterate**: Provide feedback if adjustments are needed

## Troubleshooting

**Skill doesn't show up:**
- Make sure `SKILL.md` has front-matter metadata (✅ already added)
- Restart Kiro or reload the workspace

**Spacing is wrong:**
- Check `spacing_guide.md` for adjustment rules
- Kiro will automatically select spacing level based on content

**LaTeX errors:**
- Check for special characters that need escaping
- Verify all placeholders were replaced

**Format doesn't match:**
- Compare generated `.tex` with your `resume1.tex`
- Template should match exactly - report any differences

## Files Reference

- **SKILL.md**: Main documentation for Kiro (how to conduct interview, enhance content)
- **cv_template.tex**: LaTeX template with your exact formatting
- **spacing_guide.md**: Rules for adjusting spacing based on content length
- **generation_workflow.md**: Step-by-step process for Kiro to follow
- **compilation-guide.html**: Visual guide for users on compiling to PDF

---

**Status**: ✅ Ready to use  
**Last Updated**: 2026-04-23  
**Template Version**: 1.0
