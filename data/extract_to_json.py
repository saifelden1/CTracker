import json
import re

def parse_markdown_to_json(md_filepath, json_filepath):
    with open(md_filepath, 'r', encoding='utf-8') as f:
        md_content = f.read()

    # Split by course boundaries: "### Course Name"
    # Using a lookahead so we don't consume the "### " or we can just findall blocks
    courses = []
    
    # Simple regex to extract course blocks (everything between ### headers)
    # We might need to handle the end of the file or next major section (##)
    # Let's split by "### " but ignore "#### " 
    blocks = re.split(r'\n### (?=[^#])', md_content)
    
    for block in blocks[1:]: # Skip text before the first course
        lines = block.strip().split('\n')
        if not lines: continue
        
        course_name = lines[0].strip()
        
        course_data = {
            "version": "1.0",
            "type": "course",
            "name": course_name,
            "units": []
        }
        
        # We need to parse units and sessions.
        # Sections start with **Section X: Name** or **Unit X: Name**
        # Sessions are list items like "- 1. Session Name (time) [status]"
        
        current_unit = None
        
        for line in lines[1:]:
            line = line.strip()
            
            # Match a unit/section
            # e.g. **Section 6: Write a Custom Controller (6 lectures)** ⏳ NOT STARTED
            # e.g. **Unit 4: Data Structures in C++** ⏳ NOT STARTED
            unit_match = re.match(r'^\*\*(?:Section|Unit|Module)\s+(.*?)\*\*.*', line, re.IGNORECASE)
            if unit_match:
                # Save previous unit if exists
                if current_unit:
                    course_data["units"].append(current_unit)
                    
                unit_name = unit_match.group(1).strip()
                current_unit = {
                    "name": f"Module {unit_name}", # Or keep the exact name
                    "sessions": []
                }
                continue
                
            # Match a session
            # e.g. "- 39. Intro (4min) ⏳"
            # e.g. "- Lesson 1: Intro to C++ ✅"
            session_match = re.match(r'^-\s+(.*?)(?:\s*(?:✅|⏳|\(.*\)))*$', line)
            
            if session_match and line.startswith('- ') and not line.startswith('- **'):
                if not current_unit:
                    # Some courses might not have explicit units, create a default one
                    current_unit = {
                        "name": "General",
                        "sessions": []
                    }
                
                # Determine progress
                progress = 0
                if '✅' in line or 'COMPLETED' in line:
                    progress = 100
                elif '⏳' in line or 'INCOMPLETE' in line or 'NOT STARTED' in line:
                    progress = 0
                
                session_name = session_match.group(1).strip()
                
                # Clean up session name (remove trailing ✅ ⏳ etc)
                session_name = re.sub(r'\s*(?:✅|⏳|\[.*?\])$', '', session_name).strip()
                
                current_unit["sessions"].append({
                    "name": session_name,
                    "progress": progress
                })
        
        # Append the last unit
        if current_unit:
            course_data["units"].append(current_unit)
            
        courses.append(course_data)
        
    final_output = {
        "entities": courses
    }
    
    with open(json_filepath, 'w', encoding='utf-8') as f:
        json.dump(final_output, f, indent=4)
        
if __name__ == "__main__":
    parse_markdown_to_json("courses.md", "courses_import.json")
