/*
 *  prefs_editor_atari.cpp - Preferences editor, Atari implementation
 *
 *  Basilisk II (C) 1997-2008 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"
#include "prefs.h"
#include "prefs_editor.h"
#include "user_strings.h"
#include "gem.h"
#include "basilisk.h"
#include "video_atari.h"
#include "vdi_userdef.h"
#include "mint/mintbind.h"
#include "mint/cookie.h"
#include "version.h"

#include <string.h>

#define DEBUG 0
#include "debug.h"

extern int HostCPUType;
extern int HostFPUType;
extern int CPUType;

extern OBJECT* RscMenuPtr;
static OBJECT* menu = NULL;
static OBJECT* formMain = NULL;
static bool haveYM = false;
static bool haveDMA = false;


int16 objc_get_parent(OBJECT* tree, int16 obj)
{
 	if (obj ==0)
 		return 0;
	int16 pobj = tree[obj].ob_next;
	if (pobj != 0)
	{
		while (tree[pobj].ob_tail != obj)
		{
			obj = pobj;
			pobj = tree[obj].ob_next;
		}
	}
	return pobj;
}

void objc_check(OBJECT* tree, int16 obj)
{
	objc_change(tree, obj, 0, 0, 0, 0, 0, tree[obj].ob_state | 1, 0);
}

void objc_uncheck(OBJECT* tree, int16 obj, bool draw = false)
{
	objc_change(tree, obj, 0, 0, 0, 0, 0, tree[obj].ob_state & ~1, 0);
}

void extract_filename_from_path(char* dst, const char* src, int16 len)
{
	dst[0] = 0;
	if (src)
	{
		uint16 srclen = strlen(src);
		if (srclen > 0)
		{
			char* p = &src[srclen-1];
			while(p != src)
			{
				if (*p == '/' || *p == '\\')
				{
					strncpy(dst, p+1, len);
					return;
				}
				p--;
			}
			strncpy(dst, p, len);
			return;
		}
	}
}

void remove_filename_from_path(char* src)
{
	int16 len = strlen(src) - 1;
	while (len >= 0)
	{
		if (src[len] == '/' || src[len] == '\\' || len == 0)
		{
			src[len] = 0;
			return;
		}
		len--;
	}
}

#define WIND_WIDGETS (NAME | CLOSER | MOVER)

class window_t
{
public:
	window_t() { handle = 0; form = NULL; x = 0; y = 0; w = 100; h = 100; isopen = false; }
	~window_t() { }

	void Create(OBJECT* obj, const char* name)
	{
		int16 deskx, desky, deskw, deskh;
		wind_get(0, WF_WORKXYWH, &deskx, &desky, &deskw, &deskh);
		handle = wind_create(WIND_WIDGETS, deskx, desky, deskw, deskh);
		SetForm(obj);
		SetName(name);
	}

	void Destroy()
	{
		if (handle)
		{
			Close();
			wind_delete(handle);
			handle = 0;
		}
	}

	void SetForm(OBJECT* obj)
	{
		bool isnew = (form == NULL) ? true : false;
		form = obj;
		int16 fx, fy, fw, fh;
		form_center(form, &fx, &fy, &fw, &fh);
		if (isnew) {
			x = fx; y = fy; w = fw; h = fh;
		} else {
			w = fw; h = fh;
		}
		form->ob_x = x;
		form->ob_y = y;
		if (isopen)
		{
			Close();
			Open();
		}
	}

	void SetName(const char* name)
	{
		uint32 nptr = (uint32) name;
		wind_set(handle, WF_NAME, (int16)(nptr>>16), (int16)(nptr&0xFFFF), 0, 0);
	}

	void Open()
	{
		if (!isopen)
		{
			int16 wx, wy, ww, wh;
			wind_calc(WC_BORDER, WIND_WIDGETS, x, y, w, h, &wx, &wy, &ww, &wh);
			if (wx < 0) wx = 0;
			if (wy < 0) wy = 0;
			wind_open(handle, wx, wy, ww, wh);
			isopen = true;
		}
	}

	void Close()
	{
		if (isopen)
		{
			wind_close(handle);
			isopen = false;
		}
	}


	int16 handle;
	OBJECT* form;
	int16 x,y,w,h;
	bool isopen;
};


window_t* get_window(int16 handle, window_t* windows[])
{
	for (uint16 i=0; windows[i] != NULL; i++)
		if (windows[i]->handle == handle)
			return windows[i];
	return NULL;
}


static void update_sound(bool nosound, int32 driver)
{
	if (!nosound)
	{
		objc_check(formMain, CTL_SOUND_ON);
		objc_uncheck(formMain, CTL_SOUND_OFF);
		menu_icheck(menu, MENU_SND_ENABLE, 1);
		for (uint16 i=MENU_SND_DRV_DUMMY; i<=MENU_SND_DRV_REPLAY16; i++)
		{
			menu_ienable(menu, i, 1);
			menu_icheck(menu, i, 0);
			if (!haveDMA)
				menu_ienable(menu, MENU_SND_DRV_DMA, 0);
			if (!haveYM)
				menu_ienable(menu, MENU_SND_DRV_YM, 0);
			menu_icheck(menu, driver + MENU_SND_DRV_DUMMY, 1);
		}
	}
	else
	{
		objc_uncheck(formMain, CTL_SOUND_ON);
		objc_check(formMain, CTL_SOUND_OFF);
		menu_icheck(menu, MENU_SND_ENABLE, 0);
		for (uint16 i=MENU_SND_DRV_DUMMY; i<=MENU_SND_DRV_REPLAY16; i++)
		{
			menu_ienable(menu, i, 0);
			menu_icheck(menu, i, 0);
		}
	}
}

static void update_video()
{
	int32 modeidx = 0;
	int32 modemax = MENU_GFX_MODE_CUR;
	int32 mode = PrefsFindInt32("video_mode");
	switch(mode)
	{
		case 1: modeidx = MENU_GFX_MODE_1B; break;
		case 4: modeidx = MENU_GFX_MODE_4B; break;
		case 8: modeidx = MENU_GFX_MODE_8B; break;
		case 16: modeidx = MENU_GFX_MODE_16B; break;
		default: modeidx = MENU_GFX_MODE_CUR; break;
	}
#if 0
	// GUI option is disabled until rez change is working correctly
	// in all combinations of TOS/AES/VDI
	ScreenDesc scr;
	if (QueryScreen(scr))
	{
		switch (scr.hw)
		{
			case HW_SHIFTER_ST:
			case HW_SHIFTER_STE:
				modemax = MENU_GFX_MODE_4B;
				break;
			case HW_SHIFTER_TT:
				modemax = MENU_GFX_MODE_8B;
				break;
			case HW_VIDEL:
				//modemax = MENU_GFX_MODE_16B;
				break;
		}
	}
#endif
	for (uint16 i = MENU_GFX_MODE_CUR; i<=modemax; i++)
	{
		menu_ienable(menu, i, 1);
		menu_icheck(menu, i, 0);
	}
	for (uint16 i = (modemax + 1); i <= MENU_GFX_MODE_16B; i++)
	{
		menu_ienable(menu, i, 0);
		menu_icheck(menu, i, 0);
	}
	if (modeidx > modemax)
		modeidx = MENU_GFX_MODE_CUR;

	menu_icheck(menu, modeidx, 1);

	bool emu = PrefsFindBool("video_emu");
	bool mmu = PrefsFindBool("video_mmu");
	bool cmp = PrefsFindBool("video_cmp");
	menu_icheck(menu, MENU_GFX_EMUDISABLE, emu ? 0 : 1);
	menu_icheck(menu, MENU_GFX_MMUACCEL, (/*emu &&*/ mmu) ? 1 : 0);
	menu_icheck(menu, MENU_GFX_CMPACCEL, (/*emu &&*/ cmp) ? 1 : 0);
	menu_ienable(menu, MENU_GFX_MMUACCEL, emu ? 1 : 0);
	menu_ienable(menu, MENU_GFX_CMPACCEL, emu ? 1 : 0);
}

