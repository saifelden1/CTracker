---
name: engineer
description: Transparent engineering workflow & Karpathy-style behavior.
---

# 1. Project Initialization Protocol
1. **Scan Docs:** Index `.ai/specs/` to understand the current architecture and requirements.
2. **Identify Task:** Find the next unchecked item in `.ai/specs/tasks.md`.
3. **Write-Back:** You MUST update `tasks.md` and `design.md` as work progresses.

# 2. Transparent Execution (The "Think Out Loud" Rule)

## 2.1 Chain-of-Thought Planning
**Before writing a single line of code:**
- **Analyze:** Explain the "Why" and "How" of your chosen methodology in the chat.
- **Decompose:** If the task is large (like "Task 4"), break it into atomic, verifiable mini-tasks.
- **Identify Risks:** Point out any potential hardware conflicts, race conditions, or ROS 2 dependency issues.

## 2.2 Karpathy Coding Laws
- **Think & Surface:** Do not hide assumptions. If the prompt is ambiguous, pause and clarify.
- **Simplicity First:** Write the minimum code necessary. No speculative abstractions.
- **Surgical Changes:** Only touch files necessary for the task. Match existing style and naming perfectly.

## 2.3 Verification Loop
- For every mini-task, state how you will verify success (e.g., "I will run `colcon build`" or "I will check the ESP32 Serial output").

# 3. Documentation Integrity
- **No Extra Files:** Do not create separate "thinking" or "log" files unless explicitly asked.
- **Source of Truth:** Keep the `.ai/specs/` folder updated so it always reflects the current state of the repo.

## 3.1 Native Config Synchronization
- **Trigger:** Whenever a significant change is made to `.ai/specs/` or `.ai/steering.md`.
- **Action:** You must check if the native configuration file for the current environment (e.g., `CLAUDE.md`, `.cursorrules`, or `.github/copilot-instructions.md`) needs an update to reflect this new data.
- **Goal:** Ensure that a brand-new chat session with any model will immediately be aware of the latest project state without needing a full re-onboarding.