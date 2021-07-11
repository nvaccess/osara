/*
 * OSARA: Open Source Accessibility for the REAPER Application
 * MIDI Editor commands code
 * Author: James Teh <jamie@jantrid.net>
 * Copyright 2015-2017 NV Access Limited
 * License: GNU General Public License version 2.0
 */

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include "midiEditorCommands.h"
#include "osara.h"
#include "translation.h"
#ifdef _WIN32
#include <Commctrl.h>
#endif

using namespace std;

// Note: while the below struct is called MidiControlChange in line with naming in Reaper,
// It is also used for other MIDI messages.
typedef struct {
	int channel;
	int index;
	int message1;
	int message2;
	int message3;
	double position;
} MidiControlChange;

const UINT DEFAULT_PREVIEW_LENGTH = 300; // ms

struct MidiNote {
	int channel;
	int pitch;
	int velocity;
	int index;
	double start;
	double end;

	double getLength() const {
		return max (0, (end - start));
	}
};

vector<MidiNote> previewingNotes; // Notes currently being previewed.
UINT_PTR previewDoneTimer = 0;
const int MIDI_NOTE_ON = 0x90;
const int MIDI_NOTE_OFF = 0x80;
bool shouldReportNotes = true;

// A minimal PCM_source to send MIDI events for preview.
class PreviewSource : public PCM_source {
	public:
	
	PreviewSource() {
	}

	virtual ~PreviewSource() {
	}

	// The events to send.
	// These will be consumed (and the vector cleared) soon after PlayTrackPreview is called.
	vector<MIDI_event_t> events;

	bool SetFileName(const char* fn) {
		return false;
	}

	PCM_source* Duplicate() {
		return new PreviewSource();
	}

	bool IsAvailable() {
		return true;
	}

	const char* GetType() {
		return "OsaraMIDIPreview";
	};

	int GetNumChannels() {
		return 1;
	}

	double GetSampleRate() {
		return 0.0;
	}

	double GetLength() {
		// This only needs to be long enough to send MIDI events immediately.
		// Once we send note on events, the notes stay on, even though the preview stops.
		// We then play another preview with more events to turn the notes off.
		return 0.001;
	}

	int PropertiesWindow(HWND parent) {
		return -1;
	}

	void GetSamples(PCM_source_transfer_t* block) {
		block->samples_out=0;
		if (block->midi_events) {
			for (auto event = this->events.begin(); event != this->events.end(); ++event)
				block->midi_events->AddItem(&*event);
			this->events.clear();
		}
	}

	void GetPeakInfo(PCM_source_peaktransfer_t* block) {
		block->peaks_out=0;
	}

	void SaveState(ProjectStateContext* ctx) {
	}

	int LoadState(char* firstLine, ProjectStateContext* ctx) {
		return -1;
	}

	void Peaks_Clear(bool deleteFile) {
	}

	int PeaksBuild_Begin() {
		return 0;
	}

	int PeaksBuild_Run() {
		return 0;
	}

	void PeaksBuild_Finish() {
	}

};

PreviewSource previewSource;
preview_register_t previewReg = {0};

// Queue note off events for the notes currently being previewed.
// This function doesn't begin sending the events.
void previewNotesOff() {
	for (auto note = previewingNotes.cbegin(); note != previewingNotes.cend(); ++note) {
		MIDI_event_t event = {0, 3, {
			(unsigned char)(MIDI_NOTE_OFF | note->channel),
			(unsigned char)note->pitch, (unsigned char)note->velocity}};
		previewSource.events.push_back(event);
	}
}

// Called after the preview length elapses to turn off notes currently being previewed.
void CALLBACK previewDone(HWND hwnd, UINT msg, UINT_PTR event, DWORD time) {
	if (event != previewDoneTimer)
		return; // Cancelled.
	previewNotesOff();
	previewingNotes.clear();
	// Send the events.
	previewReg.curpos = 0.0;
	PlayTrackPreview(&previewReg);
	previewDoneTimer = 0;
	KillTimer(nullptr, event);
}

// Used to find out the minimum note length.
bool compareNotesByLength(const MidiNote& note1, const MidiNote& note2) {
	return note1.getLength() < note2.getLength();
}

void previewNotes(MediaItem_Take* take, const vector<MidiNote>& notes) {
	if (!GetToggleCommandState2(SectionFromUniqueID(MIDI_EDITOR_SECTION), 40041)) {  // Options: Preview notes when inserting or editing
		return;
	}
	if (!previewReg.src) {
		// Initialise preview.
#ifdef _WIN32
		InitializeCriticalSection(&previewReg.cs);
#else
		pthread_mutex_init(&previewReg.mutex, NULL);
#endif
		previewReg.src = &previewSource;
		previewReg.m_out_chan = -1; // Use .preview_track.
	}
	if (previewDoneTimer) {
		// Notes are currently being previewed. Interrupt them.
		// We want to turn off these notes immediately.
		KillTimer(NULL, previewDoneTimer);
		previewDoneTimer = 0;
		previewNotesOff();
	}
	// Queue note on events for the new notes.
	for (auto note = notes.cbegin(); note != notes.cend(); ++note) {
		MIDI_event_t event = {0, 3, {
			(unsigned char)(MIDI_NOTE_ON | note->channel),
			(unsigned char)note->pitch, (unsigned char)note->velocity}};
		previewSource.events.push_back(event);
	}
	// Save the notes being previewed so we can turn them off later (previewNotesOff).
	previewingNotes = notes;
	// Send the events.
	void* track = GetSetMediaItemTakeInfo(take, "P_TRACK", NULL);
	previewReg.preview_track = track;
	previewReg.curpos = 0.0;
	PlayTrackPreview(&previewReg);
	// Calculate the minimum note length.
	double minLength = min_element(notes.begin(), notes.end(), compareNotesByLength)->getLength();
	// Schedule note off messages.
	previewDoneTimer = SetTimer(nullptr, 0,
		(UINT)(minLength ? minLength * 1000 : DEFAULT_PREVIEW_LENGTH), previewDone);
}