static const char* update_logmode(int16 mode)
{
	struct modeinf {int16 idx; const char* str; };

	const modeinf modes[] = {
		{MENU_DBG_LOG_OFF, "off"},
		{MENU_DBG_LOG_FILE, "file"},
		{MENU_DBG_LOG_SERIAL, "serial"},
		{MENU_DBG_LOG_SCREEN, "screen"},
	};
	const uint16 count = sizeof(modes) / sizeof(modes[0]);

	if (mode == -1) {
		const char* curmodestr = PrefsFindString("logging");
		if (curmodestr) {
			for (uint16 i=0; i<count && mode == -1; i++) {
				if (strcmp(curmodestr, modes[i].str) == 0) {
					mode = i;
				}
			}
		}
	}
	if (mode == -1)
		mode = 0;

	PrefsReplaceString("logging", modes[mode].str);

	for (uint16 i=0; i<count; i++) {
		menu_icheck(menu, modes[i].idx, 0);
	}

	menu_icheck(menu, modes[mode].idx, 1);
	return modes[mode].str;
}

/*
 *  Show preferences editor
 *  Returns true when user clicked on "Start", false otherwise
 */

bool PrefsEditor(void)
{
	log("Entering Prefs editor\n");
	rsrc_gaddr(R_TREE, FORM_MAIN, &formMain);
	if (formMain == NULL)
		return false;

	menu = RscMenuPtr;
	menu_bar(menu, 1);

	const uint32 tempdatasize = 16 * 1024;
	char* tempdata = Malloc(tempdatasize);
	if (tempdata == 0)
		return false;
	memset(tempdata, 0, tempdatasize);
	char* fselpath = tempdata;
	char* fselname = fselpath + 256;
	char* cpuString = fselname + 256;
	char* verString = cpuString + 8;
	char* romString = verString + 32;
	char* aboutString = romString + 32;

	sprintf(cpuString, "680%1d0", CPUType);
	formMain[CTL_CPU].ob_spec.tedinfo->te_ptext = cpuString;
	formMain[CTL_CPU].ob_spec.tedinfo->te_txtlen = strlen(cpuString);

	sprintf(verString, " %.32s", __DATE__);
	formMain[CTL_VERSION].ob_spec.tedinfo->te_ptext = verString;
	formMain[CTL_VERSION].ob_spec.tedinfo->te_txtlen = strlen(verString);

	if (PrefsFindString("rom", 0))
		strncpy(fselpath, PrefsFindString("rom", 0), 256);
	extract_filename_from_path(romString, fselpath, 13);
	formMain[CTL_ROM].ob_spec.tedinfo->te_ptext = romString;
	formMain[CTL_ROM].ob_spec.tedinfo->te_txtlen = strlen(romString);

 	int32 v_ram = PrefsFindInt32("ram");
	if (HostFPUType == 0)
	{
		objc_change(formMain, CTL_FPU_ON, 0, 0, 0, 0, 0, 0x8, 0);
		objc_change(formMain, CTL_FPU_OFF, 0, 0, 0, 0, 0, 0x9, 0);
	}
	else
	{
		if (PrefsFindBool("fpu"))
			objc_check(formMain, CTL_FPU_ON);
		else
			objc_check(formMain, CTL_FPU_OFF);
	}

	if (PrefsFindInt32("modelid") == 14)
		objc_check(formMain, CTL_MODEL_14);
	else
		objc_check(formMain, CTL_MODEL_5);

	menu_icheck(menu, MENU_DBG_SAFEIRQ, PrefsFindBool("irqsafe") ? 1 : 0);

	update_video();
	update_logmode(-1);

	uint32 cookie = 0;
	Getcookie('_SND', &cookie);
	haveYM = cookie & 1;
	haveDMA = cookie & 2;
	int32 snddriver = PrefsFindInt32("sound_driver");
	if ((snddriver == 1) && !haveDMA)
		snddriver = 2;
	if ((snddriver == 2) && !haveYM)
		snddriver = 0;

	bool nosound = PrefsFindBool("nosound");
	update_sound(nosound, snddriver);

	if (v_ram <= 0)
	{
		objc_check(formMain, CTL_RAM_MAX);
		objc_change(formMain, CTL_RAM, 0, 0, 0, 0, 0, 0x8, 0);
	}

	window_t wndMain;
	wndMain.Create(formMain, "BasiliskII");

	window_t* topwindow = &wndMain;
	window_t* windows[2];
	windows[0] = &wndMain;
	windows[1] = NULL;
	wndMain.Open();

	graf_mouse(ARROW, 0);

	int16 done = 0;
	while (done == 0)
	{
		int16 emx, emy, emb;	// event mouse
		int16 eks, ekr, ebr;	// event keyb
		int16 emsg[8];			// msg buffer

		int16 event = evnt_multi(
			MU_KEYBD | MU_BUTTON | MU_MESAG /*| MU_TIMER*/,
			0x02, 0x01, 0x01, 					// mouse clicks, mask, state
			0, 0, 0, 0, 0,						// mouse enter / leave rectangle 1
 			0, 0, 0, 0, 0,						// mouse enter / leave rectangle 2
			emsg,								// msgbuffer
			10,									// timer
			&emx, &emy, &emb,					// mouse state
			&eks, &ekr, &ebr);					// keyb state

		switch (event)
		{
			case MU_TIMER:
				Syield();
				break;

			case MU_MESAG:
			{
				switch(emsg[0])
				{
					// window
					case WM_CLOSED:
					{
						window_t* w = get_window(emsg[3], windows);
						if (w)
						{
							if (w->form == formMain)
								done = -1;
						}
					}
					break;

					case WM_TOPPED:
					{
						wind_set(emsg[3], WF_TOP, emsg[4], emsg[5], emsg[6], emsg[7]);
						window_t* w = get_window(emsg[3], windows);
						if (w) {
							topwindow = w;
						}
					}
					break;

					case WM_BOTTOMED:
					{
						wind_set(emsg[3], WF_BOTTOM, emsg[4], emsg[5], emsg[6], emsg[7]);
					}
					break;

					case WM_SIZED:
					case WM_MOVED:
					{
						wind_set(emsg[3], WF_CURRXYWH, emsg[4], emsg[5], emsg[6], emsg[7]);
						window_t* w = get_window(emsg[3], windows);
						if (w)
						{
							wind_calc(WC_WORK, NAME | CLOSER | MOVER, emsg[4], emsg[5], emsg[6], emsg[7], &w->x, &w->y, &w->w, &w->h);
							w->form->ob_x = w->x;
							w->form->ob_y = w->y;
						}
					}
					break;

					case WM_REDRAW:
					{
						window_t* w = get_window(emsg[3], windows);
						if (w && w->form)
						{
							int16 wx, wy, ww, wh;
							wind_get(emsg[3], WF_CURRXYWH, &wx, &wy, &ww, &wh);
							wind_calc(WC_WORK, NAME | CLOSER | MOVER, wx, wy, ww, wh, &w->x, &w->y, &w->w, &w->h);

							GRECT r1;
							r1.g_x = emsg[4];
							r1.g_y = emsg[5];
							r1.g_w = emsg[6];
							r1.g_h = emsg[7];

							GRECT r2;
							wind_update(BEG_UPDATE);
							wind_get(w->handle, WF_FIRSTXYWH, &r2.g_x, &r2.g_y, &r2.g_w, &r2.g_h);
							while (r2.g_w && r2.g_h)
							{
								if (rc_intersect(&r1, &r2))
									objc_draw(w->form, 0, MAX_DEPTH, r2.g_x, r2.g_y, r2.g_w, r2.g_h);
								wind_get(w->handle, WF_NEXTXYWH, &r2.g_x, &r2.g_y, &r2.g_w, &r2.g_h);
							}
							wind_update(END_UPDATE);
						}
					}
					break;

					// menu
					case MN_SELECTED:
					{
						int16 title = emsg[3];
						int16 item = emsg[4];
						menu_tnormal(menu, title, 1);
						switch(item)
						{
							case MENU_ABOUT:
							{
								char strver[64];
								sprintf(strver, GetString(STR_ABOUT_TEXT1), VERSION_MAJOR, VERSION_MINOR);
								sprintf(aboutString, "[0][%s|%s  | ][Ok]", strver, GetString(STR_ABOUT_TEXT2));
								form_alert(1, aboutString);
							}
							break;

							case MENU_START:
								done = 1;
								break;

							case MENU_QUIT:
								done = -1;
								break;

							case MENU_SND_ENABLE:
							{
								bool en = (formMain[CTL_SOUND_ON].ob_state & 1) == 0;
								objc_change(formMain, CTL_SOUND_ON, 0, wndMain.x, wndMain.y, wndMain.w, wndMain.h,
									(formMain[CTL_SOUND_ON].ob_state & ~1) | (en ? 1 : 0), 1);
								objc_change(formMain, CTL_SOUND_OFF, 0, wndMain.x, wndMain.y, wndMain.w, wndMain.h,
									(formMain[CTL_SOUND_OFF].ob_state & ~1) | (en ? 0 : 1), 1);
								update_sound(!en, snddriver);
							}
							break;

							case MENU_SND_DRV_DUMMY:
							case MENU_SND_DRV_DMA:
							case MENU_SND_DRV_YM:
							case MENU_SND_DRV_COVOX:
							case MENU_SND_DRV_MV16:
							case MENU_SND_DRV_REPLAY8:
							case MENU_SND_DRV_REPLAY8S:
							case MENU_SND_DRV_REPLAY16:
							{
								for (uint16 i=MENU_SND_DRV_DUMMY; i<MENU_SND_DRV_REPLAY16; i++)
									menu_icheck(menu, i, 0);
								menu_icheck(menu, item, 1);
								snddriver = item - MENU_SND_DRV_DUMMY;
							}
							break;

							case MENU_DBG_SAFEIRQ:
							{
								bool en = !PrefsFindBool("irqsafe");
								PrefsReplaceBool("irqsafe", en);
								menu_icheck(menu, item, en ? 1 : 0);
							}
							break;

							case MENU_DBG_LOG_OFF:
							case MENU_DBG_LOG_FILE:
							case MENU_DBG_LOG_SERIAL:
							case MENU_DBG_LOG_SCREEN:
							{
								update_logmode(item - MENU_DBG_LOG_OFF);
							}
							break;

							case MENU_GFX_EMUDISABLE:
							{
								bool en = !PrefsFindBool("video_emu");
								PrefsReplaceBool("video_emu", en);
								update_video();
							}
							break;
							case MENU_GFX_MMUACCEL:
							{
								bool en = !PrefsFindBool("video_mmu");
								PrefsReplaceBool("video_mmu", en);
								update_video();
							}
							break;
							case MENU_GFX_CMPACCEL:
							{
								bool en = !PrefsFindBool("video_cmp");
								PrefsReplaceBool("video_cmp", en);
								update_video();
							}
							break;
							case MENU_GFX_MODE_CUR:
							{
								PrefsReplaceInt32("video_mode", 0);
								update_video();
							}
							break;
							case MENU_GFX_MODE_1B:
							{
								PrefsReplaceInt32("video_mode", 1);
								update_video();
							}
							break;
							case MENU_GFX_MODE_4B:
							{
								PrefsReplaceInt32("video_mode", 4);
								update_video();
							}
							break;
							case MENU_GFX_MODE_8B:
							{
								PrefsReplaceInt32("video_mode", 8);
								update_video();
							}
							break;
							case MENU_GFX_MODE_16B:
							{
								PrefsReplaceInt32("video_mode", 16);
								update_video();
							}
							break;
						}
					}
					break;
				}
			}
			break;

			// keyboard
			case MU_KEYBD:
			{
				window_t* w = topwindow;
				if (w)
				{
					int16 next_obj = 0;
					int16 ret = form_keybd(w->form, 0, 0, ekr, &next_obj, &ekr);
					//D(bug("form_keyb: ret=%d, kr=%d, no=%d\n", ret, ekr, next_obj));
					if (ret == 0)
					{
						// exit object selected
						if ((ekr == 0) && (next_obj == CTL_START))
							done = 1;
					}
					else
					{
						// close window on escape key
						if (((ekr >> 8) == 1) && (next_obj == 0))
						{
							if (w->form == formMain)
								done = -1;
							else
								w->Close();
						}
					}
				}				
			}
			break;

			// form
			case MU_BUTTON:
			{
				window_t* w = topwindow;
				if (w)
				{
					int16 obj = objc_find(w->form, ROOT, MAX_DEPTH, emx, emy);
					if (obj > 0)
					{
						OBJECT& o = w->form[obj];
						D(bug("obj = %d, flags = %x, state = %x\n", obj, o.ob_flags, o.ob_state));
						if (!(o.ob_state & OS_DISABLED))
						{
							if (o.ob_flags & OF_SELECTABLE)
							{
								if (o.ob_flags & OF_RBUTTON)
								{
									if ((o.ob_state & 1) == 0)
									{
										int16 pobj = objc_get_parent(w->form, obj);
										for (int16 sobj = w->form[pobj].ob_head; sobj != pobj; sobj = w->form[sobj].ob_next)
											objc_change(w->form, sobj, 0, w->x, w->y, w->w, w->h, 0, 1);
										objc_change(w->form, obj, 0, w->x, w->y, w->w, w->h, 1, 1);

										switch (obj)
										{
											case CTL_SOUND_ON:
												update_sound(false, snddriver);
												break;

											case CTL_SOUND_OFF:
												update_sound(true, snddriver);
												break;
										}
									}
								}
								else if (o.ob_flags & OF_EXIT)
								{
									objc_change(w->form, obj, 0, w->x, w->y, w->w, w->h, 1, 1);
									objc_change(w->form, obj, 0, w->x, w->y, w->w, w->h, 0, 0);
									if (w->form == formMain)
										done = 1;
									else
										w->Close();
								}
								else if (o.ob_flags & OF_TOUCHEXIT)
								{
									if (obj == CTL_ROM)
									{
										int16 result = 0;
										sprintf(fselpath, "*.ROM");
										fselname[0] = 0;
										wind_update(BEG_MCTRL);
										if (fsel_exinput(fselpath, fselname, &result, "Select Macintosh ROM") == 0)
											result = 0;
										wind_update(END_MCTRL);
										if (result == 1)
										{
											strncpy(romString, fselname, 13);
											formMain[CTL_ROM].ob_spec.tedinfo->te_ptext = romString;
											formMain[CTL_ROM].ob_spec.tedinfo->te_txtlen = strlen(romString);
											remove_filename_from_path(fselpath);
											if (strlen(fselpath) > 0)
												strcat(fselpath, "\\");
											strcat(fselpath, fselname);
											if (strlen(fselpath) > 0)
												PrefsReplaceString("rom", fselpath);
											else
												PrefsReplaceBool("rom", 0);
										}
									}
								}
								else
								{
									objc_change(w->form, obj, 0, w->x, w->y, w->w, w->h, (~o.ob_state) & 1, 1);
								}
							}
						}
					}
				}
			}
			break;
		}
	}

	D(bug("going to exit now..\n"));

	if (formMain[CTL_FPU_ON].ob_state != 0x8)
		PrefsReplaceBool("fpu", (formMain[CTL_FPU_ON].ob_state & 1) ? true : false);
	PrefsReplaceBool("nosound", (formMain[CTL_SOUND_OFF].ob_state & 1) ? true : false);
	PrefsReplaceInt32("modelid", (formMain[CTL_MODEL_14].ob_state & 1) ? 14 : 5);
	PrefsReplaceInt32("sound_driver", snddriver);

	// todo: custom save to avoid all the non-atari stuff
	D(bug("Saving prefs\n"));
	SavePrefs();
	D(bug("Cleanup\n"));
	for (uint16 i=0; windows[i] != NULL; i++) {
		D(bug(" Destroy window %i [%08x : %08x]\n", i, windows[i], windows[i]->handle));
		windows[i]->Destroy();
	}
	log("Exiting Prefs editor\n");
	Mfree(tempdata);
	return (done > 0);
}
