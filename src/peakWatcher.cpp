/*
 * OSARA: Open Source Accessibility for the REAPER Application
 * Peak Watcher code
 * Author: James Teh <jamie@jantrid.net>
 * Copyright 2015-2021 NV Access Limited, James Teh
 * License: GNU General Public License version 2.0
 */

#include <math.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
// osara.h includes windows.h, which must be included before other Windows
// headers.
#include "osara.h"
#ifdef _WIN32
#include <Commctrl.h>
#include <Windowsx.h>
#endif
#include <WDL/win32_utf8.h>
#include <WDL/db2val.h>
#include "resource.h"
#include "translation.h"

using namespace std;

const int PW_NUM_TRACKS = 2;
const int PW_NUM_CHANNELS = 2;
struct {
	MediaTrack* track;
	bool follow;
	unsigned int levelType;
	struct {
		double peak;
		DWORD time;
	} channels[PW_NUM_CHANNELS];
} pw_tracks[PW_NUM_TRACKS] = {};
// What the user can choose to watch.
enum {
	PWT_DISABLED,
	PWT_FOLLOW,
	PWT_MASTER,
	PWT_TRACKS_START,
};
bool pw_notifyChannels[PW_NUM_CHANNELS] = {true, true};
double pw_level = 0;
int pw_hold = 0;
int pw_numTracksEnabled = 0;
UINT_PTR pw_timer = 0;

// Get a specific parameter from a specific named effect. If the effect isn't
// present, it will be added.
double getSpecificTrackFxParam(MediaTrack* track, const char* fxName,
	int param
) {
	int fx = TrackFX_AddByName(track, fxName, /* recFX */ false,
		1 /* create if not found */);
	if (fx == -1) {
		return -150.0;
	}
	return TrackFX_GetParam(track, fx, param, nullptr, nullptr);
}

void deleteTrackFx(MediaTrack* track, const char* fxName) {
	int fx = TrackFX_AddByName(track, fxName, /* recFX */ false,
		0 /* don't create */);
	if (fx != -1) {
		TrackFX_Delete(track, fx);
	}
}

const char FX_EBUR128[] = "ebur128_analysis";

void deleteEbur128(MediaTrack* track) {
	deleteTrackFx(track, FX_EBUR128);
}

// Level types.
const struct {
	const char* name;
	double (*getLevel)(MediaTrack* track, int channel);
	bool separateChannels;
	void (*reset)(MediaTrack* track);
} PW_LEVEL_TYPES[] = {
	// translate firstString begin
	{"peak dB",
		/* getValue */ [](MediaTrack* track, int channel) {
			// #119: We use Track_GetPeakHoldDB even when Peak Watcher's hold
			//  functionality is disabled because we only measure every 30 ms and we
			// might miss peaks.
			// Undocumented: Track_GetPeakHoldDB returns hundredths of a dB.
			double newPeak = Track_GetPeakHoldDB(track, channel, false) * 100.0;
			// We have the peak now, so reset REAPER's hold.
			Track_GetPeakHoldDB(track, channel, true);
			return newPeak;
		},
		/* separateChannels */ true,
		/* reset */ nullptr,
	},
	{"integrated LUFS",
		/* getValue */ [](MediaTrack* track, int channel) {
			return getSpecificTrackFxParam(track, FX_EBUR128, 8);
		},
		/* separateChannels */ false,
		/* reset */ deleteEbur128,
	},
	{"momentary LUFS",
		/* getValue */ [](MediaTrack* track, int channel) {
			return getSpecificTrackFxParam(track, FX_EBUR128, 9);
		},
		/* separateChannels */ false,
		/* reset */ deleteEbur128,
	},
	{"short term LUFS",
		/* getValue */ [](MediaTrack* track, int channel) {
			return getSpecificTrackFxParam(track, FX_EBUR128, 12);
		},
		/* separateChannels */ false,
		/* reset */ deleteEbur128,
	},
	// translate firstString end
};
constexpr unsigned int PW_NUM_LEVEL_TYPES = sizeof(PW_LEVEL_TYPES) /
	sizeof(PW_LEVEL_TYPES[0]);

void pw_resetTrack(int trackIndex, bool report=false) {
	auto& pwTrack = pw_tracks[trackIndex];
	for (int c = 0; c < PW_NUM_CHANNELS; ++c) {
		pwTrack.channels[c].peak = -150;
	}
	auto& levelType = PW_LEVEL_TYPES[pwTrack.levelType];
	if (pwTrack.track && levelType.reset) {
		levelType.reset(pwTrack.track);
	}
	if (report) {
		if (!pwTrack.track && !pwTrack.follow) {
			// Translators: Reported when the user tries to reset a Peak Watcher
			// track, but that Peak Watcher track is disabled.
			outputMessage(translate("Peak Watcher track disabled"));
		} else {
			// Translators: Reported when the user resets a Peak Watcher track.
			outputMessage(translate("reset"));
		}
	}
}