void cancelMidiPreviewNotesOff() {
	if (previewDoneTimer) {
		KillTimer(nullptr, previewDoneTimer);
		previewDoneTimer = 0;
	}
}

void cmdMidiMoveCursor(Command* command) {
	HWND editor = MIDIEditor_GetActive();
	MIDIEditor_OnCommand(editor, command->gaccel.accel.cmd);
	ostringstream s;
	s << formatCursorPosition(TF_MEASURE);
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	int noteCount;
	MIDI_CountEvts(take, &noteCount, NULL, NULL);
	double now = GetCursorPosition();
	// todo: Optimise; perhaps a binary search?
	vector<MidiNote> notes;
	for (int n = 0; n < noteCount; ++n) {
		double start, end;
		int chan, pitch, vel;
		MIDI_GetNote(take, n, NULL, NULL, &start, &end, &chan, &pitch, &vel);
		start = MIDI_GetProjTimeFromPPQPos(take, start);
		if (start > now) {
			break;
		}
		if (start == now) {
			end = MIDI_GetProjTimeFromPPQPos(take, end);
			notes.push_back({chan, pitch, vel, 0, start, end});
		}
	}
	auto count = notes.size();
	if (count > 0) {
		previewNotes(take, notes);
		fakeFocus = FOCUS_NOTE;
		s << " " << count << (count == 1 ? " note" : " notes");
	}
	outputMessage(s);
}

const string getMidiNoteName(MediaItem_Take *take, int pitch, int channel) {
	static const char* names[] = {
		// Translators: The name of a musical note.
		translate("c"),
		// Translators: The name of a musical note.
		translate("c sharp"),
		// Translators: The name of a musical note.
		translate("d"),
		// Translators: The name of a musical note.
		translate("d sharp"),
		// Translators: The name of a musical note.
		translate("e"),
		// Translators: The name of a musical note.
		translate("f"),
		// Translators: The name of a musical note.
		translate("f sharp"),
		// Translators: The name of a musical note.
		translate("g"),
		// Translators: The name of a musical note.
		translate("g sharp"),
		// Translators: The name of a musical note.
		translate("a"),
		// Translators: The name of a musical note.
		translate("a sharp"),
		// Translators: The name of a musical note.
		translate("b")
	};
	MediaTrack* track = GetMediaItemTake_Track(take);
	int tracknumber = static_cast<int> (GetMediaTrackInfo_Value(track, "IP_TRACKNUMBER")); // one based
	const char* noteName = GetTrackMIDINoteName(tracknumber - 1, pitch, channel); // track number is zero based
	ostringstream s;
	if (noteName) {
		s << noteName;
	} else {
		int octave = pitch / 12 - 1;
		int szOut = 0;
		int* octaveOffset = (int*)get_config_var("midioctoffs", &szOut);
		if (octaveOffset && (szOut == sizeof(int))) {
			octave += *octaveOffset - 1; // REAPER offset "0" is saved as "1" in the preferences file.
		}
		pitch %= 12;
		s << names[pitch] << " " << octave;
	}
	return s.str();
}

// Returns the indexes of the first and last notes in a chord in a given direction.
pair<int, int> findChord(MediaItem_Take* take, int direction) {
	int count;
	MIDI_CountEvts(take, &count, NULL, NULL);
	double now = GetCursorPosition();
	// Find the first note of the chord.
	int firstNote = direction == -1 ? count - 1 : 0;
	int movement = direction == 0 ? 1 : direction;
	bool found = false;
	int nowNote = -1;
	double firstStart;
	for (; 0 <= firstNote && firstNote < count; firstNote += movement) {
		MIDI_GetNote(take, firstNote, NULL, NULL, &firstStart, NULL, NULL, NULL, NULL);
		firstStart = MIDI_GetProjTimeFromPPQPos(take, firstStart);
		if (firstStart == now)
			nowNote = firstNote;
		if ((direction == 0 && firstStart == now)
			|| (direction == 1 && firstStart > now)
			|| (direction == -1 && firstStart < now)
		) {
			found = true;
			break;
		}
	}
	if (!found) {
		if (nowNote != -1) {
			// No chord in the requested direction, so use the chord at the cursor.
			firstNote = nowNote;
			firstStart = now;
			// Reverse the direction to find the remaining notes in the chord.
			movement = direction == 1 ? -1 : 1;
		} else
			return {-1, -1};
	}
	// Find the last note of the chord.
	int lastNote = firstNote;
	for (int note = lastNote + movement; 0 <= note && note < count; note += movement) {
		double start;
		MIDI_GetNote(take, note, NULL, NULL, &start, NULL, NULL, NULL, NULL);
		start = MIDI_GetProjTimeFromPPQPos(take, start);
		if (start != firstStart)
			break;
		lastNote = note;
	}
	return {min(firstNote, lastNote), max(lastNote, firstNote)};
}

// Keeps track of the note to which the user last moved in a chord.
// This is the number of the note in the chord; e.g. 0 is the first note.
// It is not a REAPER note index!
// -1 means not in a chord.
int curNoteInChord = -1;
// Keeps track of the CC to which the user last moved.
// This is a REAPER CC index.
// -1 means no CC.
int currentCC = -1;

// Used to order notes in a chord by pitch.
bool compareNotesByPitch(const MidiNote& note1, const MidiNote& note2) {
	return note1.pitch < note2.pitch;
}

