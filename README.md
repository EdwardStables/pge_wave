# PGE Wave

This is a VCD wave viewer written in C++ using the olcPixelGameEngine. This viewer is intended to address some of my UX frustrations with GTKWave, but will probably not suit many people's style of working.

Key bindings can be customised in `include/config.h`.

## Screenshots

Rendering single bit and multi-bit waveforms:
![Wave Dislay](images/waves.png)

Toggle-able value overlay on name panel:
![Value Dislay](images/values.png)

Picker for adding additional waveforms:
![Wave Picker](images/wave_picker.png)

## Major Missing Features

This tool is in active development. A subset of missing features that are critical to usability:

- Parsing and displaying a module hierarchy
- A status bar showing things like cursor position and other state variables
- Saving and recalling wave layouts
- Removing (or moving) waves that have been added
- Logical/bus operations to modify signals
- Groupings
- Showing Xs
- Composite types
- Resolution hard coded