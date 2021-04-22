# OSARA: Open Source Accessibility for the REAPER Application

- Author: James Teh &lt;jamie@jantrid.net&gt; & other contributors
- Copyright: 2014-2021 NV Access Limited, James Teh & other contributors
- License: GNU General Public License version 2.0

OSARA is a [REAPER](http://www.reaper.fm/) extension which aims to make REAPER accessible to screen reader users.
It was heavily inspired by and based on the concepts of the ReaAccess extension, but was created as a potential replacement because ReaAccess seems to have been abandoned and was not developed openly.
It runs on both Windows and Mac, though it is currently very experimental on Mac ad is not yet fully functional.

Features:

- Reports information about tracks when you navigate to them
- Reports information concerning track folders
- Reports adjustments to track mute, solo, arm, input monitor, phase and volume
- Reports information about items when you navigate to them
- Reports the edit cursor position when you move by pixel, measure, beat or to the start or end of the project
- Provides access to various context menus for tracks, items and the time ruler
- Reports track envelope selection
- Reports markers when you navigate to them
- Facility to adjust of automatable FX parameters
- Ability to watch and report track peak meters
- Noncontiguous selection of tracks/items
- Navigation and selection of chords and notes in the MIDI Editor

## Requirements
OSARA requires REAPER 6.09 or later.
The [SWS/S&M EXTENSION](http://www.sws-extension.org/) is highly recommended and OSARA supports several useful actions from this extension.

It has only been tested with the NVDA and VoiceOver screen readers.
However, on Windows, OSARA uses Microsoft Active Accessibility (MSAA) and UI Automation (UIA) to communicate information, so it should work with any screen reader which supports this correctly.

## Download and Installation
You can download the latest OSARA installer from the [OSARA Development Snapshots](https://osara.reaperaccessibility.com/snapshots/) page.

### Windows
Once you have downloaded the installer, simply run it and follow the instructions.

Note that if you previously copied the OSARA extension into REAPER's program directory manually (before the installer became available), you must remove this first.
The installer installs the extension into your user configuration, not the program directory.

By default, the OSARA key map will be installed, completely replacing your existing key map.
If you do not wish this to occur, you can uncheck the "Replace existing key map with OSARA key map" option.
If the installer does replace the key map, a backup of your existing key map will be made in Reaper's KeyMaps folder.

### Mac
Because OSARA is an extension (not a standalone application) and also needs to install a key map, the installation process is a little different to most Mac applications.
Please follow these instructions to install it:

1. Open the OSARA disk image file you downloaded.
2. Open the "Install OSARA.command" file.
 On macOS Catalina and later, you have to choose open from the context menu, accessed with VO+Shift+M.
3. Follow the instructions.
 If you wish to replace the existing key map with the OSARA key map (which is recommended), answer yes when prompted.
  A backup of your existing key map will be made in Reaper's KeyMaps folder. 
4. The installer leaves a terminal window open.
 It can be closed with Command+Q.
5. Press command+e to eject the disk image.

### Key Map
Even if you chose not to replace your existing key map with the OSARA key map, the OSARA key map will be copied into your REAPER "KeyMaps" folder so you can import it manually from the Actions dialog later if you wish.
This is particularly useful if you wish to merge the key map with your existing key map, rather than replacing it.
Note that any keyboard commands described in this document assume you are using the OSARA key map.

For users who previously used ReaAccess, the OSARA key map is similar to that provided by ReaAccess, though there are some differences.
You can see the full key map by selecting Key bindings and mouse modifiers from the Help menu.

## Additional Documentation
The sections below document all functionality specific to OSARA.
However, they do not attempt to explain how a screen reader user can best understand and use REAPER.
REAPER is a fairly complex application and most of the existing documentation is very much targeted at sighted users.
For more complete, in-depth documentation and guides on using REAPER with OSARA, please consult the [REAPER Accessibility Wiki](https://reaperaccessibility.com/).

## Usage

### Supported REAPER and Extension Actions
OSARA supports reporting of information for the following actions.
Most of these are actions built into REAPER, but a few are very useful actions from the SWS extension.

#### Track Navigation/Management
- Track: Go to next track: DownArrow
- Track: Go to previous track: UpArrow
- Track: Go to next track (leaving other tracks selected): Shift+DownArrow
- Track: Go to previous track (leaving other tracks selected): Shift+UpArrow
- Track: Insert new track
- Track: Cycle track folder state: Shift+Enter
- Track: Cycle track folder collapsed state: Enter
- SWS: Select nearest next folder: Alt+PageDown
- SWS: Select nearest previous folder: Alt+PageUp
- Track: Remove tracks
- Track: Select all tracks

#### Adjusting Track Parameters
- Track: Toggle mute for selected tracks
- Track: Mute/unmute tracks: F5
- Track: Solo/unsolo tracks: F6
- Track: Toggle record arm for selected tracks
- Toggle record arming for current (last touched) track: F7
- Track: Cycle track record monitor:F8
- Track: Invert track phase: F9
- Track: Toggle FX bypass for current track: B
- Track: Toggle FX bypass for master track: Shift+B
- Track: toggle FX bypass on all tracks: Alt+Shift+B
- Track: Nudge track volume up: Alt+UpArrow
- Track: Nudge track volume down: Alt+DownArrow
- (SWS extension) Xenakios/SWS: Nudge volume of selected tracks up: Alt+Shift+UpArrow
- (SWS extension) Xenakios/SWS: Nudge volume of selected tracks down: Alt+Shift+DownArrow
- Track: Nudge master track volume up: Shift+F12
- Track: Nudge master track volume down: Shift+F11
- Track: Nudge track pan left: Alt+Leftarrow
- Track: Nudge track pan right: Alt+Rightarrow
- Master track: Toggle stereo/mono (L+R): shift+f9
- Monitoring FX: Toggle bypass: Control+Shift+B
- Track: Unmute all tracks: alt+f5
- Track: Unsolo all tracks: alt+f6
- Track: Unarm all tracks for recording: alt+f7
- Track: Toggle track solo defeat: control+win+f6

#### Edit Cursor Movement
- View: Move cursor left one pixel: Leftarrow
- View: Move cursor right one pixel: Rightarrow
- Transport: Go to start of project: Control+Home
- Transport: Go to end of project: Control+End
- Move edit cursor to start of next measure: Alt+End
- Move edit cursor to start of current measure: Alt+Home
- Go forward one measure: PageDown
- Go back one measure: PageUp
- Go forward one beat: Control+PageDown
- Go back one beat: Control+PageUp
- View: Move cursor right to grid division: Alt+Shift+Rightarrow
- View: Move cursor left to grid division: Alt+Shift+Leftarrow

#### Items
- Item navigation: Select and move to previous item: control+leftArrow
- Item navigation: Select and move to next item: control+rightArrow
- Item: Split items at edit or play cursor: S
- Item: Split items at time selection: Shift+S
- Item: Remove items
- Item edit: Move items/envelope points right: . or NumPad6
- Item edit: Move items/envelope points left: , or NumPad4
- Item edit: Grow left edge of items: Control+, or Control+NumPad4
- Item edit: Shrink left edge of items: Control+. or Control+NumPad6
- Item edit: Shrink right edge of items: Alt+, or Alt+NumPad4
- Item edit: Grow right edge of items: Alt+. or Alt+NumPad6
- Item: go to next stretch marker: Control+'
- Item: go to previous stretch marker: Control+;
- Item: remove stretch marker at current position
- Item navigation: Move cursor left to edge of item: Control+Shift+,
- Item navigation: Move cursor right to edge of item: Control+Shift+.
- Item: Select all items in track: Control+Alt+A
- Item: Select all items on selected tracks in current time selection: Alt+Shift+A
- Item grouping: Group items: control+g
- Item grouping: Remove items from group: control+shift+g
- Item grouping: Select all items in groups: Shift+G
- Item: Select all items in current time selection: Control+Shift+A
- (SWS extension) Xenakios/SWS: Select items under edit cursor on selected tracks: Shift+A
- Item properties: Toggle mute: Control+F5
- Item properties: Toggle solo exclusive: Control+F6
- Item properties: Normalize items: Control+Shift+N
- Item properties: Normalize multiple items to common gain: Shift+N
- Item: Open associated project in new tab
- Item: Nudge items volume +1dB
- Item: Nudge items volume -1dB
- Xenakios/SWS: Nudge item volume down: Control+Shift+DownArrow
- Xenakios/SWS: Nudge item volume up: Control+Shift+UpArrow
- Item: Set item end to source media end
- Item: Set cursor to next take marker in selected items
- Item: Set cursor to previous take marker in selected items
- Item: Delete take marker at cursor
- Item: Delete all take markers

#### Takes
- Take: Switch items to next take: T
- Take: Switch items to previous take: Shift+T
- Item properties: Set take channel mode to normal: Control+F7
- Item properties: Set take channel mode to mono (downmix): Control+F8
- Item properties: Set take channel mode to mono (left): Control+F9
- Item properties: Set take channel mode to mono (right): Control+F10
- Take: Nudge active takes volume +1dB
- Take: Nudge active takes volume -1dB
- Xenakios/SWS: Nudge active take volume up: Control+UpArrow
- Xenakios/SWS: Nudge active take volume down: Control+DownArrow

#### Markers and Regions
- Markers: Go to previous marker/project start: ;
- Markers: Go to next marker/project end: '
- Markers: Delete marker near cursor: 
- Markers: Delete region near cursor
- Markers: Go to marker 01: 1
- Markers: Go to marker 02: 2
- Markers: Go to marker 03: 3
- Markers: Go to marker 04: 4
- Markers: Go to marker 05: 5 
- Markers: Go to marker 06:6
- Markers: Go to marker 07: 7
- Markers: Go to marker 08: 8
- Markers: Go to marker 09: 9
- Markers: Go to marker 10: 0
- Markers: Insert marker at current position: m
- Regions: Go to region 01 after current region finishes playing (smooth seek): Alt+1
- Regions: Go to region 02 after current region finishes playing (smooth seek): Alt+2
- Regions: Go to region 03 after current region finishes playing (smooth seek): Alt+3
- Regions: Go to region 04 after current region finishes playing (smooth seek): Alt+4
- Regions: Go to region 05 after current region finishes playing (smooth seek): Alt+5
- Regions: Go to region 06 after current region finishes playing (smooth seek): Alt+6
- Regions: Go to region 07 after current region finishes playing (smooth seek): Alt+7
- Regions: Go to region 08 after current region finishes playing (smooth seek): Alt+8
- Regions: Go to region 09 after current region finishes playing (smooth seek): Alt+9
- Regions: Go to region 10 after current region finishes playing (smooth seek): Alt+0
- Markers: Insert region from time selection: shift+r

#### Time Signature/Tempo Markers
- Move edit cursor to previous tempo or time signature change: Shift+;
- Move edit cursor to next tempo or time signature change: Shift+'
- Markers: Delete time signature marker near cursor

#### Automation
- Item edit: Move items/envelope points up one track/a bit: NumPad8
- Item edit: Move items/envelope points down one track/a bit: NumPad2
- (SWS extension) SWS/FNG: Move selected envelope points up: Alt+NumPad8
- (SWS extension) SWS/FNG: Move selected envelope points down: Alt+NumPad2
- Envelope: Delete all selected points: Control+Shift+Delete
- Envelope: Delete all points in time selection: Alt+Shift+Delete
- Envelope: Insert new point at current position (remove nearby points): Shift+E
- Envelope: Insert new point at current position (do not remove nearby points)
- Envelope: Select all points
- Global automation override: All automation in latch mode
- Global automation override: All automation in latch preview mode
- Global automation override: All automation in read mode
- Global automation override: All automation in touch mode
- Global automation override: All automation in trim/read mode
- Global automation override: All automation in write mode
- Global automation override: Bypass all automation
- Global automation override: No override (set automation modes per track)
- Track: Toggle track volume envelope visible: Control+Alt+V
- Track: Toggle track pan envelope visible: Control+Alt+P
- FX: Show/hide track envelope for last touched FX parameter

#### Zoom
- Zoom out horizontal: - or NumPad-
- Zoom in horizontal: = or NumPad+

#### Options
- Options: Cycle ripple editing mode: Alt+Shift+P
- Options: Toggle metronome: Control+Shift+M
- Options: Toggle auto-crossfade on/off: alt+Shift+x
- Options: Toggle locking: l
- (SWS extension) SWS/BR: Options - Cycle through record modes: Alt+\
- Options: Solo in front: Control+Alt+F6
- Pre-roll: Toggle pre-roll on record: Shift+`
- SWS/AW: Toggle count-in before recording: Control+Shift+`
- Transient detection sensitivity: Increase: alt+shift+pageUp
- Transient detection sensitivity: Decrease: alt+shift+pageDown
- Transient detection threshold: Increase: control+shift+pageUp
- Transient detection threshold: Decrease: control+shift+pageDown
- Options: Envelope points move with media items

#### Undo
- Edit: Undo: Control+Z
- Edit: Redo: Control+Shift+Z

#### Transport
- Transport: Toggle repeat: Control+R
- Transport: Increase playrate by ~6% (one semitone): Control+Shift+=
- Transport: Decrease playrate by ~6% (one semitone): Control+Shift+-
- Transport: Increase playrate by ~0.6% (10 cents): Shift+=
- Transport: Decrease playrate by ~0.6% (10 cents): Shift+-
- Transport: Set playrate to 1.0
- Transport: Toggle preserve pitch in audio items when changing master playrate

#### Selection
- Time selection: Set start point: [
- Time selection: Set end point: ]
- Loop points: Set start point: Alt+Shift+[
- Loop points: Set end point: Alt+Shift+]
- Go to start of loop: Alt+Shift+Home
- Go to end of loop: Alt+Shift+End
- Time selection: Remove contents of time selection (moving later items)
- Time selection: Remove time selection and loop point selection: Escape
- Unselect all tracks/items/envelope points: Shift+Escape
- Time selection: Nudge left edge left: Control+[
- Time selection: Nudge left edge right: Control+]
- Time selection: Nudge right edge left: Alt+[
- Time selection: Nudge right edge right: Alt+]
- Time selection: Nudge left: Control+Alt+[
- Time selection: Nudge right: Control+Alt+]
- Time selection: Shift left (by time selection length): Shift+[
- Time selection: Shift right (by time selection length): Shift+]
- Go to start of time selection: home
- Go to end of time selection: end

#### Clipboard
- Edit: Cut items/tracks/envelope points (depending on focus) ignoring time selection: Control+X
- Edit: Cut items/tracks/envelope points (depending on focus) within time selection, if any (smart cut): Control+Shift+X
- Edit: Copy items/tracks/envelope points (depending on focus) ignoring time selection: Control+C
- Edit: Copy items/tracks/envelope points (depending on focus) within time selection, if any (smart copy): Control+Shift+C
- Item: Paste items/tracks: Control+V
- Select all items/tracks/envelope points (depending on focus): control+a

#### View
- View: Toggle master track visible: Control+Alt+Shift+M

#### Grid
- Grid: Set to 1: control+shift+1
- Grid: Set to 1/2: control+shift+2
- Grid: Set to 1/32: control+shift+3
- Grid: Set to 1/4: control+shift+4
- Grid: Set to 1/6 (1/4 triplet): control+shift+5
- Grid: Set to 1/16: control+shift+6
- Grid: Set to 1/24 (1/16 triplet): control+shift+7
- Grid: Set to 1/8: control+shift+8
- Grid: Set to 1/12 (1/8 triplet): control+shift+9
- Grid: Set to 1/64

#### Project Tabs
- Close current project tab: control+f4
- New project tab: alt+shift+n
- New project tab (ignore default template)
- Next project tab: control+tab
- Previous project tab: control+shift+tab

#### Tempo
- Tempo: Decrease current project tempo 01 BPM: Alt+-
- Tempo: Increase current project tempo 01 BPM: alt+=
- Tempo: Decrease current project tempo 0.1 BPM
- Tempo: Decrease current project tempo 10 BPM
- Tempo: Decrease current project tempo 10 percent
- Tempo: Decrease current project tempo 50 percent (half)
- Tempo: Increase current project tempo 0.1 BPM
- Tempo: Increase current project tempo 10 BPM
- Tempo: Increase current project tempo 10 percent
- Tempo: Increase current project tempo 100 percent (double)
- SWS/BR: Options - Toggle "Playback position follows project timebase when changing tempo": Alt+BackSpace

#### MIDI Editor
- View: Go to start of file: Control+Home
- View: Go to end of file: Control+End
- Edit: Move edit cursor right one pixel: Alt+RightArrow or Control+Alt+Numpad6
- Edit: Move edit cursor left one pixel: Alt+LeftArrow or Control+Alt+Numpad4
- Navigate: Move edit cursor right by grid: Alt+Shift+RightArrow or Control+Numpad6
- Navigate: Move edit cursor left by grid: Alt+Shift+LeftArrow or Control+Numpad4
- Navigate: Move edit cursor right one measure: PageDown
- Navigate: Move edit cursor left one measure: PageUp
- Edit: Increase pitch cursor one semitone: Alt+UpArrow or Ctrl+NumPad8
- Edit: Decrease pitch cursor one semitone: Alt+DownArrow or Ctrl+NumPad2
- Edit: Increase pitch cursor one octave: Alt+Shift+UpArrow
- Edit: Decrease pitch cursor one octave: Alt+Shift+DownArrow
- Edit: Delete events: Delete
- Edit: Insert note at edit cursor: I
- Edit: Select all events: Ctrl+A
- Edit: Select all notes in time selection
- Invert selection: Ctrl+I
- Select all notes with the same pitch: Shift+A
- Grid: Set to 1: 1, control+shift+1
- Grid: Set to 1/2: 2, control+shift+2
- Grid: Set to 1/32: 3, control+shift+3
- Grid: Set to 1/4: 4, control+shift+4
- Grid: Set to 1/6 (1/4 triplet): 5, control+shift+5
- Grid: Set to 1/16: 6, control+shift+6
- Grid: Set to 1/24 (1/16 triplet): 7, control+shift+7
- Grid: Set to 1/8: 8, control+shift+8
- Grid: Set to 1/12 (1/8 triplet): 9, control+shift+9
- Grid: Set to 1/48 (1/32 triplet): 0
- Grid: Set to 1/64
- Set length for next inserted note: 1: control+1
- Set length for next inserted note: 1/2: control+2
- Set length for next inserted note: 1/32: control+3
- Set length for next inserted note: 1/4: control+4
- Set length for next inserted note: 1/4T: control+5
- Set length for next inserted note: 1/16: control+6
- Set length for next inserted note: 1/16T: control+7
- Set length for next inserted note: 1/8: control+8
- Set length for next inserted note: 1/8T: control+9
- Set length for next inserted note: 1/32T: control+0
- Activate next MIDI track: Ctrl+DownArrow
- Activate previous MIDI track: Ctrl+UpArrow
- Edit: Move notes up one semitone: NumPad8 or N
- Edit: Move notes down one semitone: NumPad2 or shift+N
- Edit: Move notes up one semitone ignoring scale/key: Ctrl+Alt+NumPad8
- Edit: Move notes down one semitone ignoring scale/key: Ctrl+Alt+NumPad2
- Edit: Move notes up one octave: Alt+NumPad8 or Alt+N
- Edit: Move notes down one octave: Alt+NumPad2 or Alt+Shift+N
- Edit: Lengthen notes one pixel: Alt+NumPad3 or Alt+L
- Edit: Shorten notes one pixel: Alt+NumPad1 or Alt+Shift+L
- Edit: Lengthen notes one grid unit: NumPad3 or L
- Edit: Shorten notes one grid unit: NumPad1 or Shift+L
- Edit: Set note lengths to grid size: Ctrl+Shift+L
- Edit: Make notes legato, preserving note start times: G
- Edit: Move notes left one pixel: Alt+NumPad4 or Alt+Shift+P
- Edit: Move notes right one pixel: Alt+NumPad6 or Alt+P
- Edit: Move notes left one grid unit: NumPad4 or Shift+P
- Edit: Move notes right one grid unit: NumPad6 or P
- Edit: Note velocity +01: NumPad9 or V
- Edit: Note velocity +10: Alt+NumPad9 or Alt+V
- Edit: Note velocity -01: NumPad7 or Shift+V
- Edit: Note velocity -10: Alt+NumPad7 or Alt+Shift+V
- Edit: Join notes: J
- Edit: Split notes: S
- Edit: Increase value a little bit for CC events: =
- Edit: Decrease value a little bit for CC events: -
- CC: Next CC lane: Control+Alt+=
- CC: Previous CC lane: Control+Alt+-
- Options: MIDI inputs as step input mode
- Options: F1-F12 as step input mode

### Context Menus
There are several context menus in REAPER, but some of them are difficult to access or not accessible at all from the keyboard.
OSARA enables keyboard access for the track input, track area, track routing, item, ruler, envelope point and automation item context menus.

To open the context menu for the element you are working with, press the applications key or shift+f10 on Windows, or Control+1 on Mac.
For example, if you have just moved to a track, it will open the track input context menu for that track.
If you have just moved the edit cursor, it will open the context menu for the ruler.

For tracks, there are three context menus:

1. Track input: Allows you to set the input to use when recording, etc.
 You access this by just pressing the applications key or shift+f10 on Windows, or Control+1 on Mac.
2. Track area: Provides options for inserting, duplicating and removing tracks, etc.
 You access this by pressing control+applications or control+shift+f10 on Windows, or Control+2 on Mac.
3. Routing: Allows you to quickly add and remove sends, receives and outputs without opening the I/O window.
 You access this by pressing alt+applications or control+alt+shift+f10 on Windows, or Control+3 on Mac.

### Parameter Lists
OSARA can display a list of parameters for various elements such as tracks, items and effects.
You can then check and change the values of these parameters.
This is useful for parameters which are tedious or impossible to access otherwise.

#### Track/Item Parameters
To access the parameter list for a track or an item, select the track or item you wish to work with.
Then, press alt+p (OSARA: View parameters for current track/item).

#### FX Parameters
Many effects are unfortunately either partially or completely inaccessible.
However, most effects make their parameters available for automation in a standard way.
This can also be used to make them at least partially accessible.
Thus, the FX parameter list is particular useful and is the only way to access some effects.

To access it:

1. Select a track or item with at least one effect.
 Then, press p (OSARA: View FX parameters for current track/take).
2. Alternatively, to access FX parameters for the master track, press shift+p (OSARA: View FX parameters for master track).
3. If there is more than one effect on the track, select the desired effect from the menu.
 If there are input or monitoring FX on the track, these will be included in the menu as well with an appropriate suffix.

Note that only some effects expose easily readable values, while others expose only percentages.
Even for effects that do expose easily readable values, the editable text is an internal number and probably won't correspond to the readable value on the slider.

#### Using Parameter Lists
Once you have opened a parameter list dialog, you can select a parameter from the Parameter combo box and check or adjust its value using the Value slider.
For parameters which support it, there is also an editable text field which allows you to edit the value textually.

The Filter field allows you to narrow the list to only contain parameters which include the entered text.
For example, if the full list contained "Volume" and "Pan" parameters and you type "vol" in the Filter field, the list will be narrowed to only show "Volume".
Clearing the text in the Filter field shows the entire list.

When you are done working with parameters, press the Close button.
Alternatively, you can press enter or escape.

### Reading Current Peaks
OSARA allows you to read the current audio peak for channels 1 and 2 of either the current or master tracks.
You do this using the following actions:

- OSARA: Report current peak for channel 1 of current track: j
- OSARA: Report current peak for channel 2 of current track: k
- OSARA: Report current peak for channel 1 of master track: shift+j
- OSARA: Report current peak for channel 2 of master track: shift+k

### Peak Watcher
In addition to reading current peaks, You can also be notified automatically when the volume exceeds a specified maximum level using Peak Watcher.
This can be done for one or two tracks.

To use Peak Watcher:

1. Press Alt+w (OSARA: View Peak Watcher).
2. From the First track combo box, select one of the following:
 - None: Select this if you do not wish to monitor a track.
 - Follow current track: Select this if you want to watch peaks for whatever track you move to in your project.
 - Master: This watches peaks for the master track.
 - Otherwise, you can choose any track in your project.
3. From the "Level type for first track" combo box, select the type of level you want to use: dB peak or several LUFS options.
 The LUFS options use the JS: EBUR128 Loudness Measurement V2.11 (TBProAudio) effect, which is installed with OSARA.
 OSARA will add the effect to tracks automatically and remove it when it is no longer required.
4. If you wish to monitor a second track, you can choose another track and desired level type from the subsequent combo boxes.
5. If you want to be notified when the level of channels exceeds a certain level, in the "Notify automatically for channels:" grouping, check the options for the desired channels and enter the desired level.
6. The Hold highest level grouping allows you to specify whether the highest level remains as the reported level and for how long.
 Holding the highest level gives you time to examine the highest level, even if the audio level dropped immediately after the highest level occurred.
 There are three options:
 - disabled: Don't hold the highest level at all.
 - until reset: Hold the highest level until the Peak Watcher is reset.
 - for (ms): Allows you to specify a time in milliseconds for which the highest level will be held.
7. Press the Reset button to reset the reported peak levels if they are being held.
8. When you are done, press the OK button to accept any changes or the Cancel button to discard them.

At any time, you can report or reset the peak levels for either of the tracks being watched using the following actions:

- OSARA: Report Peak Watcher value for channel 1 of first track: alt+f11
- OSARA: Report Peak Watcher value for channel 2 of first track: alt+f12
- OSARA: Report Peak Watcher value for channel 1 of second track: Alt+Shift+F11
- OSARA: Report Peak Watcher value for channel 2 of second track: Alt+Shift+F12
- OSARA: Reset Peak Watcher for first track: alt+f10
- OSARA: Reset Peak Watcher for second track: Alt+Shift+F10

You can also quickly pause Peak Watcher using OSARA: Pause/resume Peak Watcher.
While paused, Peak Watcher won't notify you of any level changes.
You can later use the same action again to resume automatic reporting.

### Shortcut Help
It is possible to have REAPER list all shortcuts and to search for individual shortcuts in the Action List.
However, it is sometimes convenient to be able to simply press a shortcut and immediately learn what action it will perform.
This is especially useful if you have forgotten an exact shortcut but do have some idea of what it might be.
You can achieve this using OSARA's shortcut help mode.

You can turn shortcut help on and off by pressing f12 (OSARA: Toggle shortcut help).
While shortcut help is enabled, pressing any shortcut will report the action associated with that shortcut, but the action itself will not be run.

### Noncontiguous Selection
Usually, selection is done contiguously; e.g. you might select tracks 1 through 4.
Sometimes, it is desirable to select noncontiguously; e.g. you might want to select tracks 1, 3 and 5.

You can do this as follows:

1. Move to the track or item you want to start with.
2. Optionally, select some other contiguous tracks or items.
3. Press shift+space (OSARA: Enable noncontiguous selection/toggle selection of current track/item) to switch to noncontiguous selection.
4. Move through tracks/items leaving other tracks/items selected; e.g. using shift+downArrow and shift+upArrow to move through tracks.
 These tracks/items will not be selected, but any previously selected tracks/items will remain selected.
5. When you reach a track/item you want to select, press shift+space (OSARA: Enable noncontiguous selection/toggle selection of current track/item).
 You can also use this if you want to unselect a previously selected track/item.

Selection will revert to contiguous selection the next time you move to a track/item without leaving other tracks/items selected.

If you want to select noncontiguous items on several different tracks, the procedure is exactly the same.
However, it's important to remember that you must move between tracks without affecting the selection; i.e. using shift+downArrow and shift+upArrow.
Otherwise, selection will revert to contiguous selection.

### Accessing Controls for Sends/Receives/Outputs in the Track I/O Window
In the Track I/O window, there are various controls for each send, receive or hardware output.
Unfortunately, these controls cannot be reached with the tab key and it is tedious at best to access these with screen reader review functions.

When you tab to the Delete button for a send/receive/output, the name of the send/receive/output will first be reported.
You can then press the Applications key or shift+f10 to access a menu of additional options.

### Manually Moving Stretch Markers
REAPER includes actions to snap stretch markers to the grid.
However, sometimes, this is not sufficient and it is useful to be able to manually move stretch markers to a specific position.

To do this:

1. Select the desired item.
2. Go to a stretch marker; e.g. using shift+apostrophe (Item: go to next stretch marker).
 Ensure that OSARA reports the stretch marker.
3. Move the edit cursor to the position to which you wish to move the stretch marker.
4. Press Control+Alt+M (OSARA: Move last focused stretch marker to current edit cursor position).

### Accessing FX Plug-in Windows
Some FX plug-ins can be controlled with keyboard commands, but you can't reach them by tabbing through the FX Chain dialog.
In these cases, you can press f6 to have OSARA attempt to focus the plug-in window.

### Automation Items
An envelope can contain one or more automation items positioned along the timeline.
With OSARA, you move to automation items as follows:

1. Select an envelope using Alt+L or Alt+Shift+L (OSARA: Select next track/take envelope (depending on focus)).
2. Now, use the normal item navigation commands; i.e. control+rightArrow and control+leftArrow (Item navigation: Select and move to next item, Item navigation: Select and move to previous item).
 Multiple selection is also possible using control+shift+rightArrow and control+shift+leftArrow (OSARA: Move to next item (leaving other items selected), OSARA: Move to previous item (leaving other items selected).
 Noncontiguous selection is done the same way described above for tracks and items.
3. The item navigation commands will revert back to moving through media items (instead of automation items) when focus is moved away from the envelope.
 For example, moving to another track and back again will again allow you to move through the media items on the track.

Once you move to an automation item, the commands to move between envelope points such as alt+k and alt+j (OSARA: Move to next envelope point, OSARA: Move to previous envelope point) move between the points in the automation item.
The points within an automation item can only be accessed after moving to that automation item; they cannot be accessed from the underlying envelope.
To return to the points in the underlying envelope, simply move focus back to the envelope by selecting it again with Alt+L and Alt+Shift+L (OSARA: Select next track/take envelope (depending on focus), OSARA: Select previous track/take envelope (depending on focus)).

### Notes and Chords in the MIDI Editor
In the MIDI Editor, OSARA enables you to move between chords and to move to individual notes in a chord.
In this context, a chord is any number of notes that are placed at the exact same position.
If there is only one note at a given position, it will be treated as a chord.

You move between chords using the left and right arrow keys (OSARA: Move to previous chord, OSARA: Move to next chord).
When you move to a chord, the edit cursor will be placed at the chord and the new position will be reported.
The notes of the chord will be played and the number of notes will be reported.
The notes in the chord are also selected so you can manipulate the entire chord.
For example, pressing delete will delete the chord.

Once you have moved to a chord, you can move to individual notes using the up and down arrow keys (OSARA: Move to previous note in chord, OSARA: Move to next note in chord).
When you move to a note, that note will be played and its name will be reported.
The single note will also be selected so you can manipulate just that note.
For example, pressing delete will delete only that note.

You can select multiple chords or multiple notes in a chord.
To do this, first move to the first chord or note you want to select.
Then, use shift plus the arrow keys to add the next or previous chord or note to the selection.
For example, if you've moved to a chord and also want to add the next chord to the selection, you would press shift+rightArrow.

Noncontiguous selection is also possible.
You do this in the same way described above for tracks and items.
That is, press shift+space (OSARA: Enable noncontiguous selection/toggle selection of current chord/note) to switch to noncontiguous selection, move to other chords/notes with shift plus the arrow keys and press shift+space to select/unselect the current chord/note.

### Note Preview in the MIDI Event List (Windows Only)
When the MIDI Editor is set to Event List mode, REAPER presents a list with all the events in the current MIDI item.
When a note gets focus in the list, OSARA will play a preview of the focused note.

### Navigating FX Presets Without Activating Them (Windows Only)
REAPER's FX preset combo box doesn't allow keyboard users to move through presets without activating them.
Sometimes, you need to be able to examine the presets without activating each one.
OSARA provides a dialog to facilitate this.

You activate this dialog by pressing alt+downArrow when focused on REAPER's FX preset combo box.
The dialog displays the preset list and allows you to navigate it with the keyboard without activating the preset.
The Filter field allows you to filter the list to show only presets containing the entered text.
Pressing the OK button activates the preset selected in the list.

### Configuration
OSARA includes a Configuration dialog to adjust various settings.
You open this dialog by pressing control+alt+shift+p (OSARA: Configuration).

The dialog contains the following options:

- Report position when scrubbing: When disabled, OSARA will not report the cursor position when using the scrubbing actions (View: Move cursor left/right one pixel).
- Report time movement during playback/recording: When disabled, OSARA will not report actions during playback/recording which move the cursor or adjust positions or lengths.
 Although not strictly time movement, this also includes inserting markers or regions.
- Move relative to the play cursor for time movement commands during playback: When enabled, time movement commands such as scrubbing or moving by bar/beat will move from where you are currently playing, rather than relative to the edit cursor.
- Report markers during playback: When enabled, project markers and regions will be reported during playback as the cursor passes them.
- Report transport state (play, record, etc.): When enabled, OSARA will report the transport state when you change it; e.g. if you begin playing or recording.
- Report FX when moving to tracks/takes: When enabled, OSARA will report the names of any effects on a track or take when you move to it.
- Report MIDI notes in MIDI editor: When enabled, OSARA will report the names of individual MIDI notes and the number of notes in a chord.
- Report changes made via control surfaces: When enabled, OSARA will report track selection changes, parameter changes, etc. made using a control surface.

When you are done, press the OK button to accept any changes or the Cancel button to discard them.

### Miscellaneous Actions
OSARA also includes some other miscellaneous actions.

#### Main
- OSARA: Move to next item (leaving other items selected): control+shift+rightArrow
- OSARA: Move to previous item (leaving other items selected): control+shift+leftArrow
- OSARA: View properties for current media item/take/automation item (depending on focus)
- OSARA: View I/O for master track: shift+i
- OSARA: Report ripple editing mode: control+shift+p
- OSARA: Report muted tracks: Alt+shift+f5
 - Pressing this twice will display the information in a dialog with a text box for easy review.
- OSARA: Report soloed tracks: Alt+shift+f6
 - Pressing this twice will display the information in a dialog with a text box for easy review.
- OSARA: Report record armed tracks: Alt+shift+f7
 - Pressing this twice will display the information in a dialog with a text box for easy review.
- OSARA: Report tracks with record monitor on: Control+shift+f8
 - Pressing this twice will display the information in a dialog with a text box for easy review.
- OSARA: Report tracks with phase inverted: Alt+shift+f9
 - Pressing this twice will display the information in a dialog with a text box for easy review.
- OSARA: Report track/item/time selection (depending on focus): control+shift+space
 - Pressing this twice will display the information in a dialog with a text box for easy review.
- OSARA: Remove items/tracks/contents of time selection/markers/envelope points (depending on focus): delete
- OSARA: Report edit/play cursor position: control+shift+j
 - If the ruler unit is set to Measures.Beats / Minutes:Seconds, Pressing this once will report the time in measures.beats, while pressing it twice will report the time in minutes:seconds .
- OSARA: Delete all time signature markers: alt+win+delete
- OSARA: Select next track/take envelope (depending on focus): alt+l
- OSARA: Select previous track/take envelope (depending on focus): alt+shift+l
- OSARA: Move to next envelope point: alt+k
- OSARA: Move to previous envelope point: alt+j
- OSARA: Move to next envelope point (leaving other points selected): alt+shift+k
- OSARA: Move to previous envelope point (leaving other points selected): alt+shift+j
- OSARA: Move to next transient: tab
- OSARA: Move to previous transient: shift+tab
- OSARA: Cycle automation mode of selected tracks: control+shift+\
- OSARA: Report global / Track Automation Mode: control+shift+l
- OSARA: Toggle global automation override between latch preview and off: control+alt+shift+l
- OSARA: Cycle through midi recording modes of selected tracks: alt+shift+\
- OSARA: About

#### Midi editor
- OSARA: Move to next midi item on track: Control+RightArrow
- OSARA: Move to previous midi item on track: Control+LeftArrow
- OSARA: Move to next CC: Control+=
- OSARA: Move to previous CC: Control+-
- OSARA: Move to next CC and add to selection: Control+Shift+=
- OSARA: Move to previous CC and add to selection: Control+Shift+-
- OSARA: Select all notes with the same pitch starting in time selection: Alt+Shift+A

#### MIDI Event List Editor
- OSARA: Focus event nearest edit cursor: control+f

## Support
If you need help, please subscribe to the [Reapers Without Peepers discussion group](https://groups.io/g/rwp) and ask your questions there.

## Reporting Issues
Issues should be reported [on GitHub](https://github.com/jcsteh/osara/issues).

## Translating
This section is for those interested in translating OSARA into their language.

OSARA can be translated using gettext PO files.
If a REAPER language pack is installed, OSARA will attempt to load a translation based on the name of the REAPER language pack.
OSARA can only load PO files in the UTF-8 encoding.

Translations are managed on [Crowdin](https://crowdin.com/project/osara).
With Crowdin, you can translate entirely online on the web, upload and download translations as .po files or directly sync your translations using Poedit.

If you'd like to translate OSARA, you will first need to [sign up for a Crowdin account](https://accounts.crowdin.com/register).
Then, please [file a GitHub issue](https://github.com/jcsteh/osara/issues/new) and include these details:

- Your Crowdin username;
- The language you'd like to translate to; and
- The file name of the REAPER language pack (or packs) for your language.

If you'd like to test your translation before it is included in OSARA, you can copy it into the osara/locale folder inside your REAPER resource folder .
If you don't know where the REAPER resource folder is, use the Show REAPER resource path action in REAPER.
The file should be named with the same name as the REAPER language pack you are using, but with a .po extension.
For example, if you have installed German.ReaperLangpack, rename the file to German.po.
You will need to restart REAPER for any translation changes to take effect.

## Building
This section is for those interested in building OSARA from source code.

### Getting the Source Code
The OSARA Git repository is located at https://github.com/jcsteh/osara.git.
You can clone it with the following command, which will place files in a directory named osara:

```
git clone --recursive https://github.com/jcsteh/osara.git
```

The `--recursive` option is needed to retrieve Git submodules we use.
If you didn't pass this option to `git clone`, you will need to run `git submodule update --init`.
Whenever a required submodule commit changes (e.g. after git pull), you will need to run `git submodule update`.
If you aren't sure, run `git submodule update` after every git pull, merge or checkout.

### Dependencies
To build OSARA, you will need:

- Windows only: Microsoft Visual Studio 2019 Community:
	* [Download Visual Studio 2019 Community](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&rel=16)
	* When installing Visual Studio, you need to enable the following:
		- On the Workloads tab, in the Windows group: Desktop development with C++
- Mac only: Either the [command line developer tools](https://developer.apple.com/library/ios/technotes/tn2339/_index.html) or Xcode 12.3:
	* You can download Xcode from the [Mac App Store](https://apps.apple.com/us/app/xcode/id497799835?ls=1&mt=12).
- Python, version 3.7 or later:
	* This is needed by SCons.
	* [Download Python](https://www.python.org/downloads/)
- [SCons](https://www.scons.org/), version 3.0.4 or later:
	* Once Python is installed, you should be able to install SCons by simply running this at the command line: `pip install scons`
- Windows only: [NSIS](https://nsis.sourceforge.io/Download), version 3.03 or later

### How to Build
To build OSARA, from a command prompt, simply change to the OSARA checkout directory and run scons.
The resulting installer can be found in the installer directory.

## Contributors
- NV Access Limited
- James Teh
- Victor Tsaran
- Scott Chesworth
- Derek Lane
- Gianluca Apollaro
- Marc Mulcahy
- Garth Humphreys
- Lars Sönnebo
- Alexey Zhelezov
- Leonard de Ruijter
- Robbie Murray
