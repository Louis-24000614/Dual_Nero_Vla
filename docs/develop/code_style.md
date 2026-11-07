# Code style

[中文](../../docs_cn/develop/code_style_cn.md) | [Modules](modules.md)

This page is the first-party style for Dual Nero VLA. It does not restyle `AgileX/`. Vendor files keep the upstream layout so they can still be compared with the official Nero SDK.

## Repository configuration

`.clang-format` is the C++ formatter. It uses the Google style and a 100-column limit. `.editorconfig` sets UTF-8, LF, and two-space indentation for ordinary text. Python files use four spaces because that is what a Python formatter enforces. Do not add an editor-specific settings directory.

The root `CMakeLists.txt` pins C++20, requires that standard, and disables compiler extensions. Each future compiled module gets its own `CMakeLists.txt`, lists its sources, and declares public and private include paths and link dependencies. Public headers belong in that module's `include/`. Private helpers stay in the module or in `detail/` and are not included from outside. The root file adds modules in dependency order. Generated interface targets go through a helper instead of a handwritten generated path.

## Comments and headers

Comments and docstrings are English. They record a non-obvious constraint, unit, owner, concurrency rule, or failure. They do not repeat a clear name. A public C++ declaration documents ownership, whether the call blocks, and how it reports errors when a caller needs that to use it safely. Doxygen on those declarations is enough. Generated Doxygen output is not edited by hand.

## Python

The first Python package targets Python 3.12. Public functions have type hints. A short docstring describes a contract that the signature does not already state. Numerical limits, hardware assumptions, and failure behavior that are not obvious belong in English comments. Type-check settings go in that package's `pyproject.toml` when the package is added.

## Tests and logs

Name a test `test_<what>_<condition>_<expected>`. A unit test feeds a mock or a stub and does not require a live arm. State the method next to the module: unit with mock, stubbed integration, vcan, replay, or manual hardware acceptance. Log lines that this product emits later follow the record in [logging](../debug/logging.md). Do not log secrets, raw images, prompts, or unbounded payloads.

Format, lint, and test with the tools declared here before a commit. Keep a formatting-only commit separate from a behavior change when the mixed diff would hide the behavior.