// Finds a single note in the chord at the cursor in a given direction and returns its info.
// This updates curNoteInChord.
MidiNote findNoteInChord(MediaItem_Take* take, int direction) {
	auto chord = findChord(take, 0);
	if (chord.first == -1)
		return {-1};
	// Notes at the same position are ordered arbitrarily.
	// This is not intuitive, so sort them.
	vector<MidiNote> notes;
	for (int note = chord.first; note <= chord.second; ++note) {
		double start, end;
		int chan, pitch, vel;
		MIDI_GetNote(take, note, NULL, NULL, &start, &end, &chan, &pitch, &vel);
		start = MIDI_GetProjTimeFromPPQPos(take, start);
		end = MIDI_GetProjTimeFromPPQPos(take, end);
		notes.push_back({chan, pitch, vel, note, start, end});
	}
	sort(notes.begin(), notes.end(), compareNotesByPitch);
	const int lastNoteIndex = (int)notes.size() - 1;
	// Work out which note to move to.
	if (direction != 0 && 0 <= curNoteInChord &&
			curNoteInChord <= (int)lastNoteIndex) {
		// Already on a note within the chord. Move to the next/previous note.
		curNoteInChord += direction;
		// If we were already on the first/last note, stay there.
		if (curNoteInChord < 0 || curNoteInChord > lastNoteIndex)
			curNoteInChord -= direction;
	} else if (direction != 0) {
		// We're moving into a new chord. Move to the first/last note.
		curNoteInChord = direction == 1 ? 0 : (int)lastNoteIndex;
	}
	return notes[curNoteInChord];
}

