#include "fakekey_x11.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>

#define N_MODIFIER_INDEXES (Mod5MapIndex + 1)

typedef unsigned int FkChar32;

struct FakeKey
{
	Display *xdpy;
	int      min_keycode, max_keycode;
	int      n_keysyms_per_keycode;
	KeySym  *keysyms;
	int      held_keycode;
	int      held_state_flags;
	KeyCode  modifier_table[N_MODIFIER_INDEXES];
	int      shift_mod_index, alt_mod_index, meta_mod_index;
};

FakeKey *fakekey_init(void *xdpy_)
{
	Display         *xdpy = (Display *)xdpy_;
	FakeKey         *fk = NULL;
	int              event, error, major, minor;
	XModifierKeymap *modifiers;
	int              mod_index;
	int              mod_key;
	KeyCode         *kp;

	if (xdpy == NULL) return NULL;
  
	if (!XTestQueryExtension(xdpy, &event, &error, &major, &minor))
		return NULL;

	fk = (FakeKey *)malloc(sizeof(FakeKey));
	memset(fk,0,sizeof(FakeKey));

	fk->xdpy = xdpy;

	XDisplayKeycodes(fk->xdpy, &fk->min_keycode, &fk->max_keycode);
	fk->keysyms = XGetKeyboardMapping(fk->xdpy,
						    fk->min_keycode,
						    fk->max_keycode - fk->min_keycode + 1,
						    &fk->n_keysyms_per_keycode);

	modifiers = XGetModifierMapping(fk->xdpy);

	kp = modifiers->modifiermap;

	for (mod_index = 0; mod_index < 8; mod_index++) {
		fk->modifier_table[mod_index] = 0;
		for (mod_key = 0; mod_key < modifiers->max_keypermod; mod_key++) {
			int keycode = kp[mod_index * modifiers->max_keypermod + mod_key];
			if (keycode != 0) {
				fk->modifier_table[mod_index] = keycode;
				break;
			}
		}
	}
  
	for (mod_index = Mod1MapIndex; mod_index <= Mod5MapIndex; mod_index++) {
		if (fk->modifier_table[mod_index]) {
			KeySym ks = XKeycodeToKeysym(fk->xdpy, fk->modifier_table[mod_index], 0);
	  
			switch (ks) {
				case XK_Meta_R:
				case XK_Meta_L: {
					fk->meta_mod_index = mod_index;
					break;
				}

				case XK_Alt_R:
				case XK_Alt_L: {
					fk->alt_mod_index = mod_index;
					break;
				}

				case XK_Shift_R:
				case XK_Shift_L: {
					fk->shift_mod_index = mod_index;
					break;
				}
			}
		}
	}
  
	if (modifiers) XFreeModifiermap(modifiers);

	return fk;
}

int fakekey_reload_keysyms(FakeKey *fk)
{
	if (fk->keysyms) XFree(fk->keysyms);

	fk->keysyms = XGetKeyboardMapping(fk->xdpy,
						    fk->min_keycode,
						    fk->max_keycode - fk->min_keycode + 1,
						    &fk->n_keysyms_per_keycode);
	return 1;
}

int fakekey_send_keyevent(FakeKey *fk, unsigned int keycode, int is_press, int flags)
{
	if (flags) {
		if (flags & FAKEKEYMOD_SHIFT)
			XTestFakeKeyEvent(fk->xdpy,
						fk->modifier_table[ShiftMapIndex],
						is_press,
						CurrentTime);

		if (flags & FAKEKEYMOD_CONTROL)
			XTestFakeKeyEvent(fk->xdpy,
						fk->modifier_table[ControlMapIndex],
						is_press,
						CurrentTime);

		if (flags & FAKEKEYMOD_ALT)
			XTestFakeKeyEvent(fk->xdpy,
						fk->modifier_table[fk->alt_mod_index],
						is_press,
						CurrentTime);

		XSync(fk->xdpy, False);
	}

	XTestFakeKeyEvent(fk->xdpy, keycode, is_press, CurrentTime);

	XSync(fk->xdpy, False);

	return 1;
}

int fakekey_press_keysym(FakeKey *fk, unsigned int keysym, int flags)
{
	static int modifiedkey;
	KeyCode    code = 0;

	if ((code = XKeysymToKeycode(fk->xdpy, keysym)) != 0) {
		if (XKeycodeToKeysym(fk->xdpy, code, 0) != keysym) {
			if (XKeycodeToKeysym(fk->xdpy, code, 1) == keysym) {
				flags |= FAKEKEYMOD_SHIFT;
			} else {
				code = 0;
			}
		} else {
			flags &= ~FAKEKEYMOD_SHIFT;
		}
	}

	if (!code) {
		int index;

		modifiedkey = (modifiedkey+1) % 10;
		index = (fk->max_keycode - fk->min_keycode - modifiedkey - 1) * fk->n_keysyms_per_keycode;
		fk->keysyms[index] = keysym;
		XChangeKeyboardMapping(fk->xdpy, fk->min_keycode, fk->n_keysyms_per_keycode, fk->keysyms, (fk->max_keycode-fk->min_keycode));
		XSync(fk->xdpy, False);

		code = fk->max_keycode - modifiedkey - 1;
	}

	if (code != 0)  {
		fakekey_send_keyevent(fk, code, True, flags);

		fk->held_state_flags = flags;
		fk->held_keycode     = code;

		return 1;
	}

	fk->held_state_flags = 0;
	fk->held_keycode     = 0;

	return 0;
}

int fakekey_press(FakeKey *fk, quint32 ucs4, int flags)
{
	if (fk->held_keycode)
		return 0;

	if (ucs4 > 0x00ff) ucs4 = ucs4 | 0x01000000;

	return fakekey_press_keysym(fk, (KeySym)ucs4, flags);
}

void fakekey_repeat(FakeKey *fk)
{
	if (!fk->held_keycode) return;
	fakekey_send_keyevent(fk, fk->held_keycode, True, fk->held_state_flags);
}

void fakekey_release(FakeKey *fk)
{
	if (!fk->held_keycode) return;

	fakekey_send_keyevent(fk, fk->held_keycode, False, fk->held_state_flags);

	fk->held_state_flags = 0;
	fk->held_keycode     = 0;
}
