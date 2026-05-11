---
inclusion: always
---
# Master IDE Directives: The  File Protocol

1. Single Source of Truth: The `.ai/specs` file in the root directory is your exclusive configuration and instruction ledger. 
2. No External Documentation: DO NOT generate, suggest, or write to external `.md` files (like `instructions.md` or `rules.md`) to store guidelines. Everything must live inside `.ai` and donot write any md file only if i asked to .
3. Read/Write Autonomy: Before executing complex architectural tasks, silently read the `.ai` file to ensure compliance with the current project standards.
4. Dynamic Updating: If a new fundamental coding rule, hardware constraint, or system architecture decision is established during our conversation, you must proactively offer to append it to the `..ai` file. Only edit the specific sections of `.ai` needed; do not overwrite the entire file unnecessarily.
5. think and evaluate all the options first and ask all of your question before taking any dicision and donot assume anything 