void selectNote(MediaItem_Take* take, const int note, bool select=true) {
	MIDI_SetNote(take, note, &select, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

bool isNoteSelected(MediaItem_Take* take, const int note) {
	bool sel;
	MIDI_GetNote(take, note, &sel, NULL, NULL, NULL, NULL, NULL, NULL);
	return sel;
}

int countSelectedNotes(MediaItem_Take* take, int offset=-1) {
	int noteIndex = offset;
	int count = 0;
	for(;;){
		noteIndex = MIDI_EnumSelNotes(take, noteIndex);
		if (noteIndex == -1) {
			break;
		}
		++count;
	}
	return count;
}

vector<MidiNote> getSelectedNotes(MediaItem_Take* take, int offset=-1) {
	int noteIndex = offset;
	vector<MidiNote> notes;
	for(;;){
		noteIndex = MIDI_EnumSelNotes(take, noteIndex);
		if (noteIndex == -1) {
			break;
		}
		double start, end;
		int chan, pitch, vel;
		MIDI_GetNote(take, noteIndex, NULL, NULL, &start, &end, &chan, &pitch, &vel);
		start = MIDI_GetProjTimeFromPPQPos(take, start);
		end = MIDI_GetProjTimeFromPPQPos(take, end);
		notes.push_back({chan, pitch, vel, noteIndex, start, end});
	}
	return notes;
}

// Finds a single CC at the cursor in a given direction and returns its info.
// This updates currentCC.
MidiControlChange findCC(MediaItem_Take* take, int direction) {
	int count;
	MIDI_CountEvts(take, NULL, &count, NULL);
	double cursor = GetCursorPosition();
	double position;
	int start = direction == -1 ? count - 1 : 0;
	if (currentCC != -1) {
		MIDI_GetCC(take, currentCC, NULL, NULL, &position, NULL, NULL, NULL, NULL);
		position = MIDI_GetProjTimeFromPPQPos(take, position);
		if (direction == 0 && position == cursor) {
			start = currentCC;
		} else if (direction == 1 ? position <= cursor : position >= cursor) {
			// The cursor is right at or has moved past the CC to which the user last moved.
			// Therefore, start at the adjacent CC.
			start = currentCC + direction;
			if (start < 0 || start >= count) {
				// There's no adjacent item in this direction,
				// so move to the current one again.
				start = currentCC;
			}
		}
	}
	int movement = direction == 0 ? 1 : direction;
	bool found = false;
	int chan, msg1, msg2, msg3;
	for (; 0 <= start && start < count; start += movement) {
		MIDI_GetCC(take, start, NULL, NULL, &position, &msg1, &chan, &msg2, &msg3);
		position = MIDI_GetProjTimeFromPPQPos(take, position);
		if (movement == -1 ? position <= cursor : position >= cursor) {
			currentCC = start;
			found = true;
			break;
		}
	}
	if (!found) {
		return {-1};
	}
	return {chan, currentCC, msg1, msg2, msg3, position};
}

void selectCC(MediaItem_Take* take, const int cc, bool select=true) {
	MIDI_SetCC(take, cc, &select, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

bool isCCSelected(MediaItem_Take* take, const int cc) {
	bool sel;
	MIDI_GetCC(take, cc, &sel, NULL, NULL, NULL, NULL, NULL, NULL);
	return sel;
}

vector<MidiControlChange> getSelectedCCs(MediaItem_Take* take, int offset=-1) {
	int ccIndex = offset;
	vector<MidiControlChange> ccs;
	for (;;) {
		ccIndex = MIDI_EnumSelCC(take, ccIndex);
		if (ccIndex == -1) {
			break;
		}
		double position;
		int chan, msg1, msg2, msg3;
		MIDI_GetCC(take, ccIndex, NULL, NULL, &position, &msg1, &chan, &msg2, &msg3);
		position = MIDI_GetProjTimeFromPPQPos(take, position);
		ccs.push_back({chan, ccIndex, msg1, msg2, msg3, position});
	}
	return ccs;
}

void cmdMidiToggleSelection(Command* command) {
	if (isSelectionContiguous) {
		isSelectionContiguous = false;
		outputMessage(translate("noncontiguous selection"));
		return;
	}
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	bool select;
	switch (fakeFocus) {
		case FOCUS_NOTE: {
			if (curNoteInChord != -1) {
				// Note in chord.
				MidiNote note = findNoteInChord(take, 0);
				if (note.channel == -1) {
					return;
				}
				select = !isNoteSelected(take, note.index);
				selectNote(take, note.index, select);
			} else {
				// Chord.
				auto chord = findChord(take, 0);
				if (chord.first == -1) {
					return;
				}
				select = !isNoteSelected(take, chord.first);
				for (int note = chord.first; note <= chord.second; ++note) {
					selectNote(take, note, select);
				}
			}
			break;
		}
		case FOCUS_CC: {
			if (currentCC == -1) {
				return;
			}
			select = !isCCSelected(take, currentCC);
			selectCC(take, currentCC, select);
			break;
		}
		default:
			return;
	}
	outputMessage(select ? translate("selected") : translate("unselected"));
}

void moveToChord(int direction, bool clearSelection=true, bool select=true) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	auto chord = findChord(take, direction);
	if (chord.first == -1)
		return;
	curNoteInChord = -1;
	if (clearSelection) {
		MIDIEditor_OnCommand(editor, 40214); // Edit: Unselect all
		isSelectionContiguous = true;
	}
	// Move the edit cursor to this chord, select it and play it.
	bool cursorSet = false;
	vector<MidiNote> notes;
	for (int note = chord.first; note <= chord.second; ++note) {
		double start, end;
		int chan, pitch, vel;
		MIDI_GetNote(take, note, NULL, NULL, &start, &end, &chan, &pitch, &vel);
		start = MIDI_GetProjTimeFromPPQPos(take, start);
		end = MIDI_GetProjTimeFromPPQPos(take, end);
		if (!cursorSet && direction != 0) {
			SetEditCurPos(start, true, false);
			cursorSet = true;
		}
		if (select)
			selectNote(take, note);
		notes.push_back({chan, pitch, vel, 0, start, end});
	}
	previewNotes(take, notes);
	fakeFocus = FOCUS_NOTE;
	ostringstream s;
	s << formatCursorPosition(TF_MEASURE) << " ";
	if (!select && !isNoteSelected(take, chord.first)) {
		s << translate("unselected") << " ";
	}
	if (shouldReportNotes) {
		int count = chord.second - chord.first + 1;
		s << count << (count == 1 ? " note" : " notes");
	}
	outputMessage(s);
}

void cmdMidiMoveToNextChord(Command* command) {
	moveToChord(1);
}

void cmdMidiMoveToPreviousChord(Command* command) {
	moveToChord(-1);
}

void cmdMidiMoveToNextChordKeepSel(Command* command) {
	moveToChord(1, false, isSelectionContiguous);
}

void cmdMidiMoveToPreviousChordKeepSel(Command* command) {
	moveToChord(-1, false, isSelectionContiguous);
}

void moveToNoteInChord(int direction, bool clearSelection=true, bool select=true) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	MidiNote note = findNoteInChord(take, direction);
	if (note.channel == -1)
		return;
	if (clearSelection) {
		MIDIEditor_OnCommand(editor, 40214); // Edit: Unselect all
		isSelectionContiguous = true;
	}
	if (select) {
		selectNote(take, note.index);
	}
	previewNotes(take, {note});
	fakeFocus = FOCUS_NOTE;
	ostringstream s;
	if (shouldReportNotes) {
		s << getMidiNoteName(take, note.pitch, note.channel);
	}
	if (!select && !isNoteSelected(take, note.index)) {
		s << " " << translate("unselected") << " ";
	} else if (shouldReportNotes) {
		s << ", ";
	}
	if (shouldReportNotes) {
		s << FormatNoteLength(note.start, note.end);
	}
	outputMessage(s);
}

void cmdMidiMoveToNextNoteInChord(Command* command) {
	moveToNoteInChord(1);
}

void cmdMidiMoveToPreviousNoteInChord(Command* command) {
	moveToNoteInChord(-1);
}

void cmdMidiMoveToNextNoteInChordKeepSel(Command* command) {
	moveToNoteInChord(1, false, isSelectionContiguous);
}

void cmdMidiMoveToPreviousNoteInChordKeepSel(Command* command) {
	moveToNoteInChord(-1, false, isSelectionContiguous);
}

void postMidiMovePitchCursor(int command) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	int pitch = MIDIEditor_GetSetting_int(editor, "active_note_row");
	int chan = MIDIEditor_GetSetting_int(editor, "default_note_chan");
	int vel = MIDIEditor_GetSetting_int(editor, "default_note_vel");
	previewNotes(take, {{chan, pitch, vel}});
	if (shouldReportNotes) {
		outputMessage(getMidiNoteName(take, pitch, chan));
	}
}

void cmdMidiInsertNote(Command* command) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	int oldCount;
	MIDI_CountEvts(take, &oldCount, nullptr, nullptr);
	MIDIEditor_OnCommand(editor, command->gaccel.accel.cmd);
	int newCount;
	MIDI_CountEvts(take, &newCount, nullptr, nullptr);
	if (newCount <= oldCount) {
		return; // Not inserted.
	}
	int pitch = MIDIEditor_GetSetting_int(editor, "active_note_row");
	// Get selected notes.
	vector<MidiNote> selectedNotes = getSelectedNotes(take);
	// Find the just inserted note based on its pitch, as that makes it unique.
	auto it = find_if(
		selectedNotes.begin(), selectedNotes.end(),
		[pitch](MidiNote n) { return n.pitch == pitch; }
	);
	if (it == selectedNotes.end()) {
		return;
	}
	auto& note = *it;
	// Play the inserted note when preview is enabled.
	previewNotes(take, {note});
	fakeFocus = FOCUS_NOTE;
	ostringstream s;
	if (shouldReportNotes) {
		s << getMidiNoteName(take, note.pitch, note.channel) << " ";
		s << FormatNoteLength(note.start, note.end);
		s << ", ";
	}
	s << formatCursorPosition(TF_MEASURE);
	outputMessage(s);
}

void cmdMidiDeleteEvents(Command* command) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	int oldCount = MIDI_CountEvts(take, NULL, NULL, NULL);
	MIDIEditor_OnCommand(editor, command->gaccel.accel.cmd);
	int removed = oldCount - MIDI_CountEvts(take, NULL, NULL, NULL);
	ostringstream s;
	s << removed << (removed == 1 ? " event" : " events") << " removed";
	outputMessage(s);
}

