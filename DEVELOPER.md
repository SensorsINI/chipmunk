# Developer Guide

This document provides guidelines and best practices for developing Chipmunk tools.

## Cross-Platform Development

Chipmunk is designed to compile and run on both **Linux** (primary target) and **macOS** (including Apple Silicon). This requires careful attention to compiler compatibility.

### Compiler Differences

- **Linux**: Uses GCC (GNU Compiler Collection)
- **macOS**: Uses Clang (Apple's LLVM-based compiler)

### Critical: Avoid GNU C Extensions

**DO NOT use GNU C extensions that are not supported by Clang:**

#### ❌ Nested Functions (GNU C Extension)

```c
/* BAD: Nested functions don't work on macOS/Clang */
void some_function() {
    auto void helper() {  /* ERROR: Not supported by Clang */
        /* ... */
    }
    helper();
}
```

#### ✅ Static File-Scope Functions (Standard C)

```c
/* GOOD: Static functions work on both GCC and Clang */
static void helper() {  /* Standard C - works everywhere */
    /* ... */
}

void some_function() {
    helper();
}
```

**Why this matters:**
- Nested functions (`auto void func() {}`) are a GNU C extension
- Clang on macOS does NOT support nested functions
- Using nested functions causes compilation errors on macOS
- Static file-scope functions are standard C89/C99 and work on both platforms

**Example from codebase:**
- See `psys/src/mylib.c` - `dbglog_inkey()`, `dbglog_inkeyn()`, `dbglog_testkey()`
- See `log/src/log.c` - `dbglog_inkey2()`

All of these were converted from nested functions to static file-scope functions for cross-platform compatibility.

### Compiler Warning Flags

The Makefiles use platform detection to set appropriate warning flags:

- **macOS (Clang)**: `-Wno-deprecated-non-prototype` (Clang doesn't support GCC-only flags)
- **Linux (GCC)**: `-Wno-format-overflow -Wno-stringop-overflow -Wno-deprecated-non-prototype`

The build system automatically detects the platform using `uname -s` and sets the correct flags.

### Testing Cross-Platform Compatibility

Before committing changes:

1. **Test on Linux** (primary target):
   ```bash
   make clean && make
   ```

2. **Test on macOS** (if available):
   ```bash
   make clean && make
   ```

3. **Verify no GNU C extensions** are used:
   - Search for `auto void` or `auto int` (nested functions)
   - Search for other GNU extensions that might not be portable

## Code Style

### Function Declarations

- Use standard C function declarations
- Avoid K&R style function definitions when possible (though legacy code may still use them)
- Use `static` for file-scope helper functions

### Platform-Specific Code

If platform-specific code is necessary, use preprocessor directives:

```c
#ifdef __APPLE__
    /* macOS-specific code */
#else
    /* Linux/other Unix code */
#endif
```

Or use Makefile-based platform detection:

```makefile
ifeq ($(UNAME_S),Darwin)
    # macOS-specific flags
else
    # Linux-specific flags
endif
```

## Build System

### Makefile Structure

- **Top-level Makefile**: Orchestrates build of psys and log
- **psys/src/Makefile**: Builds p-system emulation libraries
- **log/src/Makefile**: Builds log tools (depends on psys)
- **log/src/ana/Makefile**: Builds analog simulation components

All Makefiles use platform detection for compiler flags and paths.

### Adding New Source Files

1. Add the `.c` file to the appropriate `OBJS` list in the Makefile
2. Ensure the code follows cross-platform guidelines above
3. Test build on both Linux and macOS (if possible)

## Debugging

### Debug Logging

Debug logging is controlled via environment variables:

- `CHIPMUNK_DEBUG_ESC=1`: Enable ESC/^C debug logging
- `CHIPMUNK_DEBUG_ESC_FILE=/path/to/file`: Custom log file location

### Common Build Issues

1. **Nested function errors on macOS**: Convert to static file-scope functions
2. **Unknown warning option**: Check if flag is GCC-specific (use platform detection)
3. **X11 not found**: Ensure XQuartz is installed on macOS (`brew install --cask xquartz`)

# Release workflow

## Daily work - commit directly to main
```bash
git add <files>
git commit -m "Description"
git push origin main
```

## For releases
```bash
echo "6.4.0" > VERSION
git add VERSION
git commit -m "Bump version to 6.4.0"
git tag -a v6.4.0 -m "Release v6.4.0: ..."
git push origin main
git push origin v6.4.0
# Then create GitHub release - CI handles the rest!
```

## Contributing

When contributing code:

1. Follow cross-platform guidelines above
2. Test on Linux (required) and macOS (if available)
3. Update documentation if adding new features
4. Add appropriate comments explaining non-obvious code
5. Update CHANGELOG.md for user-visible changes

## References

- [GNU C Extensions](https://gcc.gnu.org/onlinedocs/gcc/C-Extensions.html)
- [Clang Compatibility](https://clang.llvm.org/docs/UsersManual.html#c)
- [Standard C (C89/C99)](https://en.wikipedia.org/wiki/C99)
