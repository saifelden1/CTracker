import re
import os

with open('courses.md', 'r', encoding='utf-8') as f:
    text = f.read()

courses_split = re.split(r'\n### (?!#)', text)[1:]
courses = []
for block in courses_split:
    if '- **Platform:**' in block or '- **Instructors:**' in block or '- **Instructor:**' in block:
        courses.append(block)

html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Courses Tracker</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f4f7f6; margin: 0; padding: 20px; color: #333; }
        h1 { text-align: center; color: #2c3e50; }
        .courses-container { display: grid; grid-template-columns: repeat(auto-fill, minmax(250px, 1fr)); gap: 20px; align-items: start; }
        .course-card { background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); border-top: 4px solid #3498db; display: flex; flex-direction: column; }
        .course-card.not-started { border-top-color: #e74c3c; }
        .course-card.in-progress { border-top-color: #f39c12; }
        .course-card.completed { border-top-color: #2ecc71; }
        .course-title { margin: 0 0 10px; color: #2c3e50; font-size: 20px; }
        .course-info { font-size: 14px; color: #555; margin-bottom: 15px; }
        .course-info span { display: block; margin-bottom: 5px; }
        .status-badge { display: inline-block; padding: 4px 8px; border-radius: 4px; font-size: 12px; font-weight: bold; color: #fff; }
        .status-badge.not-started { background-color: #e74c3c; }
        .status-badge.in-progress { background-color: #f39c12; }
        .status-badge.completed { background-color: #2ecc71; }
        .progress-bar-container { background-color: #ecf0f1; border-radius: 4px; overflow: hidden; height: 20px; margin-bottom: 15px; position: relative; }
        .progress-bar { height: 100%; background-color: #3498db; transition: width 0.5s ease; }
        .progress-text { position: absolute; top: 0; left: 50%; transform: translateX(-50%); font-size: 12px; line-height: 20px; color: #fff; text-shadow: 1px 1px 1px rgba(0,0,0,0.5); font-weight: bold; width: 100%; text-align: center; }
        .modules-list { list-style: none; padding: 0; margin: 0; font-size: 14px; border-top: 1px solid #eee; padding-top: 10px; flex-grow: 1; }
        .section-title { font-weight: bold; margin-top: 15px; margin-bottom: 5px; color: #2980b9; border-bottom: 1px solid #ddd; padding-bottom: 3px; }
        .session-item { padding: 4px 0; border-bottom: 1px solid #f9f9f9; display: flex; justify-content: space-between; padding-left: 10px; font-size: 13px; }
        .session-item:last-child { border-bottom: none; }
    </style>
</head>
<body>
    <h1>📚 My Courses Tracker</h1>
    <div class="courses-container">
"""

for course in courses:
    lines = course.splitlines()
    title = lines[0].strip()
    
    metadata = {}
    content_lines = []
    in_content = False
    
    for line in lines[1:]:
        if line.startswith('- **') and not in_content:
            m = re.match(r'- \*\*([^:]+):\*\*\s*(.*)', line)
            if m:
                metadata[m.group(1).lower()] = m.group(2)
        elif 'Course Content Breakdown' in line or 'Course Structure' in line:
            in_content = True
        elif in_content:
            if line.startswith('####') and ('Progress Summary' in line or 'What You' in line or 'Key Concepts' in line):
                in_content = False
            else:
                content_lines.append(line)

    # Group continuous numbered lectures (e.g. from Udemy) with the same status
    compressed_lines = []
    group = []
    def flush_group():
        if not group: return
        if len(group) == 1:
            compressed_lines.append(group[0][2])
        else:
            first_num = group[0][0]
            last_num = group[-1][0]
            status = group[0][1]
            compressed_lines.append(f'- Lectures {first_num}-{last_num} {status}')
        group.clear()

    for line in content_lines:
        line_s = line.strip()
        if line_s.startswith('- '):
            m = re.match(r'^- (\d+)\.\s+(.*?)\s+(✅|⏳|⌛|🔄|❌)', line_s)
            if m:
                num = int(m.group(1))
                status = m.group(3)
                if group and num == group[-1][0] + 1 and status == group[-1][1]:
                    group.append((num, status, line))
                else:
                    flush_group()
                    group.append((num, status, line))
                continue
        flush_group()
        compressed_lines.append(line)
    flush_group()
    content_lines = compressed_lines
                
    status = metadata.get('status', 'Not Started').lower()
    
    status_class = 'not-started'
    bg_color = '#e74c3c'
    if 'progress' in status or 'rewatching' in status:
        status_class = 'in-progress'
        bg_color = '#f39c12'
    elif 'completed' in status:
        status_class = 'completed'
        bg_color = '#2ecc71'
        
    progress_str = metadata.get('progress', '0%')
    progress_match = re.search(r'(\d+)%', progress_str)
    progress_pct = progress_match.group(1) if progress_match else '0'
    
    html += f'''
        <div class="course-card {status_class}">
            <h2 class="course-title">{title}</h2>
            <div class="course-info">
                <span><strong>Instructor:</strong> {metadata.get('instructor', metadata.get('instructors', 'N/A'))}</span>
                <span><strong>Platform:</strong> {metadata.get('platform', 'N/A')}</span>
                <span><span class="status-badge {status_class}">{metadata.get('status', 'Not Started')}</span></span>
            </div>
            
            <div class="progress-bar-container">
                <div class="progress-bar" style="width: {progress_pct}%; background-color: {bg_color};"></div>
                <div class="progress-text">{progress_pct}%</div>
            </div>
            
            <details>
                <summary style="cursor:pointer; font-weight:bold; padding: 10px 0; color: #2980b9; user-select: none;">📂 View Course Content</summary>
                <div class="modules-list">
'''

    for line in content_lines:
        line = line.strip()
        if not line: continue
        
        # Check if line is a section header
        is_header = False
        if line.startswith('**') and ':**' not in line:
            is_header = True
        elif line.startswith('- **Unit') or line.startswith('- **Module') or line.startswith('- **Section'):
            is_header = True
            
        if is_header:
            # Clean up the header text from asterisks and bullet points
            clean_header = re.sub(r'^\-\s*\*\*', '', line)
            clean_header = re.sub(r'^\*\*', '', clean_header)
            clean_header = clean_header.replace('**', '').strip()
            html += f'                <div class="section-title">{clean_header}</div>\n'
        elif line.startswith('- '):
            html += f'                <div class="session-item"><span>{line[2:]}</span></div>\n'
        elif line.startswith('S') and ':' in line:
             html += f'                <div class="session-item"><span>{line}</span></div>\n'

    html += '''
                </div>
            </details>
        </div>
'''

html += """
    </div>
</body>
</html>
"""

with open('courses_tracker.html', 'w', encoding='utf-8') as f:
    f.write(html)
print("done")