#include <QObject>
#include <QScreen>
#include <QWindow>
#include <QX11Info>
#include <QList>
#include <QApplication>
#include <QWidget>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "platformhelper_x11.h"

typedef QMap<QString, unsigned long> CAtomHash;

static CAtomHash  known_atoms;
static CAtomHash  wm_supports;
static Display   *display;
static Window     root;

static inline Atom internalGetAtom(const QString &name, bool only_if_exists = false)
{
	CAtomHash::iterator it     = known_atoms.find(name);
	Atom                result = None;

	if (it == known_atoms.end()) {
		result = XInternAtom(display, name.toUtf8().data(), (only_if_exists ? True : False));
		known_atoms[name] = result;
	} else {
		result = known_atoms[name];
	}

	return result;
}

static inline void internalAddAtom(Atom atom, bool supported = false)
{
	char *name = XGetAtomName(display, atom);

	if (name) {
		known_atoms[QString(name)] = atom;
		if (supported) {
			wm_supports[QString(name)] = atom;
		}
		XFree(name);
	}
}

static inline void internalDetectWMSupportedAtoms()
{
	Atom           a1, a2;
	unsigned long  supp_count = 0;
	unsigned long  items_return = 0;
	unsigned long  bytes_after_return = 1;
	int            actual_format = 32;
	unsigned char *data = NULL;

	known_atoms.clear();
	wm_supports.clear();

	display = QX11Info::display();
	if (!display) {
		qDebug("Failed to connect to the X server");
		exit( 2 );
	}
	root = DefaultRootWindow(display);

	a1 = internalGetAtom("_NET_SUPPORTED");
	while (bytes_after_return) {
		XGetWindowProperty(display, root, a1, supp_count, 1, False,XA_ATOM, &a2,
						   &actual_format, &items_return, &bytes_after_return, &data);
		if (items_return) {
			a2 = *((Atom *)data);
			internalAddAtom(a2, true);
			supp_count += 1;
			XFree(data);
		} else {
			break;
		}
	}
	if ( !wm_supports.size() ) {
		qDebug("FAILED to detect what your WM supports");
	}
}

static inline bool WMSupports(const QString &name)
{
	if (root != DefaultRootWindow(display))
		internalDetectWMSupportedAtoms();

	CAtomHash::iterator it = wm_supports.find(name);

	if (it != wm_supports.end()) return true;
	return false;
}

static inline Atom atom(const QString &name, bool only_if_exists = false)
{
	return internalGetAtom(name, only_if_exists);
}

static inline Window getXWID(QWidget *w)
{
	if (w->isWindow()) {
		return Window(w->winId());
	} else {
		return None;
	}
}

PlatformHelperImpl::PlatformHelperImpl(QObject *parent) : QObject(parent)
{
	internalDetectWMSupportedAtoms();
}

PlatformHelperImpl::~PlatformHelperImpl()
{
}

bool PlatformHelperImpl::isShownOnAllDesktops(QWidget *w)
{
	XID            wid                = getXWID(w);
	Atom           actual_type        = None;
	int            actual_format      = 0;
	unsigned long  items_return       = 0;
	unsigned long  bytes_after_return = 0;
	unsigned char *data               = NULL;
	unsigned long  i                  = 0;

	if (wid == None) return false;

	if (WMSupports("_NET_WM_DESKTOP")) {
		XGetWindowProperty(display, wid, atom( "_NET_WM_DESKTOP" ), 0, 1, False, XA_CARDINAL,
						   &actual_type, &actual_format, &items_return, &bytes_after_return, &data);
		if (items_return) {
			if ((*((unsigned * )data)) == 0xffffffff) {
				XFree(data);
				return true;
			}
			XFree(data);
		}
		return false;
	} else {
		if (WMSupports("_NET_WM_STATE")) {
			XGetWindowProperty(display, wid, atom( "_NET_WM_STATE" ), 0, 100, False, XA_ATOM,
							   &actual_type, &actual_format, &items_return, &bytes_after_return, &data );
			if (items_return) {
				Atom *ar = (Atom *)data;
				for (i = 0; i < items_return; i++) {
					if (ar[i] == atom("_NET_WM_STATE_STICKY")) {
						XFree(data);
						return true;
					}
				}
			}
			XFree(data);
		}
	}

	return false;
}

