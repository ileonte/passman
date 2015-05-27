#include "wallet.h"
#include "wallet_buffer.h"
#include "wallet_stringmap.h"
#include "wallet_stringlist.h"

#include <QDebug>
#include <QCoreApplication>
#include <kwallet.h>

using namespace WalletBackend;

static StringList wsl;
static Buffer wbf;
static StringMap wsm;
static char sbuf[1024];

static StringList *listWallets()
{
	return QSLtoWSL(KWallet::Wallet::walletList(), &wsl);
}

static Wallet openWallet(const char *appName, const char *walletName, unsigned long wid)
{
	QCoreApplication::setApplicationName(appName);
	return KWallet::Wallet::openWallet(walletName, wid);
}

static void closeWallet(Wallet wallet)
{
	auto w = (KWallet::Wallet *)wallet;
	if (w) {
		KWallet::Wallet::closeWallet(w->walletName(), false);
		delete w;
	}
}

static StringList *folderList(Wallet wallet)
{
	auto w = (KWallet::Wallet *)wallet;
	return QSLtoWSL(w->folderList(), &wsl);
}

static int folderExists(Wallet wallet, const char *folderName)
{
	auto w = (KWallet::Wallet *)wallet;
	return (int)w->hasFolder(folderName);
}

static int folderSet(Wallet wallet, const char *folderName)
{
	auto w = (KWallet::Wallet *)wallet;
	return (int)w->setFolder(folderName);
}

static int folderDelete(Wallet wallet, const char *folderName)
{
	auto w = (KWallet::Wallet *)wallet;
	return (int)w->removeFolder(folderName);
}

static int folderCreate(Wallet wallet, const char *folderName)
{
	auto w = (KWallet::Wallet *)wallet;
	return w->createFolder(folderName);
}

static const char *folderCurrent(Wallet wallet)
{
	auto w = (KWallet::Wallet *)wallet;
	snprintf(sbuf, sizeof(sbuf), "%s", w->currentFolder().toUtf8().data());
	return sbuf;
}

static StringList *entryList(Wallet wallet)
{
	auto w = (KWallet::Wallet *)wallet;
	return QSLtoWSL(w->entryList(), &wsl);
}

static int entryExists(Wallet wallet, const char *key)
{
	auto w = (KWallet::Wallet *)wallet;
	return (int)w->hasEntry(key);
}

static Buffer *entryRead(Wallet wallet, const char *key)
{
	QByteArray ar;
	auto w = (KWallet::Wallet *)wallet;
	w->readEntry(key, ar);
	return QBAtoWBF(ar, &wbf);
}

static int entryWrite(Wallet wallet, EntryType type, const char *key, const void *data, int size)
{
	auto w = (KWallet::Wallet *)wallet;
	return (int)w->writeEntry(key, QByteArray((const char *)data, size), (KWallet::Wallet::EntryType)type);
}

static EntryType entryType(Wallet wallet, const char *key)
{
	auto w = (KWallet::Wallet *)wallet;
	return (EntryType)w->entryType(key);
}

static int entryRename(Wallet wallet, const char *oldName, const char *newName)
{
	auto w = (KWallet::Wallet *)wallet;
	return (int)w->renameEntry(oldName, newName);
}

static int entryDelete(Wallet wallet, const char *key)
{
	auto w = (KWallet::Wallet *)wallet;
	return (int)w->removeEntry(key);
}

static int passWrite(Wallet wallet, const char *key, const char *passwd)
{
	auto w = (KWallet::Wallet *)wallet;
	return (int)w->writePassword(key, passwd);
}

static const char *passRead(Wallet wallet, const char *key)
{
	auto w = (KWallet::Wallet *)wallet;
	QString p;
	if (w->readPassword(key, p))
		return nullptr;
	snprintf(sbuf, sizeof(sbuf), "%s", p.toUtf8().data());
	return sbuf;
}

static int mapWrite(Wallet wallet, const char *key, StringMap *map)
{
	auto w = (KWallet::Wallet *)wallet;
	return (int)w->writeMap(key, WSMtoQSM(map));
}

static StringMap *mapRead(Wallet wallet, const char *key)
{
	QMap<QString, QString> m;
	auto w = (KWallet::Wallet *)wallet;
	w->readMap(key, m);
	return QSMtoQSM(m, &wsm);
}

extern "C" int passmanWalletBackendInit(LibraryInterface *intf)
{
	intf->intfPluginUnixName = "kwallet4";
	intf->intfPluginName     = "KWallet (KDE4)";
	intf->intfPluginVersion  = "0.1.1";

	intf->listWallets = &listWallets;

	intf->openWallet = &openWallet;
	intf->closeWallet = &closeWallet;

	intf->folderList = &folderList;
	intf->folderCreate = &folderCreate;
	intf->folderCurrent = &folderCurrent;
	intf->folderDelete = &folderDelete;
	intf->folderExists = &folderExists;
	intf->folderSet = &folderSet;

	intf->entryDelete = &entryDelete;
	intf->entryExists = &entryExists;
	intf->entryList = &entryList;
	intf->entryRead = &entryRead;
	intf->entryRename = &entryRename;
	intf->entryType = &entryType;
	intf->entryWrite = &entryWrite;

	intf->mapRead = &mapRead;
	intf->mapWrite = &mapWrite;

	intf->passRead = &passRead;
	intf->passWrite = &passWrite;

	return 0;
}
