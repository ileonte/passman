#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <QShortcut>
#include <QAction>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QComboBox>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <QPushButton>

#include "platforms/globalshortcut.h"

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	virtual ~MainWindow();

public slots:
	void show();
	void toggle();
	void populateWidgets();

private:
	Q_SLOT void systrayActivated(QSystemTrayIcon::ActivationReason reason);
	Q_SLOT void activeBackendChanged();
	Q_SLOT void walletNameChanged(const QString &name);

	void setGlobalShortcut(GlobalShortcut *shortcut, const QKeySequence &ks);
	void getGlobalShortcut(GlobalShortcut *shortcut, QKeySequenceEdit *edit);

	QStringList walletNames_;

	QComboBox *cbBackends_;
	QComboBox *cbWallets_;

	QKeySequenceEdit *kseSettings_;
	QKeySequenceEdit *kseQuit_;
	QKeySequenceEdit *kseSearch_;
	QKeySequenceEdit *kseManage_;

	QPushButton *btnAbout_;
	QPushButton *btnClose_;
	QPushButton *btnManage_;
	QPushButton *btnQuit_;

	QSystemTrayIcon *trayIcon_;
};

#endif // MAINWINDOW_H