void PlatformHelperImpl::setShownOnAllDesktops(QWidget *w)
{
	XID    wid = getXWID(w);
	XEvent evt;

	if (wid == None) return;
	if (isShownOnAllDesktops(w)) return;

	if (WMSupports("_NET_WM_DESKTOP")) {
		/* use the standard way */
		memset(&evt, 0, sizeof(evt));
		evt.xclient.type         = ClientMessage;
		evt.xclient.window       = wid;
		evt.xclient.message_type = atom("_NET_WM_DESKTOP");
		evt.xclient.format       = 32;
		evt.xclient.data.l[0]    = 0xffffffff;
		evt.xclient.data.l[1]    = 1;

		XSendEvent(display, root, False, SubstructureRedirectMask, &evt);
		XSync(display, False);
	} else {
		if (WMSupports("_NET_WM_STATE")) {
			/* hack 1: set the window to 'sticky' */
			memset(&evt, 0, sizeof(evt));
			evt.xclient.type         = ClientMessage;
			evt.xclient.window       = wid;
			evt.xclient.message_type = atom("_NET_WM_STATE");
			evt.xclient.format       = 32;
			evt.xclient.data.l[0]    = 1;  /* _NET_WM_STATE_ADD */
			evt.xclient.data.l[1]    = atom("_NET_WM_STATE_STICKY");
			evt.xclient.data.l[2]    = 0;
			evt.xclient.data.l[3]    = 1;

			XSendEvent(display, root, False, SubstructureRedirectMask, &evt);
			XSync(display, False);
		}
	}
}

bool PlatformHelperImpl::isShownOnSingleDesktop(QWidget *w)
{
	XID            wid                = getXWID(w);
	Atom           actual_type        = None;
	int            actual_format      = 0;
	unsigned long  items_return       = 0;
	unsigned long  bytes_after_return = 0;
	unsigned char *data               = NULL;
	unsigned long  i                  = 0;

	if (wid == None) return true;

	if (WMSupports("_NET_WM_DESKTOP")) {
		XGetWindowProperty(display, wid, atom( "_NET_WM_DESKTOP" ), 0, 1, False, XA_CARDINAL,
						   &actual_type, &actual_format, &items_return, &bytes_after_return, &data);
		if (items_return) {
			if ((*((unsigned *)data)) == 0xffffffff) {
				XFree(data);
				return false;
			}
			XFree(data);
		}
		return true;
	} else {
		if (WMSupports("_NET_WM_STATE")) {
			XGetWindowProperty(display, wid, atom( "_NET_WM_STATE" ), 0, 100, False, XA_ATOM,
							   &actual_type, &actual_format, &items_return, &bytes_after_return, &data);
			if (items_return) {
				Atom *ar = (Atom *)data;
				for (i = 0; i < items_return; i++) {
					if (ar[i] == atom("_NET_WM_STATE_STICKY")) {
						XFree( data );
						return false;
					}
				}
			}
			XFree(data);
		}
	}

	return true;
}

void PlatformHelperImpl::setShownOnSingleDesktop(QWidget *w)
{
	XID    wid = getXWID(w);
	XEvent evt;

	if (wid == None) return;
	if (isShownOnSingleDesktop(w)) return;

	if (WMSupports("_NET_WM_DESKTOP")) {
		/* use the standard way */
		memset(&evt, 0, sizeof(evt));
		evt.xclient.type         = ClientMessage;
		evt.xclient.window       = wid;
		evt.xclient.message_type = atom("_NET_WM_DESKTOP");
		evt.xclient.format       = 32;
		evt.xclient.data.l[0]    = 0;
		evt.xclient.data.l[1]    = 1;

		XSendEvent(display, root, False, SubstructureRedirectMask, &evt);
		XSync(display, False);
	} else {
		if (WMSupports("_NET_WM_STATE")) {
			/* hack 1: set the window to 'sticky' */
			memset( &evt, 0, sizeof( evt ) );
			evt.xclient.type         = ClientMessage;
			evt.xclient.window       = wid;
			evt.xclient.message_type = atom( "_NET_WM_STATE" );
			evt.xclient.format       = 32;
			evt.xclient.data.l[0]    = 0;   /* _NET_WM_STATE_REMOVE */
			evt.xclient.data.l[1]    = atom( "_NET_WM_STATE_STICKY" );
			evt.xclient.data.l[2]    = 0;
			evt.xclient.data.l[3]    = 1;

			XSendEvent(display, root, False, SubstructureRedirectMask, &evt);
			XSync(display, False);
		}
	}
}