void postMidiSelectNotes(int command) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	int noteIndex=-1;
	int count=0;
	for(;;){
		noteIndex = MIDI_EnumSelNotes(take, noteIndex);
		if (noteIndex == -1)
			break;
		++count;
	}
	fakeFocus = FOCUS_NOTE;
	ostringstream s;
	s << count << " note" << ((count == 1) ? "" : "s") << " selected";
	outputMessage(s);
}

const string getMidiControlName(MediaItem_Take *take, int control, int channel) {
	static map<int, string> names = {
		{0, "Bank Select MSB"},
		{1, "Mod Wheel MSB"},
		{2, "Breath MSB"},
		{4, "Foot Pedal MSB"},
		{5, "Portamento MSB"},
		{6, "Data Entry MSB"},
		{7, "Volume MSB"},
		{8, "Balance MSB"},
		{10, "Pan Position MSB"},
		{11, "Expression MSB"},
		{12, "Control 1 MSB"},
		{13, "Control 2 MSB"},
		{16, "GP Slider 1"},
		{17, "GP Slider 2"},
		{18, "GP Slider 3"},
		{19, "GP Slider 4"},
		{32, "Bank Select LSB"},
		{33, "Mod Wheel LSB"},
		{34, "Breath LSB"},
		{36, "Foot Pedal LSB"},
		{37, "Portamento LSB"},
		{38, "Data Entry LSB"},
		{39, "Volume LSB"},
		{40, "Balance LSB"},
		{42, "Pan Position LSB"},
		{43, "Expression LSB"},
		{44, "Control 1 LSB"},
		{45, "Control 2 LSB"},
		{64, "Hold Pedal (on/off)"},
		{65, "Portamento (on/off)"},
		{66, "Sostenuto (on/off)"},
		{67, "Soft Pedal (on/off)"},
		{68, "Legato Pedal (on/off)"},
		{69, "Hold 2 Pedal (on/off)"},
		{70, "Sound Variation"},
		{71, "Timbre/Resonance"},
		{72, "Sound Release"},
		{73, "Sound Attack"},
		{74, "Brightness/Cutoff Freq"},
		{75, "Sound Control 6"},
		{76, "Sound Control 7"},
		{77, "Sound Controll 8"},
		{78, "Sound Control 9"},
		{79, "Sound Control 10"},
		{80, "GP Button 1 (on/off)"},
		{81, "GP Button 2 (on/off)"},
		{82, "GP Button 3 (on/off)"},
		{83, "GP Button 4 (on/off)"},
		{91, "Effects Level"},
		{92, "Tremolo Level"},
		{93, "Chorus Level"},
		{94, "Celeste Level"},
		{95, "Phaser Level"},
		{96, "Data Button Inc"},
		{97, "Data Button Dec"},
		{98, "Non-Reg Parm LSB"},
		{99, "Non-Reg Parm MSB"},
		{100, "Reg Parm LSB"},
		{101, "Reg Parm MSB"},
		{120, "All Sound Off"},
		{121, "Reset"},
		{122, "Local"},
		{123, "All Notes Off"},
		{124, "Omni On"},
		{125, "Omni Off"},
		{126, "Mono On"},
		{127, "Poly On"}
	};
	MediaTrack* track = GetMediaItemTake_Track(take);
	int tracknumber = static_cast<int> (GetMediaTrackInfo_Value(track, "IP_TRACKNUMBER")); // one based
	ostringstream s;
	const char* controlName = GetTrackMIDINoteName(tracknumber - 1, 128 + control, channel); // track number is zero based, controls start at 128
	s << control;
	if (controlName) {
		s << " (" << controlName << ")";
	} else {
		auto it = names.find(control);
		if (it != names.end()) {
			s << " (" << it->second << ")";
		}
	}
	return s.str();
}

void moveToCC(int direction, bool clearSelection=true, bool select=true) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	auto cc = findCC(take, direction);
	if (cc.channel == -1) {
		return;
	}
	if (clearSelection || select) {
		Undo_BeginBlock();
	}
	if (clearSelection) {
		MIDIEditor_OnCommand(editor, 40214); // Edit: Unselect all
		isSelectionContiguous = true;
	}
	if (select) {
		selectCC(take, cc.index);
	}
	if (clearSelection || select) {
		Undo_EndBlock("Change CC Selection", 0);
	}
	SetEditCurPos(cc.position, true, false);
	fakeFocus = FOCUS_CC;
	ostringstream s;
	s << formatCursorPosition(TF_MEASURE) << " ";
	if (cc.message1 == 0xA0) {
		s << "Poly Aftertouch ";
		// Note: separate the note and value with two spaces to avoid treatment as thousands separator.
		s << getMidiNoteName(take, cc.message2, cc.channel) << "  ";
		s << cc.message3;
	} else if (cc.message1 == 0xB0) {
		s << "Control ";
		s << getMidiControlName(take, cc.message2, cc.channel) << ", ";
		s << cc.message3;
	} else if (cc.message1 == 0xC0) {
		s << "Program " << cc.message2;
	} else if (cc.message1 == 0xD0) {
		s << "Channel pressure " << cc.message2;
	} else if (cc.message1 == 0xE0) {
		auto pitchBendValue = (cc.message3 << 7) | cc.message2;
		s << "Pitchhhh Bend " << pitchBendValue;
	}
	if (!select && !isCCSelected(take, cc.index)) {
		s << "unselected" << " ";
	}
	outputMessage(s);
}

void cmdMidiMoveToNextCC(Command* command) {
	moveToCC(1);
}

void cmdMidiMoveToPreviousCC(Command* command) {
	moveToCC(-1);
}

void cmdMidiMoveToNextCCKeepSel(Command* command) {
	moveToCC(1, false, isSelectionContiguous);
}

void cmdMidiMoveToPreviousCCKeepSel(Command* command) {
	moveToCC(-1, false, isSelectionContiguous);
}

