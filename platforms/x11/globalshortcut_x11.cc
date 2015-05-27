#include "globalshortcut_x11.h"

#include <QKeyEvent>
#include <QApplication>
#include <QX11Info>
#include <QVector>
#include <QDebug>

#include <xcb/xcb.h>
#include <X11/Xlib.h>

const QVector<quint32> maskModifiers({0, Mod2Mask, LockMask, Mod2Mask|LockMask});

/*
 * X11 error handler
 */
class CX11ErrorHandler;
static CX11ErrorHandler *g_x11ErrorHandler;

class CX11ErrorHandler {
public:
	bool error;
	QString errorString;

	static int cX11ErrorHandler(Display *display, XErrorEvent *event)
	{
		if (g_x11ErrorHandler) {
			char ebuff[512] = {};
			XGetErrorText(display, event->error_code, ebuff, sizeof(ebuff));

			g_x11ErrorHandler->error = true;
			g_x11ErrorHandler->errorString = ebuff;
		}

		return 0;
	}

	CX11ErrorHandler() : error(false), errorString("")
	{
		g_x11ErrorHandler = this;
		previousErrorHandler = XSetErrorHandler(cX11ErrorHandler);
	}

	~CX11ErrorHandler()
	{
		XSetErrorHandler(previousErrorHandler);
		g_x11ErrorHandler = nullptr;
	}

private:
	XErrorHandler previousErrorHandler;
};

/*
 * Utility functions
 */
static inline Display *dpy()
{
	Display *r = QX11Info::display();
	return r ? r : XOpenDisplay(0);
}

static inline quint32 nativeModifiers(Qt::KeyboardModifiers modifiers)
{
	quint32 native = 0;
      if (modifiers & Qt::ShiftModifier)
          native |= ShiftMask;
      if (modifiers & Qt::ControlModifier)
          native |= ControlMask;
      if (modifiers & Qt::AltModifier)
          native |= Mod1Mask;
      if (modifiers & Qt::MetaModifier)
          native |= Mod4Mask;
      return native;
}

static inline quint32 nativeKey(Qt::Key key)
{
	KeySym keysym = XStringToKeysym(QKeySequence(key).toString().toLatin1().data());
	if (keysym == NoSymbol)
		keysym = KeySym(key);
	return XKeysymToKeycode(QX11Info::display(), keysym);
}

/*
 *
 * GlobalShortcutImpl class implementation for X11
 *
 */
GlobalShortcutImpl::GlobalShortcutImpl(QObject *parent) : QObject(parent), ks_(), nativeKey_(0),
	  nativeMod_(0), active_(false), triggerWhenFocused_(false)
{
}

GlobalShortcutImpl::~GlobalShortcutImpl()
{
	setActive(false);
}

bool GlobalShortcutImpl::active() const
{
	return active_;
}

bool GlobalShortcutImpl::grabKey()
{
	CX11ErrorHandler errorHandler;
	Display *display = dpy();
	Window   window  = DefaultRootWindow(display);

	for (int i = 0; i < maskModifiers.size() && !errorHandler.error; i++)
		XGrabKey(display, nativeKey_, nativeMod_ | maskModifiers[i], window, True, GrabModeAsync, GrabModeAsync);

	if (errorHandler.error) {
		qDebug() << "FAILED TO GRAB KEY" << ks_ << ":" << errorHandler.errorString;
		ungrabKey();
		return false;
	}

	return true;
}

bool GlobalShortcutImpl::ungrabKey()
{
	CX11ErrorHandler errorHandler;
	Display *display = dpy();
	Window   window  = DefaultRootWindow(display);

	for (int i = 0; i < maskModifiers.size(); i++)
		XUngrabKey(display, nativeKey_, nativeMod_ | maskModifiers[i], window);

	if (errorHandler.error) {
		qDebug() << "FAILED TO UNGRAB KEY" << ks_ << ":" << errorHandler.errorString;
		return false;
	}

	return true;
}

void GlobalShortcutImpl::setActive(bool onoff)
{
	if (onoff) {
		if (active_) return;
		if (grabKey()) {
			qApp->installNativeEventFilter(this);
			active_ = true;
			qDebug() << ks_ << "ACTIVE";
		}
	} else {
		if (!active_) return;
		if (ungrabKey()) {
			qApp->removeNativeEventFilter(this);
			active_ = false;
			qDebug() << ks_ << "INACTIVE";
		}
	}
}

QKeySequence GlobalShortcutImpl::keySequence() const
{
	return ks_;
}

void GlobalShortcutImpl::setKeySequence(const QKeySequence &qks)
{
	if (qks == ks_) return;

	bool active = active_;
	setActive(false);

	int allMods = int(Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
	int ks = qks.count() ? qks[0] : 0;
	nativeMod_ = nativeModifiers(Qt::KeyboardModifiers(ks & allMods));
	nativeKey_ = nativeKey(Qt::Key(ks & ~allMods));
	ks_        = qks;

	setActive(active);
}

bool GlobalShortcutImpl::triggeredWhenFocused() const
{
	return triggerWhenFocused_;
}

void GlobalShortcutImpl::setTriggeredWhenFocused(bool yesno)
{
	triggerWhenFocused_ = yesno;
}

bool GlobalShortcutImpl::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
	Q_UNUSED(result);
	quint8 evt;

	if (!triggerWhenFocused_ && qApp->applicationState() == Qt::ApplicationActive)
		return false;

	xcb_key_press_event_t *kev = 0;
	if (eventType == "xcb_generic_event_t") {
		xcb_generic_event_t *ev = (xcb_generic_event_t *)message;
		evt = ev->response_type & 127;
		if (evt == XCB_KEY_PRESS || evt == XCB_KEY_RELEASE)
			kev = (xcb_key_press_event_t *)message;
	}

	if (kev && evt == XCB_KEY_PRESS) {
		quint32 keycode  = kev->detail;
		quint32 keystate = 0;
		if(kev->state & XCB_MOD_MASK_1)       keystate |= Mod1Mask;
		if(kev->state & XCB_MOD_MASK_CONTROL) keystate |= ControlMask;
		if(kev->state & XCB_MOD_MASK_4)       keystate |= Mod4Mask;
		if(kev->state & XCB_MOD_MASK_SHIFT)   keystate |= ShiftMask;

		if (nativeKey_ == keycode && nativeMod_ == keystate) {
			qDebug() << ks_;
			emit activated();
			return true;
		}
	}

	return false;
}
