#ifndef __X11HELPER_H_INCLUDED__
#define __X11HELPER_H_INCLUDED__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <QMap>
#include <QWidget>
#include <QWindow>
#include <QScreen>
#include <QStringList>
#include <QApplication>

class PlatformHelperImpl;

class PlatformHelper : public QObject {
public:
	Q_OBJECT

public:
	PlatformHelper(QObject *parent);
	virtual ~PlatformHelper();

	bool isShownOnAllDesktops(QWidget *w);
	void setShownOnAllDesktops(QWidget *w);

	bool isShownOnSingleDesktop(QWidget *w);
	void setShownOnSingleDesktop(QWidget *w);

	void removeTaskbarIcon(QWidget *w);

	bool beginNativeDrag(QWidget *w, const QPoint &dragPoint);

	void popUp(QWidget *w);

	QRect getActiveWindowGeometry();
	QScreen *getActiveWindowScreen();
	void centerOnActiveScreen(QWidget *w);

private:
	PlatformHelperImpl *impl_;
};

PlatformHelper *platformHelper();

#endif  /* __X11HELPER_H_INCLUDED__ */