void CALLBACK pw_watcher(HWND hwnd, UINT msg, UINT_PTR event, DWORD time) {
	ostringstream s;
	s << fixed << setprecision(1);
	for (int t = 0; t < PW_NUM_TRACKS; ++t) {
		auto& pwTrack = pw_tracks[t];
		auto& levelType = PW_LEVEL_TYPES[pwTrack.levelType];
		if (!pwTrack.track && !pwTrack.follow)
			continue; // Disabled.
		MediaTrack* currentTrack = GetLastTouchedTrack();
		if (pwTrack.follow && pwTrack.track != currentTrack) {
			// We're following the current track and it changed.
			pw_resetTrack(t);
			pwTrack.track = currentTrack;
			if (!currentTrack)
				continue; // No current track, so nothing to do.
		}

		bool trackReported = false;
		// If this level type doesn't care about separate channels, we only need
		// to process one channel.
		const int numChannels = levelType.separateChannels ?
			PW_NUM_CHANNELS : 1;
		for (int c = 0; c < numChannels; ++c) {
			auto& pwChan = pwTrack.channels[c];
			double newPeak = levelType.getLevel(pwTrack.track, c) ;
			if (pw_hold == -1 // Hold disabled
				|| newPeak > pwChan.peak
				|| (pw_hold != 0 && time > pwChan.time + pw_hold)
			) {
				pwChan.peak = newPeak;
				pwChan.time = time;
				if (pw_notifyChannels[c] && newPeak > pw_level) {
					if (pw_numTracksEnabled > 1 && !trackReported) {
						// Only report the track name if watching more than one track
						// and a channel actually changed for this track.
						int trackNum = (int)(size_t)GetSetMediaTrackInfo(pwTrack.track, "IP_TRACKNUMBER", NULL);
						if (trackNum <= 0) {
							s << translate("master") << " ";
						} else {
							// Translators: used when Peak Watcher notifies about a peak.
							// {} will be replaced with the track number; e.g. "track 2".
							s << format(translate("track {}"), trackNum) << " ";
						}
					}
					if (levelType.separateChannels) {
						// Translators: used when Peak Watcher notifies about a peak. This is
						// placed after the track and before the peak value.
						// {} will be replaced with the channel number; e.g. "chan 2".
						s << format(translate("chan {}:"), c + 1);
					} else {
						// Translators: used when Peak Watcher notifies about a peak for cases
						// where the channel is irrelevant. This is placed after the track and
						// before the peak value.
						s << translate("level");
					}
					s << " " << newPeak;
					outputMessage(s);
				}
			}
		}
		if (!levelType.separateChannels) {
			// Copy the value to the other channels for on-demand reporting.
			for (int c = 1; c < PW_NUM_CHANNELS; ++c) {
				pwTrack.channels[c].peak = pwTrack.channels[0].peak;
			}
		}
	}
}

void pw_start() {
	pw_timer = SetTimer(nullptr, 0, 30, pw_watcher);
}

void pw_stop() {
	KillTimer(nullptr, pw_timer);
	pw_timer = 0;
}

