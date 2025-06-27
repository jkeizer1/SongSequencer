# SongSequencer

Make a song by sequencing other sequencers.  Made for the Expert Sleepers Disting NT.

## Overview

Song Sequencer is used to sequence other sequencers forming a song.  Think of it as a 8:1 switch for sequencers; up to 8 input sequencers are switched to the master outputs at the right time.

The **Custom UI** displays 8 song steps in a grid where each step has:

- Assigned Input sequencer (A through H)
- Repeat value (0 through 16)
- On|off step switch
- **Navigate** the grid with the left and right encoders (bottom row, not the top row of Pots)
- **Change** values by pressing and turning the Right Pot (top row); ie. press and hold.

Song Sequencer outputs the following for the current step:

- Pitch (with optional added Transpose CV)
- Gate, 
- Assignable control voltage (CV) (optional) 
- Reset trigger output (optional)

The assignable control voltage can be anything but most commonly would be used for something like velocity. 

## How It Works

You can think of each step as being a part of a song, with an associated input sequencer.

Song Sequencer uses a master **Beat (clock)** input common across all sequencers.  The beat clock is used to advance to the next step by counting beats.

- Each sequencer has a **Bars** parameter and a **Beats Per Bar** parameter.   For example, if Bars = 1, and Beats per Bar = 4, Song Sequencer will count beats from the master beat input until it reaches 4, and then it will advance to the next sequencer with an ON switch. 
- Of course, sequencers can output CV/Gates in any timing not just "on the beat clock". 
- Maximum 256 beats each sequence.

Each step has a **repeat control** allowing a sequence to be repeated up to 16 times on a step.  If it is set to zero, the sequencer for that step is run once and then Song Sequencer advances to the next step.  If it is set to 1..16 it will additionally repeat that sequencer that number of times.

Each of the 8 steps has a **on | off Switch** that determines if Song Sequencer will run that step or not.  Any combination is valid; if all 8 switches are off there is no output of CV, Gate, or assignable CV. 

Each sequencer supports a **Transpose** input; the transpose CV value is added to the Pitch CV output. This is not quantized.

At the end of each sequence, Sound Sequencer issues a **Reset output** that can be routed to the Reset input on the sequencers so that the next sequencer starts on time.

In addition there is a master Reset Input that resets the internal state of SongSequencer, and sends a reset to the next (first) real sequencer.

## Key Features

- Sequence up to 8 other sequencers
- Sequencer repeats up to 16 times
- On Off switches for the 8 steps of the song
- Master reset input to reset Song Sequencer and attached sequencers
- Beat input corresponding to the tempo used to advance steps at the right time
- Pitch CV Output
- Gate Output
- Assignable CV Output (pass any CV from the input sequencer for a step to an output)

## Custom User Interface

### Display Line ONE
TOP LINE
- Shows Bars and Beats per Bar (Bpb) for the active step
- Rep n; shows the current repeat count of the active step
- Bar shows the current bar of the active step
- Beats shows the curent beat within the bar

### Display Line TWO
STEPS TITLES
- Shows the step numbers from 1..8 (bright background)

### Display Lines THREE, FOUR, FIVE
SEQUENCER GRID
- SEQ shows the assigned sequencer A..H for the step
- REP shows the number of repeats for the step (0 means run the step once, 1 means repeat it once, etc.)
- On shows whether the step is on or off ("--"); 

### Navigation
- Turn left encoder to navigate horizontally across steps
- Turn right encoder to navigate vertically across rows (Seq Row - assigned sequencer, REP - # of repeats for the step, On - set the step on or off ("-"))

### Change Values
- Press and turn pot right to change value
- Click it to enter last value used (it is a pot without catch behavior)

### Tricks
- Change SEQ to some unassigned sequencer, and configure its bars/beats per bars: you will get "silence" or a rest step
- Change up the SEQ assignments, repeats, and ON OFF live to vary your song
- Use a sequencer to generate a transpose value as input
- Set the bars/beats per bar to be shorter than your sequencer; you can use the standard parameter screens to dynamically play from 1 to n sequencer steps

## Step Parameters (per 8) : Managed by a custom UI for playability

- Assigned Sequencer (indicated by number 0..7) { to do: show A-H instead }
- Repeat Count
- On or Off Switch (display shows "ON" or "--")

## Sequencer Parameters (per 8 sequencers A .. H)

- CV Input
- Gate Input
- Reset Output
- Transpose Input
- Assignable CV Input

## Sequencer Configuration (per 8 sequencers A .. H)

These parameters let you control how many beats from the master beat input are used before advancing to the next sequencer. Match these setting to the input sequencers. 
- Bars
- Beats per Bar
- Maximums: 16 Bars * 16 Beats per Bar = 256 beats

Example: A typical 16 step sequencer could be set as:
A) 1 Bar, 16 steps per Bar = 16 beats
B) 2 Bars, 8 steps per Bar = 16 beats
C) 4 Bars, 4 steps per Bar = 16 beats
** The important thing as that 16 beats will be counted before Song Sequencer advances to the next step. 
You can, of course, set this up for a 4 step or 8 step sequencer (or whatever).

## Installation

Copy SongSequencer.o to the Disting NT's Plugins folder.

## Building

-- Use the Makefile in the repository; you will have to adjust the path the api.h file
-- NB: Uses api version 1.8.  Module developed against firmware v1.9.0

## License

MIT License

## Background

Port from my HighSeq module developed for VCVRack.








