#ifndef PLATFORMHELPER_X11_H
#define PLATFORMHELPER_X11_H

#include <QObject>
#include <QWidget>

class PlatformHelperImpl : public QObject {
	Q_OBJECT

public:
	explicit PlatformHelperImpl(QObject *parent = 0);
	virtual ~PlatformHelperImpl();

	bool isShownOnAllDesktops(QWidget *w);
	void setShownOnAllDesktops(QWidget *w);

	bool isShownOnSingleDesktop(QWidget *w);
	void setShownOnSingleDesktop(QWidget *w);

	void removeTaskbarIcon(QWidget *w);

	bool beginNativeDrag(QWidget *w, const QPoint &dragPoint);

	bool popUp(QWidget *w);

	WId getActiveWindow();
	QRect getActiveWindowGeometry();

	void sendTextToActiveWindow(const QString &text);
};

#endif  // PLATFORMHELPER_X11_H