void midiMoveToItem(int direction) {
	HWND editor = MIDIEditor_GetActive();
	MIDIEditor_OnCommand(editor, ((direction==1)?40798:40797)); // Contents: Activate next/previous MIDI media item on this track, clearing the editor first
	MIDIEditor_OnCommand(editor, 40036); // View: Go to start of file
	int cmd = NamedCommandLookup("_FNG_ME_SELECT_NOTES_NEAR_EDIT_CURSOR");
	if(cmd>0)
		MIDIEditor_OnCommand(editor, cmd); // SWS/FNG: Select notes nearest edit cursor
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	MediaItem* item = GetMediaItemTake_Item(take);
	MediaTrack* track = GetMediaItem_Track(item);
	int count = CountTrackMediaItems(track);
	int itemNum;
	for (int i=0; i<count; ++i) {
		MediaItem* itemTmp = GetTrackMediaItem(track, i);
		if (itemTmp == item) {
			itemNum = i+1;
			break;
		}
	}
	fakeFocus = FOCUS_ITEM;
	ostringstream s;
	s << itemNum << " " << GetTakeName(take);
	s << " " << formatCursorPosition();
	outputMessage(s);
}

void cmdMidiMoveToNextItem(Command* command) {
	Undo_BeginBlock();
	midiMoveToItem(1);
	Undo_EndBlock("OSARA: Move to next midi item on track", 0);
}

void cmdMidiMoveToPrevItem(Command* command) {
	Undo_BeginBlock();
	midiMoveToItem(-1);
	Undo_EndBlock("OSARA: Move to previous midi item on track", 0);
}

void cmdMidiMoveToTrack(Command* command) {
	HWND editor = MIDIEditor_GetActive();
	MIDIEditor_OnCommand(editor, command->gaccel.accel.cmd);
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	MediaItem* item = GetMediaItemTake_Item(take);
	MediaTrack* track = GetMediaItem_Track(item);
	int count = CountTrackMediaItems(track);
	int itemNum;
	for (int i=0; i<count; ++i) {
		MediaItem* itemTmp = GetTrackMediaItem(track, i);
		if (itemTmp == item) {
			itemNum = i+1;
			break;
		}
	}
	fakeFocus = FOCUS_TRACK;
	ostringstream s;
	int trackNum = (int)(size_t)GetSetMediaTrackInfo(track, "IP_TRACKNUMBER", NULL);
	s << trackNum;
	char* trackName = (char*)GetSetMediaTrackInfo(track, "P_NAME", NULL);
	if (trackName)
		s << " " << trackName;
	s << " item " << itemNum << " " << GetTakeName(take);
	outputMessage(s);
}

void cmdMidiSelectSamePitchStartingInTimeSelection(Command* command) {
	double tsStart,tsEnd;
	GetSet_LoopTimeRange(false, false, &tsStart, &tsEnd, false);
	if(tsStart == tsEnd) {
		outputMessage(translate("no time selection"));
		return;
	}
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	int selNote = MIDI_EnumSelNotes(take, -1);
	if(selNote==-1) {
		outputMessage(translate("no notes selected"));
		return;
	}
	int selPitch;
	MIDI_GetNote(take, selNote, nullptr, nullptr, nullptr, nullptr, nullptr, &selPitch, nullptr);
	Undo_BeginBlock();
	MIDIEditor_OnCommand(editor, 40214); // Edit: Unselect all
	int noteCount {0}, selectCount {0};
	MIDI_CountEvts(take, &noteCount, nullptr, nullptr);
	for(int i=0; i<noteCount; i++) {
		double startPPQPos;
		int pitch;
		MIDI_GetNote(take, i, nullptr, nullptr, &startPPQPos, nullptr, nullptr, &pitch, nullptr);
		double startTime = MIDI_GetProjTimeFromPPQPos(take, startPPQPos);
		if(tsStart<=startTime && startTime<tsEnd && pitch==selPitch) {
			selectNote(take, i);
			selectCount++;
		}
	}
	Undo_EndBlock("OSARA: Select all notes with the same pitch within time selection",0);
	ostringstream s;
	s<< selectCount << " note"<<((selectCount==1)?"":"s")<<" selected";
	outputMessage(s);
}

void cmdMidiNoteSplitOrJoin(Command* command) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	// Get selected note count before action.
	auto oldCount = countSelectedNotes(take);
	auto cmdId = command->gaccel.accel.cmd;
	MIDIEditor_OnCommand(editor, cmdId);
	auto newCount = countSelectedNotes(take);
	if (oldCount == newCount) {
		return;
	}
	ostringstream s;
	s << oldCount << " notes ";
	switch (cmdId) {
		case 40046:
			s << "split";
			break;
		case 40456:
			s << "joined";
			break;
		default:
			s << "transformed";
			break;
	}
	s << " into " << newCount;
	outputMessage(s);
}

