#ifndef QGLOBALSHORTCUT_X11_H
#define QGLOBALSHORTCUT_X11_H

#include <QObject>
#include <QWidget>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>

class GlobalShortcutImpl : public QObject, public QAbstractNativeEventFilter
{
	Q_OBJECT

public:
	explicit GlobalShortcutImpl(QObject *parent);
	virtual ~GlobalShortcutImpl();

	bool active() const;
	bool triggeredWhenFocused() const;
	QKeySequence keySequence() const;

signals:
	void activated();

public slots:
	void setActive(bool onoff);
	void setKeySequence(const QKeySequence &qks);
	void setTriggeredWhenFocused(bool yesno);

protected:
	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

private:
	bool grabKey();
	bool ungrabKey();

	QKeySequence ks_;
	quint32 nativeKey_;
	quint32 nativeMod_;
	bool    active_;
	bool    triggerWhenFocused_;
};

#endif // QGLOBALSHORTCUT_X11_H