void PlatformHelperImpl::removeTaskbarIcon(QWidget *w)
{
	XID    wid = getXWID(w);
	XEvent evt;

	if (wid == None) return;

	if (WMSupports("_NET_WM_STATE")) {
		memset(&evt, 0, sizeof(evt));
		evt.xclient.type         = ClientMessage;
		evt.xclient.window       = wid;
		evt.xclient.message_type = atom("_NET_WM_STATE");
		evt.xclient.format       = 32;
		evt.xclient.data.l[0]    = 1;
		evt.xclient.data.l[1]    = atom("_NET_WM_STATE_SKIP_TASKBAR");
		evt.xclient.data.l[2]    = 0;
		evt.xclient.data.l[3]    = 1;

		XSendEvent(display, root, False, SubstructureRedirectMask, &evt);
		XSync(display, False);
	}
}

bool PlatformHelperImpl::beginNativeDrag(QWidget *w, const QPoint &dragPoint)
{
	XID    wid = getXWID(w);
	XEvent evt;

	if (wid == None) return false;

	if (WMSupports("_NET_WM_MOVERESIZE")) {
		QPoint gdp = dragPoint;

		XUngrabPointer(display, 0L);
		XFlush( display );

		evt.xclient.type         = ClientMessage;
		evt.xclient.window       = wid;
		evt.xclient.message_type = atom("_NET_WM_MOVERESIZE");
		evt.xclient.format       = 32;
		evt.xclient.data.l[0]    = gdp.x();
		evt.xclient.data.l[1]    = gdp.y();
		evt.xclient.data.l[2]    = 8;    /* _NET_WM_MOVERESIZE_MOVE */
		evt.xclient.data.l[3]    = 0;
		evt.xclient.data.l[4]    = 0;

		XSendEvent(display, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &evt);
		XSync(display, False);

		return true;
	} else {
		return false;
	}
}

bool PlatformHelperImpl::popUp(QWidget *w)
{
	XID    wid = getXWID(w);
	XEvent evt;

	if (wid != None && WMSupports("_NET_ACTIVE_WINDOW")) {
		memset(&evt, 0, sizeof(evt));
		evt.xclient.type         = ClientMessage;
		evt.xclient.window       = wid;
		evt.xclient.message_type = atom("_NET_ACTIVE_WINDOW");
		evt.xclient.format       = 32;
		evt.xclient.data.l[0]    = 2;
		evt.xclient.data.l[1]    = CurrentTime;
		evt.xclient.data.l[2]    = wid;
		evt.xclient.data.l[3]    = 0;
		evt.xclient.data.l[4]    = 0;

		XSendEvent(display, root, False, SubstructureRedirectMask, &evt);
		XSync(display, False);
		return true;
	}

	return false;
}

QRect PlatformHelperImpl::getActiveWindowGeometry()
{
	QRect ret;
	Atom actual_type_return;
	int actual_format_return;
	unsigned long nitems_return = 0;
	unsigned long bytes_after_return;
	unsigned char *prop_return = nullptr;

	if(XGetWindowProperty(display, root, atom("_NET_ACTIVE_WINDOW"), 0l, 1l, False, AnyPropertyType,
	                      &actual_type_return, &actual_format_return, &nitems_return, &bytes_after_return,
	                      &prop_return) == Success && nitems_return > 0) {
		Window wid = *((Window *)prop_return);
		int x, y, tx, ty;
		unsigned w, h, b, d;
		Window r;

		XGetGeometry(display, wid, &r, &x, &y, &w, &h, &b, &d);
		XTranslateCoordinates(display, wid, r, 0, 0, &tx, &ty, &wid);
		ret = QRect(tx, ty, w, h);

		XFree(prop_return);
	}

	return ret;
}
