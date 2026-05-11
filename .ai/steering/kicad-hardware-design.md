---
inclusion: fileMatch
fileMatchPattern: '*.kicad_*'
description: KiCad hardware design analysis and review guidance
---

# KiCad Hardware Design

## When Working with KiCad Projects

You have access to comprehensive KiCad analysis skills that can:

- Parse and analyze schematics (.kicad_sch files)
- Analyze PCB layouts (.kicad_pcb files)
- Review Gerber files for manufacturing
- Run SPICE simulations on detected subcircuits
- Perform EMC pre-compliance checks
- Source components from distributors (DigiKey, Mouser, LCSC, element14)
- Generate BOMs and manufacturing files for JLCPCB/PCBWay

## Automatic Analysis Workflow

When the user asks you to review or analyze a KiCad project:

1. **Scan the directory** for all KiCad files (.kicad_sch, .kicad_pcb, .kicad_pro)
2. **Run all applicable analyzers** in parallel:
   - `analyze_schematic.py` for schematic analysis
   - `analyze_pcb.py --full` for PCB layout analysis
   - `analyze_gerbers.py` if gerber directory exists
3. **Sync datasheets** before verification (critical for correctness checks)
4. **Run SPICE simulation** if ngspice/LTspice/Xyce is available
5. **Run EMC analysis** if both schematic and PCB exist
6. **Cross-reference** schematic vs PCB for consistency
7. **Generate unified report** with all findings

## Analysis Depth

Default to thorough analysis:
- Verify ALL components, not samples
- Use datasheets as ground truth (not KiCad symbols)
- Check pin-to-net mappings against manufacturer datasheets
- Assess plausibility when verification isn't possible
- Think beyond what the analyzer detects

## Key Skills Available

- `kicad` - Main analysis skill for schematics, PCBs, Gerbers
- `spice` - SPICE simulation verification
- `emc` - EMC pre-compliance risk analysis
- `bom` - BOM management and component sourcing
- `digikey` - DigiKey component search and datasheets
- `mouser` - Mouser component search
- `lcsc` - LCSC component search (JLCPCB parts)
- `element14` - Newark/Farnell/element14 search
- `jlcpcb` - JLCPCB fabrication and assembly
- `pcbway` - PCBWay fabrication and assembly

## Common User Requests

- "Analyze my KiCad project" → Run full workflow
- "Check my schematic" → Run schematic analyzer + verification
- "Review my PCB layout" → Run PCB analyzer + DFM checks
- "Will this pass EMC?" → Run EMC pre-compliance analysis
- "Source components" → Use BOM skill + distributor skills
- "Generate JLCPCB files" → Use jlcpcb skill for BOM/CPL export
