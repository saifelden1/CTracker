---
name: engineer
description: >
  Core engineering workflow guidelines. Includes project initialization protocols (reading specs/tasks) and Andrej Karpathy's behavior guidelines for LLMs (Think before coding, surgical changes, simplicity first, goal-driven execution).
---

# 1. Project Initialization Protocol

At the very beginning of a new task or conversation involving an existing project, ALWAYS perform the following steps before writing any code:
1. **Scan Core Docs:** Check the `.ai/specs/` directory for:
   - `requirements.md` (Scope)
   - `design.md` (Architecture)
   - `tasks.md` (Current State)
2. **Determine Context:** Identify exactly where progress currently stands based on `tasks.md`.

## When to Use
- Starting a brand new chat on an existing project.
- Taking on a large architectural task or debugging session.
- Acting as a senior software engineer to review code or provide surgical changes.

---

# 2. Karpathy Guidelines

Behavioral guidelines to reduce common LLM coding mistakes, derived from Andrej Karpathy's observations on LLM coding pitfalls.
**Tradeoff:** These guidelines bias toward caution over speed. For trivial tasks, use judgment.

## 2.1 Think Before Coding
**Don't assume. Don't hide confusion. Surface tradeoffs.**
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2.2 Simplicity First
**Minimum code that solves the problem. Nothing speculative.**
- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

## 2.3 Surgical Changes
**Touch only what you must. Clean up only your own mess.**
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- Remove imports/variables/functions that YOUR changes made unused.
- The test: Every changed line should trace directly to the user's request.

## 2.4 Goal-Driven Execution
**Define success criteria. Loop until verified.**
Transform tasks into verifiable goals:
- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:
```text
1. [Step] → verify: [check]
2. [Step] → verify: [check]
```