#ifdef _WIN32
void cmdFocusNearestMidiEvent(Command* command) {
	HWND focus = GetFocus();
	if (!focus)
		return;
	double cursorPos = GetCursorPosition();
	for (int i = 0; i < ListView_GetItemCount(focus); ++i) {
		char text[50];
		// Get the text from the position column (1).
		ListView_GetItemText(focus, i, 1, text, sizeof(text));
		// Convert this to project time. text is always in measures.beats.
		double eventPos = parse_timestr_pos(text, 2);
		if (eventPos >= cursorPos) {
			// This item is at or just after the cursor.
			int oldFocus = ListView_GetNextItem(focus, -1, LVNI_FOCUSED);
			// Focus and select this item.
			ListView_SetItemState(focus, i,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			ListView_EnsureVisible (focus, i, false);
			if (oldFocus != -1 && oldFocus != i) {
				// Unselect the previously focused item.
				ListView_SetItemState(focus, oldFocus,
					0, LVIS_SELECTED);
			}
			break;
		}
	}
}

void cmdMidiFilterWindow(Command *command) {
	HWND editor = MIDIEditor_GetActive();
	MIDIEditor_OnCommand(editor, command->gaccel.accel.cmd);
	// TODO: we could also check the command state was "off", to skip searching otherwise
	HWND filter = FindWindow(WC_DIALOG, "Filter Events");
	if (filter && (filter != GetFocus())) {
		SetFocus(filter); // focus the window
	}
}

void maybePreviewCurrentNoteInEventList(HWND hwnd) {
	if (!GetToggleCommandState2(SectionFromUniqueID(MIDI_EVENT_LIST_SECTION), 40041)) {  // Options: Preview notes when inserting or editing
		return;
	}
	auto focused = ListView_GetNextItem(hwnd, -1, LVNI_FOCUSED);
	char text[50] = "\0";
	// Get the text from the length column (2).
	// If this column is empty, we aren't dealing with a note.
	ListView_GetItemText(hwnd, focused, 2, text, sizeof(text));
	if (!text[0]) {
		return;
	}
	MidiNote note;
	// Convert length to project time. text is always in measures.beats.
	auto length = parse_timestr_len(text, 0, 2);
	text[0] = '\0';
	// Get the text from the position column (1).
	ListView_GetItemText(hwnd, focused, 1, text, sizeof(text));
	// Convert this to project time. text is always in measures.beats.
	note.start = parse_timestr_pos(text, 2);
	note.end = note.start + length;
	text[0] = '\0';
	// Get the text from the channel column (3).
	ListView_GetItemText(hwnd, focused, 3, text, sizeof(text));
	note.channel = atoi(text) -1;
	text[0] = '\0';
	// Get the text from the parameter (note name) column (5).
	ListView_GetItemText(hwnd, focused, 5, text, sizeof(text));
	string noteNameWithOctave { text };
	static const string noteNames[] = {"C", "C#", "D", "D#", "E", "F",
		"F#", "G", "G#", "A", "A#", "B"};
	// Loop through noteNames in reverse order so the sharp notes are handled first.
	bool found = false;
	for (int i = static_cast<int>(size(noteNames)) -1; i >= 0; --i) {
		auto noteNameLength = noteNames[i].length();
		if (noteNameWithOctave.compare(0, noteNameLength, noteNames[i]) != 0) {
			continue;
		}
		found = true;
		// The octave is a number in the range -1 up and until 9.
		// Therefore, the number counts either one or two characters in noteNameWithOctave.
		// If the note is explicitly named, the name comes after the octave and a space.
		// As stoi simply ignores whitespace or the end of a string, all possible appearances should be covered.
		int octave;
		try {
			octave = stoi(noteNameWithOctave.substr(noteNameLength, 2));
		} catch (invalid_argument) {
			// If a REAPER language pack translates note names, we might get something
			// unexpected here. There's nothing we can do to compensate, so just
			// gracefully ignore this.
			continue;
		}
		note.pitch = ((octave + 1) * 12) + i;
		break;
	}
	if (!found) {
		return; // Note not found.
	}
	text[0] = '\0';
	// Get the text from the value (velocity) column (6).
	ListView_GetItemText(hwnd, focused, 6, text, sizeof(text));
	note.velocity = atoi(text);
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	previewNotes(take, {note});
}

#endif // _WIN32

void postMidiChangeVelocity(int command) {
	if (!shouldReportNotes) {
		return;
	}
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	// Get selected notes.
	vector<MidiNote> selectedNotes = getSelectedNotes(take);
	if (selectedNotes.size() == 0) {
		return;
	}
	bool generalize = false;
	if (selectedNotes.size() >= 8) {
		generalize = true;
	} else {
		// Get indexes for the current chord.
		auto chord = findChord(take, 0);
		if (chord.first == -1) {
			generalize = true;
		} else {
			generalize = !(all_of(
				selectedNotes.begin(), selectedNotes.end(),
				[chord](MidiNote n) { return chord.first <= n.index && n.index <= chord.second; }
			));
		}
	}
	// The Reaper action takes care of note preview.
	ostringstream s;
	if (generalize) {
		auto count = selectedNotes.size();
		s << count << (count == 1 ? " note" : " notes ");
		switch (command) {
			case 40462:
				s << "velocity +1";
				break;
			case 40463:
				s << "velocity +10";
				break;
			case 40464:
				s << "velocity -1";
				break;
			case 40465:
				s << "velocity -10";
				break;
			default:
				s << "velocity changed";
				break;
		}
	} else{
		for (auto note = selectedNotes.cbegin(); note != selectedNotes.cend(); ++note) {
			s << getMidiNoteName(take, note->pitch, note->channel) << "  " << note->velocity;
			if (note != selectedNotes.cend() - 1) {
				s << ", ";
			}
		}
	}
	outputMessage(s);
}

void postMidiChangeLength(int command) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);	
	// Get selected notes.
	vector<MidiNote> selectedNotes = getSelectedNotes(take);
	if (selectedNotes.size() == 0) {
		return;
	}
	if (command == 40765 && selectedNotes.size() == 1) {
		// Making notes legato doesn't do anything when only one note is selected.
		return;
	}
	bool generalize = false;
	if (selectedNotes.size() >= 8) {
		generalize = true;
	} else {
		// Get indexes for the current chord.
		auto chord = findChord(take, 0);
		if (chord.first == -1) {
			generalize = true;
		} else {
			generalize = !(all_of(
				selectedNotes.begin(), selectedNotes.end(),
				[chord](MidiNote n) { return chord.first <= n.index && n.index <= chord.second; }
			));
		}
	}
	if (!generalize) {
		previewNotes(take, selectedNotes);
	}
	if (shouldReportNotes) {
		ostringstream s;
		if (generalize) {
			auto count = selectedNotes.size();
			s << count << (count == 1 ? " note" : " notes ");
			switch (command) {
				case 40444:
					s << "lengthened pixel";
					break;
				case 40445:
					s << "shortened pixel";
					break;
				case 40446:
					s << "lengthened grid unit";
					break;
				case 40447:
					s << "shortened grid unit";
					break;
				case 40633:
					s << "length set to grid size";
					break;
				case 40765:
					s << "made legato";
					break;
				default:
					s << "length changed";
					break;
			}
		} else{ 
			for (auto note = selectedNotes.cbegin(); note != selectedNotes.cend(); ++note) {
				s << getMidiNoteName(take, note->pitch, note->channel) << " ";
				s << FormatNoteLength(note->start, note->end);
				if (note != selectedNotes.cend() - 1) {
					s << ", ";
				}
			}
		}
		outputMessage(s);
	}
}

