# How to Use CV Generator

## Quick Start

1. **Activate the skill** by typing in chat:
   ```
   /cv-generator
   ```
   Or just say: "Create my CV" or "Help me build my resume"

2. **Answer Kiro's questions**
   - Kiro will ask about your name, experience, skills, etc.
   - Give brief, casual answers - AI will enhance them!
   - Example: "I made websites" → AI enhances to "Developed responsive web applications"

3. **Kiro generates your CV**
   - AI creates a professional LaTeX file: `your_name_cv.tex`
   - All your answers are enhanced with action verbs and numbers

4. **Compile to PDF**
   - **Easy way**: Upload to [Overleaf.com](https://www.overleaf.com)
   - **Local way**: Run `pdflatex your_name_cv.tex`

## That's It!

The entire process is conversational. Just answer questions naturally and Kiro handles the rest.

## What You'll Be Asked

**Required:**
- Personal info (name, email, phone, location)
- Professional summary (your background and goals)
- Education (degree, school, dates)
- Skills (programming languages, tools, etc.)

**Optional:**
- Work experience
- Projects
- Volunteering
- Courses & certifications

## Example

**You:** `/cv-generator`

**Kiro:** "What's your full name?"

**You:** "John Smith"

**Kiro:** "What's your professional title?"

**You:** "I'm studying computer science"

**Kiro:** "I'll format that as: 'Computer Science Student -- Software Development'. What's your email?"

[... continues through all sections ...]

**Kiro:** "Done! I've generated `john_smith_cv.tex`. Upload it to Overleaf.com to get your PDF!"

## Need Help Compiling?

See `compilation-guide.html` for detailed instructions with screenshots.
