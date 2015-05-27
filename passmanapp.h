#ifndef PASSMANAPP_H
#define PASSMANAPP_H

#include <QObject>
#include <QWidget>
#include <QLockFile>
#include <QDir>
#include <QApplication>
#include <QLockFile>
#include <QVector>
#include <QMap>
#include <QVector>
#include <QLibrary>
#include <QStringList>
#include <QString>
#include <QKeySequence>

#include "wallets/wallet.h"
#include "searchwidget.h"
#include "mainwindow.h"
#include "platforms/globalshortcut.h"
#include "passwordmodel.h"
#include "passwordmanager.h"

typedef QVector<WalletBackend::LibraryInterface *> PMBAckendList;

class PassManApp : public QApplication
{
	Q_OBJECT

public:
	explicit PassManApp(int &argc, char **argv);
	virtual ~PassManApp();

	bool init();

	QStringList backendNames() const;
	QStringList backendUnixNames() const;

	int backendCount() const;

	int activeBackend() const;

	QStringList walletNames() const;

	/*
	 * The walletPasswordEntries(), walletGetPassword(), walletSetPassword() and
	 * walletDeletePasswords() functions operate on the current open wallet
	 * (wallet_) and the currentwallet directory. The current wallet and wallet
	 * directory are set by setWalletName()
	 */
	QStringList walletPasswordEntries() const;
	bool walletGetPassword(const QString &key, QString &password) const;
	bool walletSetPassword(const QString &key, const QString &password) const;
	void walletDeletePasswords(const QStringList &keys);

	QString walletName() const;

	GlobalShortcut *shortcutSearch() const;
	GlobalShortcut *shortcutQuit() const;
	GlobalShortcut *shortcutSettings() const;
	GlobalShortcut *shortcutManage() const;

	PasswordModel *dataModel() const;

signals:
	void activeBackendChanged(int i);
	void walletNameChanged(const QString &newName);

public slots:
	void loadBackends();
	void setWalletName(const QString &name);
	void setActiveBackend(int idx);
	void showPasswordManager();
	void refreshWalletData();

private:
	QLockFile appLock_;

	PMBAckendList backends_;
	int activeBackend_;
	QString walletName_;

	QStringList walletNames_;
	WalletBackend::Wallet wallet_;

	MainWindow *mainWindow_;
	SearchWidget *searchWidget_;
	PasswordManager *passwordManager_;

	GlobalShortcut *gsSearch_;
	GlobalShortcut *gsQuit_;
	GlobalShortcut *gsSettings_;
	GlobalShortcut *gsManage_;

	PasswordModel *passwordData_;
};

#define myApp (static_cast<PassManApp *>(QCoreApplication::instance()))

#endif // PASSMANAPP_H
