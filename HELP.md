# Chipmunk Analog Circuit Simulator - Quick Help

## Getting Started

### First Steps
1. **Run the simulator**: `./bin/analog`
   - The tutorial circuit (`lesson1.lgf`) opens automatically for new users; open any other file with `./bin/analog file.lgf`. 
   - Default window size is 1280x960 (automatically saved and restored between sessions)
   - To override, add to `~/.Xresources`: `mylib.geometry: 1600x1200+0+0`
2. **Get command-line help**: `./bin/analog --help`
3. **In-program help**: Press `?` key or click the HELP button

## Core Interaction Model

### Mode-Based Editing
- **Keyboard** enters modes (e.g., `d` for delete, `m` for move, `c` for configure, '/' for copy)
- **Mouse** performs actions within the active mode
- **Esc** or **Ctrl-C** exits any mode and returns to normal

### Mouse Actions
- **Tap** (quick press/release): Rotate gates, configure gates, draw wires
- **Drag** (press + move + release): Move objects, select areas
- **Right button**: Cancel wire-drawing and other simple modes
- **Drag off screen edge**: Delete objects (but only in delete mode)

### Grid and Connections
- All objects snap to a **grid** for precise alignment
- **Red dots** indicate connection points - align these when connecting gates
- **T-connections** (T-junctions): Automatically connect
- **Crossing wires**: Must be manually soldered (don't connect automatically)

## Essential Keyboard Shortcuts

### Navigation
- **Space**: Refresh screen
- **`<`** / **`>`**: Zoom out / Zoom in (7 levels: 1x, 2x, 3x, 5x, 8x, 12x, 20x)
- **Arrow keys**: Scroll the schematic
- **`h`**: Home (return to origin and reset zoom to default 5x)
- **`F`**: Fit - automatically zoom and center all objects to fill the window
- **`G`**: Toggle grid display
- **`A`**: Auto-window mode (toggles automatic window raising for small screens; does NOT fit circuit to screen)

### Editing
- **`C`**: Open Gate Catalog
- **`c`**: Configure mode (configure gates)
- **`d`**: Delete mode
- **`m`**: Move mode
- **`/`**: Copy mode
- **`*`**: Paste
- **`.`**: Probe mode (measure signals)

### Simulation
- **`s`**: Open Scope screen
- **`0`** or **`R`**: Reset simulator (time=0)
- **`f`**: Fast mode (speeds up simulation)

### Getting Help
- **`?`**: Open this help (or press HELP button)
- **`q`**: Exit help mode
- **Esc** or **Ctrl-C**: Cancel current mode

## Common Workflows

### Drawing Wires
1. Tap to start a wire segment
2. Tap again to end the segment and start a new one
3. Press right button to cancel wire-drawing
4. Note: First click after window focus is ignored to prevent accidental wire-drawing
5. Note: Esc and Ctrl-C work to exit other modes, but right-click is the intended way to cancel wire-drawing

### Moving an Object
1. Press `m` (move mode)
2. Press and drag the object with left mouse button
3. Release to place it

### Configuring a Gate
1. Press `c` (configure mode)
2. Tap on the gate you want to configure
3. Use arrow keys to select attributes, left/right to change values
4. Press Ctrl-C when done


### Getting Gates from Catalog
1. Press `C` (or click CAT button) to open Gate Catalog
2. Press and drag a gate from the catalog to your schematic
3. Tap on gates in the schematic to rotate them

## Sample Circuits

Interactive tutorial circuits are available in the `lessons/` directory:
- **`lesson1.lgf`**: First tutorial (opens automatically)
- **`lesson2.lgf`**, **`lesson3.lgf`**, **`lesson4.lgf`**, **`lesson5.lgf`**: Additional tutorials
- **`nfet.lgf`**: NFET transistor characterization example

Open a circuit: `./bin/analog lessons/lesson1.lgf`

## Configuration Files

Default configuration files are in `log/lib/`:
- **`analog.cnf`**: Default analog simulation configuration
- **`log.cnf`**: General LOG configuration
- **`genlog.cnf`**: Generic LOG configuration

Use custom config: `./bin/analog -c log/lib/custom.cnf`

## Command-Line Options

The `analog` command supports the following options:

- **`-h, --help`**: Show help message and exit
- **`-c <file>`**: Specify configuration file (default: `analog.cnf` if not specified)
- **`-v`**: Vanilla LOG mode (no CNF file loaded)
- **`-x <display>`**: Specify X display name (e.g., `:0.0` or `hostname:0`)
- **`-z [<file>]`**: Enable trace mode (debugging). If file is specified, trace output goes to that file (default: `trace.text`)
- **`-d <file>`**: Specify dump file for debug output
- **`-t <file>`**: Specify trace file for trace output (alternative to `-z <file>`)
- **`-r <tool>`**: Run a specific tool immediately on startup (non-interactive mode)
- **`-h <dir>`**: Specify home directory for searching gate files, config files, etc. (Note: conflicts with `-h` for help; use `--help` for help)

**Examples:**
```bash
./bin/analog                           # Start with lesson1.lgf (for beginners)
./bin/analog lessons/lesson1.lgf       # Open a specific circuit file
./bin/analog -c log/lib/custom.cnf     # Use custom configuration
./bin/analog -v                        # Start without any configuration file
./bin/analog -z trace.txt circuit.lgf  # Enable trace mode with output file
./bin/analog -x :1.0                   # Use display :1.0
```

### How `-c` Option Works

According to the [official documentation](https://john-lazzaro.github.io/chipmunk/document/log/ref/ref2.html):

**When `-c` is specified:**
- The specified configuration file **completely replaces** the default loading behavior
- Default files like `log.cnf` or `analog.cnf` are **NOT** automatically loaded
- Only the file specified with `-c` is read

**When `-c` is NOT specified (default behavior):**
- LOG looks for a configuration file in this order:
  1. Program-specific name: `analog.cnf` (if running `analog`) or `diglog.cnf` (if running `diglog`)
  2. Fallback: `log.cnf`
  3. Search order: current directory → home directory (`~/log`) → `/lib/log` (or `$LOGLIB`)

**Recommended approach for custom configurations:**
To view a legacy design, it might be necessary to provide a custom .cnf file.

Instead of copying system CNF files (which may change), create your own CNF file that uses `INCLUDE` to inherit from the base configuration, then adds your customizations:

```cnf
{ Custom configuration }
include analog.cnf              { Inherit base configuration and gates, e.g. GND, TO, FROM, nfet4, and at least some of the original PCMP pads}
gates + gates.gate              { Add your custom gates file }
menu gate1 gate2 gate3          { Customize menu gates }
```

**Note on INCLUDE paths:**
- **Relative paths** (e.g., `genlog.cnf`) are resolved relative to the current working directory, which is `log/lib` after the wrapper script runs. This is the recommended approach.
- **Absolute paths** (starting with `/`) will work but are not portable across systems (e.g., `/home/user/chipmunk/log/lib/genlog.cnf`).
- Paths are searched in this order: current directory → launch directory → home directory (`~/log`) → LOGLIB directory (`log/lib`).

**Note**: The wrapper script (`bin/analog`) automatically loads `analog.cnf` by default. To use a completely different configuration, specify it with `-c` or use `-v` for vanilla mode (no config file).

## Environment Variables

Chipmunk-specific environment variables that affect behavior:

- **`CHIPMUNK_LAUNCH_DIR`**: Automatically set by the `bin/analog` wrapper script to your current working directory. Used to resolve relative file paths for `:load` and `:save` commands, ensuring files are loaded/saved relative to where you launched the program, not the internal working directory.

- **`CHIPMUNK_MODE`**: Automatically set to `analog` by the wrapper script. Affects window naming (main window shows "analog" instead of "log").

- **`CHIPMUNK_DEBUG_ESC`**: Set to `1` to enable debug logging for Escape/Ctrl-C key detection. Useful for troubleshooting keyboard input issues.

- **`CHIPMUNK_DEBUG_ESC_FILE`**: Set to a file path to redirect debug output (from `CHIPMUNK_DEBUG_ESC`) to a file instead of stderr. Example: `CHIPMUNK_DEBUG_ESC_FILE=debug.log`.

**Note**: The wrapper script (`bin/analog`) automatically sets `CHIPMUNK_LAUNCH_DIR` and `CHIPMUNK_MODE`. You typically don't need to set these manually unless running `diglog` directly.

## Complete Documentation

For comprehensive documentation, tutorials, and advanced features:

**📖 [Full Documentation](https://john-lazzaro.github.io/chipmunk/document/log/index.html)**

The complete documentation includes:
- Detailed user manual
- Interactive lesson descriptions
- Advanced simulation features
- Gate library documentation
- SPICE netlist conversion
- Troubleshooting guide

## Quick Reference Cheat Sheet

Based on `log/lib/cheat.text` - Handy Tips for Using AnaLOG:

| # | Tip |
|---|-----|
| 0 | Type "analog" to run the AnaLOG simulator |
| 1 | Press Esc or Control-C to get out of any mode |
| 2 | Press the "q" key to get out of the Help command |
| 3 | Press the space bar to refresh the screen |
| 4 | PRESS and DRAG with the left button to move things |
| 5 | Drag off the edge of the screen to delete things |
| 6 | TAP to rotate and configure gates, and to draw wires |
| 7 | Press right button to cancel wire-drawing and other simple modes |
| 8 | Make sure the red dots are aligned when connecting gates together |
| 9 | Touch CAT to see the Gate Catalog; press and drag to get gates |
| 10 | T-connections always connect; crossing wires must be soldered by hand |
| 11 | Press < and > to zoom, use the arrow keys to scroll |
| 12 | Change ROT or MIR to CNFG, then touch gates, to configure them |
| 13 | Use up/down arrows to select an attribute, left/right arrows to change |
| 14 | Select RESET from Misc to reset the simulator to time=0 |
| 15 | Use "terminals" (TO or FROM arrows) to name signals for Scope |
| 16 | Select Scope in Misc for Scope screen, or press "s" key |
| 17 | To display a signal on the Scope, just type its name |
| 18 | Touch Config or a signal name to adjust display parameters |
| 19 | "On Reset" mode: when scope runs out of data points, it stops |
| 20 | "Continuous" mode: when scope runs out, it eats the oldest points |
| 21 | To plot currents, connect a TO arrow to the "hook" of an ISCOPE |
| 22 | Press/drag in the traces to measure absolute/delta/value information |
| 23 | Select Delete in Editing menu, then drag a rectangle to delete areas |
| 24 | Use Delete + Paste to move, Copy + Paste to copy |
| 25 | DO NOT use the Open or Close commands in the Editing menu |
| 26 | Use Save and Load in Misc to save or load your circuit |
| 27 | Save renames the old version to ".lfo", for backups |
| 28 | Press "p" to get ready to plot the circuit on the plotter |

**Source**: `log/lib/cheat.text` (original cheat sheet file)

## Additional Resources

- **README**: See [README.md](README.md) for installation, building, and more details
- **Version**: 6.0.0 (Base LOG: 5.66)

---

**Note**: This interface uses a custom design from the 1990s. See README.md for planned UI modernization.

