# SongSequencer
Make a song by sequencing other sequencers.  Made for the Expert Sleepers Disting NT.

## Overview

SongSequencer is used to sequence other sequencers forming a song.  Think of it as a 8:1 switch for sequencers; up to 8 input sequencers are switched to the master outputs at the right time.

The UI displays 8 steps where each step is assigned an input sequencer, a repeat value, and an on|off switch.   Song Sequencer advances through the steps and outputs pitch, gate, and an assignable control voltage (CV) value from the sequencer of the active step.  The assignable control voltage can be anything but most commonly would be used for something like velocity. This value can be routed to any output.

Song Sequencer uses an input used for the master **Beat (clock)** common across all sequencers.  It is used to count beats and advance to the next sequencer at the right time.  Each sequencer has a **Bars** parameter and a **Beats Per Bar** parameter.   For example, if Bars = 1, and Beats per Bar = 4, Song Sequencer will count Beats from the master beat input until it reaches 4, and then it will advance (switch) to the next sequencer with an ON switch. 

Each step has a **repeat control** allows a sequence to be repeated up to 16 times.  If it is set to zero, the sequencer for that step is run once and then SongSequencer advances to the next step.  If it is set to 1..16 it will repeat that sequencer that number of times.

Each of the 8 steps has a **on | off Switch** that determines if SongSequencer will run that step or not.  Any combination is valid; if all 8 switches are off there is no output of CV, Gate, or assignable CV. 

Each sequencer supports a **Transpose *nput**; the transpose CV value is added to the Pitch CV/

At the end of the sequence, Sound Sequencer issues a **Reset output** that can be routed to the Reset input on a sequencer so that sequencers start on time.

In addition there is a master Reset Input that sends a Reset to all sequencers configured to receive it as well as resetting Song Sequencer to the first step that is ON (it looks from step 1).

## Key Features

- Sequence up to 8 other sequencers
- Sequencer Repeats up to 16 times
- On Off switches for the 8 steps
- Master reset input to reset Song Sequencer and attached sequencers
- Beat input corresponding to the song's tempo used to advance steps at the right time
- Pitch CV Output
- Gate Output
- Assignable CV Output (pass any CV from the input sequencer for a step to an output)

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

These parameters let you control how many beats from the master beat input are used before advancing to the next sequencer.
- Beats per Bar
- Bars

## Installation

Copy SongSequencer.o to the Disting NT's Plugins folder.

## Building

-- Use the Makefile in the repository; you will have to adjust the path the api.h file
-- NB: Uses api version 1.8.  Module developed against firmware v1.9.0

## License

MIT License

## Background

Port from my HighSeq module developed for VCVRack.








