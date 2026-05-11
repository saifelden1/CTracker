---
inclusion: always
---

# 1. Project Initialization Protocol

At the very beginning of a new task or conversation involving an existing project, ALWAYS perform the following steps before writing any code:
1. **Scan Core Docs:** Use your file reading tools to check the `.ai/specs/` directory (or wherever they are stored) for:
   - `requirements.md` (Scope)
   - `design.md` (Architecture)
   - `tasks.md` (Current State)
   *(Note: compensate for potential spelling mistakes like `requiremnts.md`)*
2. **Determine Context:** Identify exactly where my progress currently stands based on `tasks.md` and the existing spec constraints.

---

# 2. Karpathy Guidelines & Core Directives

Behavioral guidelines to reduce common LLM coding mistakes:

## 2.1 Think Before Coding
**Don't assume. Don't hide confusion. Surface tradeoffs.**
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- think and evaluate all options first and ask all your questions before taking any decision and do not assume anything.

## 2.2 Simplicity First
**Minimum code that solves the problem. Nothing speculative.**
- No features beyond what was asked. No abstractions for single-use code.
- If you write 200 lines and it could be 50, rewrite it.

## 2.3 Surgical Changes
**Touch only what you must. Clean up only your own mess.**
- Don't "improve" adjacent code, comments, or formatting.
- Match existing style. Every changed line should trace directly to the user's request.

## 2.4 Goal-Driven Execution
**Define success criteria. Loop until verified.**
- Transform tasks into verifiable goals (e.g., test before/after).

## 2.5 Master IDE Directives
1. **Single Source of Truth:** Core project rules live in the `.ai/` directory. 
2. **No External Documentation:** DO NOT generate `.md` files (like `instructions.md`) unless explicitly asked.
3. **Dynamic Updating:** Offer to append new fundamental rules to the `.ai/steering/` or equivalent directory. Only edit specific sections. 