# GEMINI.md - Mandates & Reference Guide

## 1. Learning Mandate (HIGHEST PRIORITY)
This is a **LEARNING PROJECT** focused on low-level systems and architectures. The primary goal is the user's knowledge acquisition, not just functional code.

- **Collaborative Decisions:** Never implement a significant architectural or low-level design choice autonomously. Always present options, explain the trade-offs, and involve the user in the decision-making process.
- **Deep Understanding:** When explaining or implementing, focus on *how* and *why* things work at the hardware/architecture level. Do not aim for the quickest "fix"; aim for the most educational path.
- **Pedagogical Communication:** Inquiries should be met with detailed technical rationale. Directives should be executed with "show-your-work" transparency regarding low-level details (e.g., bit manipulation, register usage, memory layouts).

## 2. Single Source of Truth
This document defines the core operational mandates for Gemini CLI in this workspace. Technical standards and workflows are delegated to the local `Guidelines/` repository, which is the **single source of truth**. All coding standards, architectural patterns, and agent behaviors are defined in the following locations. **Always consult these before making changes or decisions.**

- **Common Guidelines:** [Guidelines/guidelines/common_guidelines](Guidelines/guidelines/common_guidelines)
- **Python Standards:** [Guidelines/guidelines/python_guidelines](Guidelines/guidelines/python_guidelines)
- **C++ Standards:** [Guidelines/guidelines/cpp_guidelines](Guidelines/guidelines/cpp_guidelines)
- **Agent Patterns & Protocols:** [Guidelines/agents/agents_base.md](Guidelines/agents/agents_base.md)
- **Verification Protocols:** [Guidelines/VERIFICATION.md](Guidelines/VERIFICATION.md)

---

## 3. Core Mandates

### Security & System Integrity
- **Credential Protection:** Never log, print, or commit secrets, API keys, or sensitive credentials. Rigorously protect `.env` files, `.git`, and system configuration folders.
- **Source Control:** Do not stage or commit changes unless specifically requested by the user.

### Context Efficiency
- **Search First:** Use `grep_search` and `glob` to map the codebase before reading files.
- **Parallelism:** Combine independent tool calls in parallel to reduce turns.
- **Minimal Reads:** Use `start_line` and `end_line` for targeted, surgical reads.

### Engineering Standards
- **Technical Integrity:** You are responsible for the entire lifecycle: implementation, testing, and validation.
- **Empirical Reproduction:** For bug fixes, you MUST empirically reproduce the failure with a new test case before applying the fix.
- **Validation:** Always run tests, linting, and type-checking after changes. A task is only complete when behavioral correctness is verified and structural integrity is confirmed.
