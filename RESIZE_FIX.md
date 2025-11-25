# Window Resize Hang Fix

## Problem Summary
Window resizing in the analog application caused severe CPU busy-wait loops and hangs, requiring Ctrl-Z and kill.

## Root Causes (Diagnosed via strace)

### 1. X11 Event Queue Recycling
Analysis of `resize_trace.log` revealed:
- 33,839 `EAGAIN` responses out of 33,923 `recvmsg()` calls (99.75% busy-wait!)
- Tight loop pattern: `poll() → recvmsg(data) → recvmsg(EAGAIN) → recvmsg(EAGAIN)...`
- ConfigureNotify events were being put back on the X11 event queue with `XPutBackEvent()`
- Event handlers in infinite loops (`for(;;)`) would immediately fetch the same event again
- This created an infinite event recycling loop consuming 100% CPU

### 2. Empty Busy-Wait Loop in inkey2()
The input function had an **empty busy-wait loop**:
```c
do {
} while (!pollkbd2());  // Spins until input available!
```
This continuously called `m_pollkbd()` → `XCheckMaskEvent()` → `recvmsg(EAGAIN)` in a tight loop.

### 3. No Throttling on Event Polling
`m_pollkbd()` was called in tight loops throughout the application (from `pen()` function), 
causing continuous X11 socket polling even when no events were available.

## The Fixes

### Fix 1: Remove X11 Event Recycling (psys/src/mylib.c)

Changed all ConfigureNotify event handlers to **consume** events instead of putting them back:

**Before (caused busy loop):**
```c
case ConfigureNotify:
    update_window_size(...);
    XPutBackEvent(m_display, &event);  // ← Put event back = infinite loop!
    return(1);
```

**After (consumes event):**
```c
case ConfigureNotify:
    m_across = event.xconfigure.width - 1;
    m_down = event.xconfigure.height - 1;
    update_window_size(...);
    // Event consumed - don't put back to avoid busy loop
    return(1);
```

**Functions Modified in mylib.c:**
1. `m_pollkbd()` - Non-blocking keyboard poll (line ~5434)
2. `m_inkey()` - Blocking keyboard input (line ~5650)
3. `m_inkeyn()` - Non-blocking keyboard input (line ~5857)
4. `m_testkey()` - Keyboard peek (line ~6058)

### Fix 2: Add Event Polling Throttle (psys/src/mylib.c)

Added intelligent throttling to `m_pollkbd()` to prevent busy-wait when no events are available:

**Before (continuous polling):**
```c
boolean m_pollkbd() {
  if (!XCheckMaskEvent(...))
    return(0);  // Returns immediately, causing tight loop
  // ... process event
}
```

**After (throttled when idle):**
```c
boolean m_pollkbd() {
  if (!XCheckMaskEvent(...)) {
    // No events. If we checked recently (< 10ms ago), sleep to prevent busy-wait
    if (time_since_last_check < 10000us) {
      usleep(10000);  // Sleep 10ms
    }
    return(0);
  }
  // Event available - process immediately (no throttling)
}
```

This prevents the `recvmsg() → EAGAIN` busy-loop while maintaining responsiveness.

### Fix 3: Remove Empty Busy-Wait Loop (log/src/log.c)

Removed the unnecessary busy-wait loop in `inkey2()`:

**Before (busy-wait):**
```c
Static Char inkey2() {
  do {
    do {
    } while (!pollkbd2());  // ← Empty busy-wait loop!
    realkey = nk_getkey();
  } while ((unsigned char)ch == 251);
}
```

**After (blocking wait):**
```c
Static Char inkey2() {
  do {
    // nk_getkey() blocks until input available - no busy-wait needed!
    realkey = nk_getkey();
  } while ((unsigned char)ch == 251);
}
```

Since `nk_getkey()` is `m_inkey()` which **blocks** waiting for input, the busy-wait loop was 
completely unnecessary and caused continuous polling.

### Fix 4: Fix Display Refresh (log/src/loged.c)

Added window refresh after resize to prevent corrupted display:

**Before (window not redrawn):**
```c
case 251:             /* X ConfigureNotify (resize) event */
  autoscale(1L);
  break;
```

**After (forces refresh):**
```c
case 251:             /* X ConfigureNotify (resize) event */
  autoscale(1L);
  needrefr = true;  /* Force refresh after resize */
  break;
```

### Fix 5: Add Layout Update and Event Consumption (log/src/log.c)

The main LOG program (which analog uses) lacked any handling for ConfigureNotify events. This caused:

1. **Layout variables never updated** - Variables like `across`, `down`, `baseline`, `line1`, `line2` that control menu positioning were set once at init but never updated on resize
2. **No event consumption** - Resize events (code 251) were not properly consumed

