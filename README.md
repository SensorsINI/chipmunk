# Chipmunk Tools

This repository contains the Chipmunk system tools, originally developed by Dave Gillespie, John Lazzaro, and others.

## Original Source

The original Chipmunk tools are distributed via GitHub Pages:
- **Official Website**: https://john-lazzaro.github.io/chipmunk/
- **Author Contact**: john [dot] lazzaro [at] gmail [dot] com

## License

This software is distributed under the GNU General Public License (GPL) version 1 or later. See the `COPYING` files in the `psys/src/` and `log/src/` directories for the full license text.

## About Chipmunk

The Chipmunk system is a collection of software tools for Unix systems and OS/2, including:

- **Log**: A schematic editor, analog and digital simulator, and netlist generator
- **Analog**: Analog circuit simulation tool
- **Diglog**: Digital circuit simulation tool
- **Loged**: Gate editor for creating custom gate icons
- **View, Until, Wol**: Additional CAD tools

## Modifications in This Repository

This repository includes the following modifications:

- **Wrapper Scripts**: Added wrapper scripts (`analog`, `diglog-wrapper`) that automatically set the `LOGLIB` environment variable to ensure proper configuration file discovery
- **Build Fixes**: Compiled and tested on modern Linux systems with X11

## Building and Installation

### Prerequisites

- ANSI C compiler (typically GCC)
- X11 (R4, R5, or R6)
- **X11 fonts: `xfonts-base`, `xfonts-75dpi`, `xfonts-100dpi`** (required - see Installation Steps)
  - These packages provide the `6x10` and `8x13` fonts that Chipmunk requires
  - Without these fonts, the program will fail with X11 font errors

### Installation Steps

1. **Install required X11 fonts** (required for Ubuntu/WSL2):
   ```bash
   sudo apt-get install xfonts-base xfonts-75dpi xfonts-100dpi
   xset fp rehash
   ```
   
   **Important**: The Chipmunk tools require the X11 fonts `6x10` and `8x13`. These fonts are provided by the packages above. Without them, you will see errors like:
   ```
   X Error of failed request: BadName (named color or font does not exist)
   Major opcode of failed request: 45 (X_OpenFont)
   ```

2. **Verify requirements** (optional but recommended):
   ```bash
   ./check_requirements.sh
   ```
   This script checks for fonts, X11 display, and other requirements.

3. **Build the tools**:
   ```bash
   make
   ```
   Or build manually:
   ```bash
   cd psys/src && make install
   cd ../../log/src && make install
   ```

4. **Run the analog simulator**:
   ```bash
   ./bin/analog
   ```

The wrapper scripts automatically configure the `LOGLIB` environment variable and load the appropriate configuration file (`analog.cnf` for analog mode).

## Usage

### Running Analog Simulator

The `analog` command launches the Log system in analog simulation mode:

```bash
./analog                    # Launch analog simulator
./analog -c custom.cnf      # Use custom configuration file
./analog circuit.lgf        # Open a circuit file
```

### Command Line Options

- `-c <file>`: Specify configuration file (default: `analog.cnf`)
- `-v`: Vanilla LOG mode (no CNF file)
- `-x <display>`: Specify X display name
- `-h <dir>`: Specify home directory
- `file`: Open a circuit file on startup

## Documentation

### Official Documentation

The complete documentation is available on the [official Chipmunk website](https://john-lazzaro.github.io/chipmunk/):

- **[Log Reference Manual](https://john-lazzaro.github.io/chipmunk/document/log/index.html)**: Complete reference for all Log system features
  - [Getting Started](https://john-lazzaro.github.io/chipmunk/document/log/index.html#getting-started)
  - [Circuit Editing](https://john-lazzaro.github.io/chipmunk/document/log/index.html#circuit-editing)
  - [Analog Simulator](https://john-lazzaro.github.io/chipmunk/document/log/index.html#analog-simulator)
  - [Digital Simulator](https://john-lazzaro.github.io/chipmunk/document/log/index.html#digital-simulator)
  - [Plotting Circuits](https://john-lazzaro.github.io/chipmunk/document/log/index.html#plotting-circuits)

### For Analog Users

- **[Postscript Manual](https://john-lazzaro.github.io/chipmunk/document/log/index.html#postscript-manual)**: Guide for analog simulation users
- **[Interactive Lessons](https://john-lazzaro.github.io/chipmunk/document/log/index.html#interactive-lessons)**: Five annotated circuit schematics (`lesson1.lgf` through `lesson5.lgf`) for learning Analog
- **[Pocket Reference](https://john-lazzaro.github.io/chipmunk/document/log/index.html#pocket-reference)**: 28 tips for novice Analog users (see `log/lib/cheat.text`)
- **[Device Model Details](https://john-lazzaro.github.io/chipmunk/document/log/index.html#device-model-details)**: Documentation on FET7 series MOS models and other device models
- **[Simulation Engine Details](https://john-lazzaro.github.io/chipmunk/document/log/index.html#simulation-engine-details)**: Technical documentation on how the simulation engine works
- **[Adding New Gates](https://john-lazzaro.github.io/chipmunk/document/log/index.html#adding-new-gates)**: Guide for adding custom gates to Analog

### Reference Material

- [List of Commands](https://john-lazzaro.github.io/chipmunk/document/log/index.html#list-of-commands)
- [Configuration Files](https://john-lazzaro.github.io/chipmunk/document/log/index.html#configuration-files)
- [Analog Simulator Commands](https://john-lazzaro.github.io/chipmunk/document/log/index.html#analog-simulator-commands)
- [Command-line Options](https://john-lazzaro.github.io/chipmunk/document/log/index.html#command-line-options)

## Attribution

Original authors:
- Dave Gillespie
- John Lazzaro
- Rick Koshi
- Glenn Gribble
- Adam Greenblatt
- Maryann Maher

Maintained under Unix by Dave Gillespie and John Lazzaro.

## Repository Information

This repository is maintained by Tobi Delbruck for the [Sensors Group](https://sensors.ini.ch) at the Inst. of Neuroinformatics (UZH-ETH Zurich). For the original source and official documentation, please visit the [official Chipmunk website](https://john-lazzaro.github.io/chipmunk/).

