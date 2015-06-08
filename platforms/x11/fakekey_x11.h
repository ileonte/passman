#ifndef _HAVE_LIBFAKEKEY_H
#define _HAVE_LIBFAKEKEY_H

/*
 * This implementation has been 'borrowed' and adapted from:
 *
 *    http://git.yoctoproject.org/cgit/cgit.cgi/libfakekey/
 *
 */

#include <QString>
#include <QChar>
#include <QVector>

typedef struct FakeKey FakeKey;

enum FakeKeyModifier {
	FAKEKEYMOD_SHIFT   = (1<<1),
	FAKEKEYMOD_CONTROL = (1<<2),
	FAKEKEYMOD_ALT     = (1<<3),
	FAKEKEYMOD_META    = (1<<4)
};

FakeKey *fakekey_init(void *xdpy_);
int      fakekey_press(FakeKey *fk, quint32 ucs4, int modifiers);
void     fakekey_repeat(FakeKey *fk);
void     fakekey_release(FakeKey *fk);
int      fakekey_reload_keysyms(FakeKey *fk);
int      fakekey_press_keysym(FakeKey *fk, unsigned int keysym, int flags);
int      fakekey_send_keyevent(FakeKey *fk, unsigned int keycode, int is_press, int modifiers);

#endif /* _HAVE_LIBFAKEKEY_H */
