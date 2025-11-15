# Chipmunk Analog Circuit Simulator - Quick Help

## Getting Started

### First Steps
1. **Run the simulator**: `./bin/analog`
   - The tutorial circuit (`lesson1.lgf`) opens automatically for new users; open any other file with ./bin/analog file.lgf. 
2. **Get command-line help**: `./bin/analog --help`
3. **In-program help**: Press `?` key or click the HELP button

## Core Interaction Model

### Mode-Based Editing
- **Keyboard** enters modes (e.g., `d` for delete, `m` for move, `c` for configure)
- **Mouse** performs actions within the active mode
- **Ctrl-C** exits any mode and returns to normal

### Mouse Actions
- **Tap** (quick press/release): Rotate gates, configure gates, draw wires
- **Drag** (press + move + release): Move objects, select areas
- **Right button**: Cancel wire-drawing and other simple modes
- **Drag off screen edge**: Delete objects (in delete mode)

### Grid and Connections
- All objects snap to a **grid** for precise alignment
- **Red dots** indicate connection points - align these when connecting gates
- **T-connections** (T-junctions): Automatically connect
- **Crossing wires**: Must be manually soldered (don't connect automatically)

## Essential Keyboard Shortcuts

### Navigation
- **Space**: Refresh screen
- **`<`** / **`>`**: Zoom out / Zoom in
- **Arrow keys**: Scroll the schematic
- **`h`**: Home (return to origin)
- **`G`**: Toggle grid display
- **`A`**: Auto-window (fit circuit to window)

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
- **`f`**: Fast mode

### Getting Help
- **`?`**: Open this help (or press HELP button)
- **`q`**: Exit help mode
- **Ctrl-C**: Cancel current mode

## Common Workflows

### Moving an Object
1. Press `m` (move mode)
2. Press and drag the object with left mouse button
3. Release to place it

### Configuring a Gate
1. Press `c` (configure mode)
2. Tap on the gate you want to configure
3. Use arrow keys to select attributes, left/right to change values
4. Press Ctrl-C when done

### Drawing Wires
1. Tap to start a wire segment
2. Tap again to end the segment and start a new one
3. Press right button to cancel wire-drawing

### Getting Gates from Catalog
1. Press `C` (or click CAT button) to open Gate Catalog
2. Press and drag a gate from the catalog to your schematic
3. Tap on gates in the schematic to rotate them

## Sample Circuits

Interactive tutorial circuits are available in `log/lib/`:
- **`lesson1.lgf`**: First tutorial (opens automatically)
- **`lesson2.lgf`**, **`lesson3.lgf`**, etc.: Additional tutorials

Open a circuit: `./bin/analog log/lib/lesson1.lgf`

## Configuration Files

Default configuration files are in `log/lib/`:
- **`analog.cnf`**: Default analog simulation configuration
- **`log.cnf`**: General LOG configuration
- **`genlog.cnf`**: Generic LOG configuration

Use custom config: `./bin/analog -c log/lib/custom.cnf`

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
| 1 | Press the Control-C key to get out of any mode |
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

