#include <QScreen>
#include <QWindow>
#include <QX11Info>
#include <QList>
#include <QApplication>

#include "platformhelper.h"
#if defined(Q_OS_LINUX)
#include "platforms/x11/platformhelper_x11.h"
#endif

PlatformHelper::PlatformHelper(QObject *parent) : QObject(parent)
{
	impl_ = new PlatformHelperImpl(this);
}

PlatformHelper::~PlatformHelper()
{
}

bool PlatformHelper::isShownOnAllDesktops(QWidget *w)
{
	return impl_->isShownOnAllDesktops(w);
}

void PlatformHelper::setShownOnAllDesktops(QWidget *w)
{
	impl_->setShownOnAllDesktops(w);
}

bool PlatformHelper::isShownOnSingleDesktop(QWidget *w)
{
	return impl_->isShownOnSingleDesktop(w);
}

void PlatformHelper::setShownOnSingleDesktop(QWidget *w)
{
	impl_->setShownOnSingleDesktop(w);
}

void PlatformHelper::removeTaskbarIcon(QWidget *w)
{
	impl_->removeTaskbarIcon(w);
}

bool PlatformHelper::beginNativeDrag(QWidget *w, const QPoint &dragPoint)
{
	return impl_->beginNativeDrag(w, dragPoint);
}

void PlatformHelper::popUp(QWidget *w)
{
	if (impl_->popUp(w))
		qApp->setActiveWindow(w);
}

QRect PlatformHelper::getActiveWindowGeometry()
{
	return impl_->getActiveWindowGeometry();
}

QScreen *PlatformHelper::getActiveWindowScreen()
{
	QRect ar(getActiveWindowGeometry());
	QScreen *s = qApp->screens().at(0);

	foreach (QScreen *it, qApp->screens()) {
		if (it->geometry().contains(ar.topLeft())) {
			s = it;
			break;
		}
	}

	return s;
}

void PlatformHelper::centerOnActiveScreen(QWidget *w)
{
	QScreen *s = getActiveWindowScreen();
	if (s)
		w->move(s->geometry().center() - w->rect().center());
}

static PlatformHelper *__platformHelper = NULL;

PlatformHelper *platformHelper()
{
	if (!__platformHelper)
		__platformHelper = new PlatformHelper(qApp);

	return __platformHelper;
}