void pw_onOk(HWND dialog) {
	// Retrieve the notification state for channels.
	for (int c = 0; c < PW_NUM_CHANNELS; ++c) {
		pw_notifyChannels[c] = IsDlgButtonChecked(dialog, ID_PEAK_CHAN1 + c) == BST_CHECKED;
	}

	char inText[7];
	// Retrieve the entered maximum level.
	if (GetDlgItemText(dialog, ID_PEAK_LEVEL, inText, sizeof(inText)) > 0) {
		pw_level = atof(inText);
		// Restrict the range.
		pw_level = max(min(pw_level, 40.0), -40.0);
	}

	// Retrieve the hold choice/time.
	if (IsDlgButtonChecked(dialog, ID_PEAK_HOLD_DISABLED) == BST_CHECKED)
		pw_hold = -1;
	else if (IsDlgButtonChecked(dialog, ID_PEAK_HOLD_FOREVER) == BST_CHECKED)
		pw_hold = 0;
	else if (GetDlgItemText(dialog, ID_PEAK_HOLD_TIME, inText, sizeof(inText)) > 0) {
		pw_hold = atoi(inText);
		// Restrict the range.
		pw_hold = max(min(pw_hold, 20000), 1);
	}

	// Set up according to what tracks the user chose to watch.
	// If there was a change, reset.
	pw_numTracksEnabled = 0;
	for (int t = 0; t < PW_NUM_TRACKS; ++t) {
		auto& pwTrack = pw_tracks[t];
		HWND typeSel = GetDlgItem(dialog, ID_PEAK_TYPE1 + t);
		unsigned int newType = ComboBox_GetCurSel(typeSel);
		bool typeChanged = newType != pwTrack.levelType;
		// Don't set pwTrack.levelType here because we might need to reset tracks,
		// which depends on knowing the old type. We'll set it later.

		HWND trackSel = GetDlgItem(dialog, ID_PEAK_TRACK1 + t);
		int sel = ComboBox_GetCurSel(trackSel);

		if (sel == PWT_DISABLED) {
			// Disabled.
			pw_resetTrack(t);
			pwTrack.track = NULL;
			pwTrack.follow = false;
			continue;
		}
		++pw_numTracksEnabled;
		auto handleTypeChanged = [&] {
			if (typeChanged) {
				pw_resetTrack(t);
				pwTrack.levelType = newType;
			}
		};
		if (sel == PWT_FOLLOW) {
			// Follow current track.
			handleTypeChanged();
			if (pwTrack.follow) {
				continue; // Already following.
			}
			// handleTypeChanged might have already reset.
			if (!typeChanged) {
				pw_resetTrack(t);
			}
			pwTrack.follow = true;
			pwTrack.track = NULL;
			continue;
		}
		MediaTrack* track;
		if (sel == PWT_MASTER)
			track = GetMasterTrack(0);
		else // sel >= PWT_TRACKS_START
			track = GetTrack(0, sel - PWT_TRACKS_START);
		handleTypeChanged();
		if (pwTrack.track != track) {
			// handleTypeChanged might have already reset.
			if (!typeChanged) {
				pw_resetTrack(t);
			}
			pwTrack.track = track;
			pwTrack.follow = false;
		}
	}

	if (pw_numTracksEnabled == 0 && pw_timer) {
		// Peak watcher disabled completely.
		pw_stop();
	} else if (!pw_timer) { // Previously disabled or paused.
		pw_start();
	}
}

INT_PTR CALLBACK pw_dialogProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_COMMAND: {
			int id = LOWORD(wParam);
			if (ID_PEAK_HOLD_DISABLED <= id && id <= ID_PEAK_HOLD_FOR) {
				EnableWindow(GetDlgItem(dialog, ID_PEAK_HOLD_TIME),
					id == ID_PEAK_HOLD_FOR ? BST_CHECKED : BST_UNCHECKED);
			} else if (id == ID_PEAK_RESET) {
				for (int t = 0; t < PW_NUM_TRACKS; ++t)
					pw_resetTrack(t);
				DestroyWindow(dialog);
				return TRUE;
			} else if (id == IDOK) {
				pw_onOk(dialog);
				DestroyWindow(dialog);
				return TRUE;
			} else if (id == IDCANCEL) {
				DestroyWindow(dialog);
				return TRUE;
			}
			break;
		}
		case WM_CLOSE:
			DestroyWindow(dialog);
			return TRUE;
	}
	return FALSE;
}

