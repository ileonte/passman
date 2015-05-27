#ifndef QGLOBALSHORTCUT_H
#define QGLOBALSHORTCUT_H

#include <QObject>
#include <QWidget>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>

class GlobalShortcutImpl;

class GlobalShortcut : public QObject
{
	Q_OBJECT

public:
	explicit GlobalShortcut(QObject *parent);
	virtual ~GlobalShortcut();

	bool active() const;
	bool triggeredWhenFocused() const;
	QKeySequence keySequence() const;

signals:
	void activated();

public slots:
	void setActive(bool onoff);
	void setKeySequence(const QKeySequence &qks);
	void setTriggeredWhenFocused(bool yesno);

private:
	GlobalShortcutImpl *impl_;
};

#endif // QGLOBALSHORTCUT_H
