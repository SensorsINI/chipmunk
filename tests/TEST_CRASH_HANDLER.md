# Testing the Crash Handler

The crash handler has been added to provide better diagnostics when the program crashes. Here's how to test it:

## Quick Test with Test Program

A test program is provided to verify the crash handler works:

```bash
# Compile the test program
gcc -g -rdynamic -o test_crash_handler test_crash_handler.c

# Test SIGSEGV (segmentation fault) - default
./test_crash_handler 0

# Test SIGFPE (division by zero)
./test_crash_handler 1

# Test SIGABRT (abort)
./test_crash_handler 2
```

The test program will:
1. Set up crash handlers
2. Trigger a crash after 2 seconds
3. Display crash information to stderr
4. Write crash log to `/tmp/chipmunk-crash-test.log`

## Testing with the Actual Program

To test the crash handler with the actual `analog` program, you can:

### Option 1: Use GDB to trigger a crash

```bash
# Start analog in gdb
gdb ./bin/diglog

# In gdb, set a breakpoint and then cause a crash
(gdb) run -c analog.cnf
(gdb) break main
(gdb) continue
(gdb) call abort()  # This will trigger SIGABRT
```

### Option 2: Use a debugger to inject a crash

```bash
# Run analog and attach with gdb
./bin/analog &
PID=$!
gdb -p $PID
(gdb) call *(int*)0 = 0  # This will cause SIGSEGV
```

### Option 3: Test with known problematic operations

If you know of operations that might cause crashes (e.g., loading corrupted files, invalid circuit configurations), you can test those scenarios.

## What to Look For

When a crash occurs, you should see:

1. **Console output** with:
   - Crash detection message
   - Timestamp
   - Signal type and cause
   - Fault address
   - Stack trace (on Linux)
   - Instructions for reporting

2. **Crash log file** at:
   - `chipmunk-crash.log` in the repo root (if LOGLIB is set)
   - `/tmp/chipmunk-crash.log` (fallback)

## Verifying the Crash Log

After a crash, check the log file:

```bash
# If LOGLIB is set (normal operation)
cat chipmunk-crash.log

# Or check /tmp (fallback location)
cat /tmp/chipmunk-crash.log
```

## Expected Output

The crash handler should produce output like:

```
========================================
CHIPMUNK CRASH DETECTED
========================================
Time: 2024-11-15 14:30:45
Signal: SIGSEGV (Segmentation Fault)
Cause: Memory access violation (invalid pointer, buffer overflow, etc.)
Fault address: 0x12345678

Stack trace (10 frames):
  [ 0] crash_handler+0x123
  [ 1] main+0x456
  ...

To help fix this issue, please:
1. Note what you were doing when the crash occurred
2. Check if you can reproduce the crash
3. Report the issue at: https://github.com/sensorsINI/chipmunk/issues
4. Include this crash information and the crash log file (if created)

Crash log written to: /path/to/chipmunk-crash.log
========================================
```

## Notes

- The crash handler only works on Linux/Unix systems (not OS/2)
- Stack traces require the program to be compiled with `-g -rdynamic` flags
- The crash log is appended to, so multiple crashes will accumulate in the same file
- The program will still terminate after displaying crash information (this is expected)