void postMidiChangePitch(int command) {
	if (!shouldReportNotes) {
		return;
	}
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);	
	// Get selected notes.
	vector<MidiNote> selectedNotes = getSelectedNotes(take);
	if (selectedNotes.size() == 0) {
		return;
	}
	bool generalize = false;
	if (selectedNotes.size() >= 8) {
		generalize = true;
	} else {
		// Get indexes for the current chord.
		auto chord = findChord(take, 0);
		if (chord.first == -1) {
			generalize = true;
		} else {
			generalize = !(all_of(
				selectedNotes.begin(), selectedNotes.end(),
				[chord](MidiNote n) { return chord.first <= n.index && n.index <= chord.second; }
			));
		}
	}
	// The Reaper action takes care of note preview.
	ostringstream s;
	if (generalize) {
		auto count = selectedNotes.size();
		s << count << (count == 1 ? " note" : " notes ");
		switch (command) {
			case 40177:
				s << "semitone up";
				break;
			case 40178:
				s << "semitone down";
				break;
			case 40179:
				s << "octave up";
				break;
			case 40180:
				s << "octave down";
				break;
			case 41026:
				s << "semitone up ignoring scale";
				break;
			case 41027:
				s << "semitone down ignoring scale";
				break;
			default:
				s << "pitch changed";
				break;
		}
	} else{ 
		for (auto note = selectedNotes.cbegin(); note != selectedNotes.cend(); ++note) {
			s << getMidiNoteName(take, note->pitch, note->channel);
			if (note != selectedNotes.cend() - 1) {
				s << ", ";
			}
		}
	}
	outputMessage(s);
}

void postMidiMoveStart(int command) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	// Get selected notes.
	vector<MidiNote> selectedNotes = getSelectedNotes(take);
	auto count = selectedNotes.size();
	if (count == 0) {
		return;
	}
	auto firstStart = selectedNotes.cbegin()->start;
	bool generalize = count >= 8 || any_of(
		selectedNotes.begin(), selectedNotes.end(),
		[firstStart](MidiNote n) { return firstStart != n.start; }
		);
	if (!generalize) {
		previewNotes(take, selectedNotes);
	}
	if (shouldReportNotes) {
		ostringstream s;
		if (generalize) {
			s << count << (count == 1 ? " note" : " notes ");
			switch (command) {
				case 40181:
					s << "pixel left";
					break;
				case 40182:
					s << "pixel right";
					break;
				case 40183:
					s << "grid unit left";
					break;
				case 40184:
					s << "grid unit right";
					break;				
				default:
					s << "start moved";
					break;
			}
		} else{ 
			for (auto note = selectedNotes.cbegin(); note != selectedNotes.cend(); ++note) {
				if (note == selectedNotes.cbegin()) {
					s << formatTime(note->start, TF_MEASURE) << " ";
				}
				s << getMidiNoteName(take, note->pitch, note->channel);
				if (note != selectedNotes.cend() - 1) {
					s << ", ";
				}
			}
		}
		outputMessage(s);
	}
}

void postMidiChangeCCValue(int command) {
	HWND editor = MIDIEditor_GetActive();
	MediaItem_Take* take = MIDIEditor_GetTake(editor);
	// Get selected CCs.
	vector<MidiControlChange> selectedCCs = getSelectedCCs(take);
	auto count = selectedCCs.size();
	if (count == 0) {
		return;
	}
	ostringstream s;
	if (count > 1) {
		s << count << " values ";
		switch (command) {
			case 40676: {
				s << "increased";
				break;
			}
			case 40677: {
				s << "decreased";
				break;
			}
			default: {
				s << "changed";
				break;
			}
		}
	} else{ 
		auto cc = *selectedCCs.cbegin();
		if (cc.message1 == 0xA0) {
			// Note: separate the note and value with two spaces to avoid treatment as thausands separator.
			s << getMidiNoteName(take, cc.message2, cc.channel) << "  ";
			s << cc.message3;
		} else if (cc.message1 == 0xB0) {
			s << cc.message3;
		} else if (cc.message1 == 0xC0 || cc.message1 == 0xD0) {
			s << cc.message2;
		} else if (cc.message1 == 0xE0) {
			auto pitchBendValue = (cc.message3 << 7) | cc.message2;
			s << pitchBendValue;
		}
	}
	outputMessage(s);
}

void postMidiSwitchCCLane(int command) {
	HWND editor = MIDIEditor_GetActive();
	ostringstream s;
	int ccNum = MIDIEditor_GetSetting_int(editor, "last_clicked_cc_lane");
	if (ccNum < 128) {
		s << ccNum << " ";
	}
	const int BUFFER_LENGTH = 64;
	char textBuffer[BUFFER_LENGTH];
	MIDIEditor_GetSetting_str(editor, "last_clicked_cc_lane", textBuffer, BUFFER_LENGTH);
	s << textBuffer;
	outputMessage(s);
}

void postToggleMidiInputsAsStepInput(int command) {
	ostringstream s;
	s << (GetToggleCommandState2(SectionFromUniqueID(MIDI_EDITOR_SECTION),
		command) ? "enabled" : "disabled");
	s << " MIDI inputs as step input";
	outputMessage(s);
}

void postToggleFunctionKeysAsStepInput(int command) {
	ostringstream s;
	s << (GetToggleCommandState2(SectionFromUniqueID(MIDI_EDITOR_SECTION),
		command) ? "enabled" : "disabled");
	s << " f1-f12 as step input";
	outputMessage(s);
}