**The Fix:**

Added a new `update_screen_layout()` function that recalculates all layout variables:

```c
Static Void update_screen_layout()
{
  across = m_across;
  down = m_down;
  baseline = down - 53;   /* Position of baseline on screen */
  line1 = down - 43;      /* Position of first text line in menu */
  line2 = down - 23;      /* Position of second text line in menu */
  histdown = down - 26;
  histdivsacross = (double)(across - histleft) / histdivision;
  kindgroupsize = (across - 160) / kindgroupspacing;
  // ... and all other layout-dependent variables
}
```

Modified `inkey2()` to silently consume resize events (without calling graphics functions):

```c
Static Char inkey2()
{
  do {
    realkey = nk_getkey();  // Blocks until input available
    ch = realkey;
    /* Resize events (251) are consumed silently */
    /* Layout update happens in pen() function */
  } while ((unsigned char)ch == 251);
  
  return ch;  /* Return first non-resize key */
}
```

Modified `pen()` to check for window size changes and update layout:

```c
Static Void pen()
{
  // ... existing code ...
  
  /* Update layout if window size changed */
  if (across != m_across || down != m_down) {
    update_screen_layout();
  }
  
  // ... rest of function
}
```

**Key Design Decisions:**

1. **No graphics calls from inkey2()** - Calling `clearscreen()` from `inkey2()` caused deadlocks. Layout updates happen separately in `pen()`.
2. **Loop instead of recursion** - Consumes all pending resize events in one go, avoiding stack overflow.
3. **Deferred layout update** - Layout variables are updated in `pen()` (called frequently in main loop) rather than in the input handler, avoiding timing issues.

## How It Works Now

1. **User resizes window** → X11 generates ConfigureNotify events
2. **Event handler receives event (mylib.c)** → Updates `m_across`, `m_down` immediately
3. **Event is consumed** → NOT put back on queue (prevents event recycling)
4. **Returns code 251** → Signals application of resize
5. **Application consumes event (log.c)** → `inkey2()` silently consumes code 251
6. **Layout update (log.c)** → `pen()` detects size change, calls `update_screen_layout()`
7. **Screen refresh** → Application's natural redraw cycle updates display
8. **Idle throttling** → When no events available, `m_pollkbd()` sleeps 10ms to prevent busy-wait
9. **Geometry saved** → Final size written to `~/.chipmunk` on exit

## Result

- ✅ **Smooth window resizing** - No hang or busy-wait loop
- ✅ **Low CPU usage** - Throttling prevents 100% CPU consumption during idle
- ✅ **Window dimensions update in real-time** - Immediate visual feedback
- ✅ **Menu positioning correct** - Layout variables updated properly after resize
- ✅ **Automatic viewport adjustment** - Content scales to new window size
- ✅ **Proper display refresh** - Window redraws correctly after resize
- ✅ **Simulation continues** - Clock continues to flash during and after resize
- ✅ **Window geometry persisted** - Final size saved to ~/.chipmunk on exit
- ✅ **Multiple resizes work** - No hang on subsequent resizes

## Testing

```bash
cd /home/tobi/chipmunk
./bin/analog lessons/nfet.lgf
# Resize the window - should be smooth, no hang!
```

## Technical Notes

### Why Multiple Fixes Were Needed

1. **Event Recycling** - `XPutBackEvent()` with `XCheckMaskEvent()` in `for(;;)` loops created infinite event recycling
2. **Busy-Wait in Input** - Empty `while (!pollkbd2())` loop continuously polled for input
3. **No Throttling** - `m_pollkbd()` was called in tight loops from `pen()` with no rate limiting
4. **Missing Layout Updates** - Screen layout variables were never recalculated after resize

Each fix addresses a different layer of the problem:
- **mylib.c** - Low-level X11 event handling and polling throttle
- **log.c** - Application-level event consumption and layout management
- **loged.c** - Display refresh triggering

### Design Principles Applied

1. **Event consumption** - Each event processed exactly once, never recycled
2. **Separation of concerns** - Input handling separate from graphics/layout updates
3. **Intelligent throttling** - Only throttle when idle, process events immediately when available
4. **Deferred updates** - Layout recalculation happens in main loop, not input handler
5. **Blocking over busy-wait** - Use blocking calls (`m_inkey()`) instead of polling loops

### Performance Impact

- **Before**: 99.75% EAGAIN rate (33,839 of 33,923 calls), 100% CPU usage during resize
- **After**: Events processed immediately, ~10ms sleep when idle, minimal CPU usage

---
**Fixed:** November 25, 2025
**Diagnosed with:** strace -f -o resize_trace.log ./bin/analog
**Multiple iterations:** Required 5+ rebuild cycles to identify and fix all busy-wait sources