void cmdPeakWatcher(Command* command) {
	int trackCount = CountTracks(0);
	if (trackCount == 0)
		return;
	ostringstream s;
	HWND dialog = CreateDialog(pluginHInstance, MAKEINTRESOURCE(ID_PEAK_WATCHER_DLG), mainHwnd, pw_dialogProc);
	translateDialog(dialog);

	for (int pwt = 0; pwt < PW_NUM_TRACKS; ++pwt) {
		HWND trackSel = GetDlgItem(dialog, ID_PEAK_TRACK1 + pwt);
		WDL_UTF8_HookComboBox(trackSel);
		auto& pwTrack = pw_tracks[pwt];
		// Populate the list of what to watch.
		ComboBox_AddString(trackSel, "None");
		if (!pwTrack.follow && !pwTrack.track)
			ComboBox_SetCurSel(trackSel, PWT_DISABLED);
		ComboBox_AddString(trackSel, "Follow current track");
		if (pwTrack.follow)
			ComboBox_SetCurSel(trackSel, PWT_FOLLOW);
		ComboBox_AddString(trackSel, "Master");
		MediaTrack* track = GetMasterTrack(0);
		if (pwTrack.track == track)
			ComboBox_SetCurSel(trackSel, PWT_MASTER);
		MediaTrack* currentTrack = GetLastTouchedTrack();
		for (int t = 0; t < trackCount; ++t) {
			track = GetTrack(0, t);
			if (track == currentTrack) {
				// Translators: Used in the lists of tracks in the Peak Watcher dialog
				// to indicate the current track.
				s << translate("current") << " ";
			}
			s << (int)(size_t)GetSetMediaTrackInfo(track, "IP_TRACKNUMBER", NULL);
			char* name;
			if ((name = (char*)GetSetMediaTrackInfo(track, "P_NAME", nullptr))) {
				s << ": " << name;
			}
			ComboBox_AddString(trackSel, s.str().c_str());
			s.str("");
			if (!pwTrack.follow && pwTrack.track == track)
				ComboBox_SetCurSel(trackSel, PWT_TRACKS_START + t);
		}

		HWND typeSel = GetDlgItem(dialog, ID_PEAK_TYPE1 + pwt);
		for (unsigned int type = 0; type < PW_NUM_LEVEL_TYPES; ++type) {
			WDL_UTF8_HookComboBox(typeSel);
			ComboBox_AddString(typeSel, translate(PW_LEVEL_TYPES[type].name));
		}
		ComboBox_SetCurSel(typeSel, pwTrack.levelType);
	}

	for (int c = 0; c < PW_NUM_CHANNELS; ++c) {
		CheckDlgButton(dialog, ID_PEAK_CHAN1 + c, pw_notifyChannels[c] ? BST_CHECKED : BST_UNCHECKED);
	}

	HWND level = GetDlgItem(dialog, ID_PEAK_LEVEL);
#ifdef _WIN32
	SendMessage(level, EM_SETLIMITTEXT, 6, 0);
#endif
	s << fixed << setprecision(2);
	s << pw_level;
	SetWindowText(level, s.str().c_str());
	s.str("");

	HWND holdTime = GetDlgItem(dialog, ID_PEAK_HOLD_TIME);
#ifdef _WIN32
	SendMessage(holdTime, EM_SETLIMITTEXT, 5, 0);
#endif
	int id;
	if (pw_hold == -1)
		id = ID_PEAK_HOLD_DISABLED;
	else if (pw_hold == 0)
		id = ID_PEAK_HOLD_FOREVER;
	else {
		id = ID_PEAK_HOLD_FOR;
		s << pw_hold;
		SetWindowText(holdTime, s.str().c_str());
	}
	CheckDlgButton(dialog, id, BST_CHECKED);
	EnableWindow(holdTime, pw_hold > 0);

	ShowWindow(dialog, SW_SHOWNORMAL);
}

void pw_report(int trackIndex, int channel) {
	assert(trackIndex < PW_NUM_TRACKS);
	assert(channel < PW_NUM_CHANNELS);
	auto& pwTrack = pw_tracks[trackIndex];
	if (!pwTrack.track && !pwTrack.follow) {
		// Translators: Reported when the user tries to report a Peak Watcher
		// channel, but the Peak Watcher track is disabled.
		outputMessage(translate("Peak Watcher track disabled"));
		return;
	}
	if (!pw_timer) {
		// Translators: Reported when the user tries to report a Peak Watcher
		// channel, but the Peak Watcher is paused.
		outputMessage(translate("Peak Watcher paused"));
		return;
	}
	ostringstream s;
	s << fixed << setprecision(1);
	s << pwTrack.channels[channel].peak;
	outputMessage(s);
}

void cmdReportPeakWatcherT1C1(Command* command) {
	pw_report(0, 0);
}

void cmdReportPeakWatcherT1C2(Command* command) {
	pw_report(0, 1);
}

void cmdReportPeakWatcherT2C1(Command* command) {
	pw_report(1, 0);
}

void cmdReportPeakWatcherT2C2(Command* command) {
	pw_report(1, 1);
}

void cmdResetPeakWatcherT1(Command* command) {
	pw_resetTrack(0, true);
}

void cmdResetPeakWatcherT2(Command* command) {
	pw_resetTrack(1, true);
}

void cmdPausePeakWatcher(Command* command) {
	if (pw_timer) {
		// Running.
		pw_stop();
		outputMessage(translate("paused Peak Watcher"));
	} else if (pw_numTracksEnabled > 0) {
		// Paused.
		pw_start();
		outputMessage(translate("resumed Peak Watcher"));
	} else {
		// Disabled.
		outputMessage(translate("Peak Watcher not enabled"));
	}
}
