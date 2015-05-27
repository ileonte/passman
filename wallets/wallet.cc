#include "wallet.h"
#include "wallet_buffer.h"
#include "wallet_stringmap.h"
#include "wallet_stringlist.h"

#include <QDebug>
#include <QCoreApplication>
#include <QMap>
#include <QString>

using namespace WalletBackend;

static StringList wsl;
static Buffer wbf;
static StringMap wsm;
static char sbuf[1024];

static QMap<QString, QString> passwords;
static QStringList folders({"PassMan"});
static QString currentFolder;

static StringList *listWallets()
{
	return QSLtoWSL(QStringList({"Dummy wallet"}), &wsl);
}

static Wallet openWallet(const char *appName, const char *walletName, unsigned long wid)
{
	Q_UNUSED(appName);
	Q_UNUSED(walletName);
	Q_UNUSED(wid);
	return Wallet(0xdeadbeef);
}

static void closeWallet(Wallet wallet)
{
	Q_UNUSED(wallet);
}

static StringList *folderList(Wallet wallet)
{
	Q_UNUSED(wallet);
	return QSLtoWSL(folders, &wsl);
}

static int folderExists(Wallet wallet, const char *folderName)
{
	Q_UNUSED(wallet);
	return folders.contains(folderName) ? 1 : 0;
}

static int folderSet(Wallet wallet, const char *folderName)
{
	Q_UNUSED(wallet);
	if (folders.contains(folderName)) {
		currentFolder = folderName;
		return 1;
	}
	return 0;
}

static int folderDelete(Wallet wallet, const char *folderName)
{
	Q_UNUSED(wallet);
	int idx = folders.indexOf(folderName);
	if (idx >= 0) {
		folders.removeAt(idx);
		if (currentFolder == folderName) currentFolder = "";
		return 1;
	}
	return 0;
}

static int folderCreate(Wallet wallet, const char *folderName)
{
	Q_UNUSED(wallet);
	if (!folders.contains(folderName)) {
		folders.append(folderName);
		return 1;
	}
	return 0;
}

static const char *folderCurrent(Wallet wallet)
{
	Q_UNUSED(wallet);
	snprintf(sbuf, sizeof(sbuf), "%s", currentFolder.toUtf8().data());
	return sbuf;
}

static StringList *entryList(Wallet wallet)
{
	Q_UNUSED(wallet);
	return QSLtoWSL(passwords.keys(), &wsl);
}

static int entryExists(Wallet wallet, const char *key)
{
	Q_UNUSED(wallet);
	return (int)(passwords.find(key) != passwords.end());
}

static Buffer *entryRead(Wallet wallet, const char *key)
{
	Q_UNUSED(wallet);
	Q_UNUSED(key);
	return QBAtoWBF(QByteArray(), &wbf);
}

static int entryWrite(Wallet wallet, EntryType type, const char *key, const void *data, int size)
{
	Q_UNUSED(wallet);
	Q_UNUSED(type);
	Q_UNUSED(key);
	Q_UNUSED(data);
	Q_UNUSED(size);
	return -1;
}

static EntryType entryType(Wallet wallet, const char *key)
{
	Q_UNUSED(wallet);
	if (passwords.find(key) != passwords.end())
		return WALLET_ENTRY_PASSWORD;
	return WALLET_ENTRY_UNKNOWN;
}

static int entryRename(Wallet wallet, const char *oldName, const char *newName)
{
	Q_UNUSED(wallet);
	Q_UNUSED(oldName);
	Q_UNUSED(newName);
	return -1;
}

static int entryDelete(Wallet wallet, const char *key)
{
	Q_UNUSED(wallet);
	passwords.remove(key);
	return 0;
}

static int passWrite(Wallet wallet, const char *key, const char *passwd)
{
	Q_UNUSED(wallet);

	auto it = passwords.find(key);
	if (it != passwords.end())
		passwords[key] = passwd;
	else
		passwords.insert(key, passwd);
	return 0;
}

static const char *passRead(Wallet wallet, const char *key)
{
	Q_UNUSED(wallet);

	memset(sbuf, 0, sizeof(sbuf));

	auto it = passwords.find(key);
	if (it != passwords.end())
		snprintf(sbuf, sizeof(sbuf), "%s", it.value().toUtf8().data());
	return sbuf;
}

static int mapWrite(Wallet wallet, const char *key, StringMap *map)
{
	Q_UNUSED(wallet);
	Q_UNUSED(key);
	Q_UNUSED(map);
	return -1;
}

static StringMap *mapRead(Wallet wallet, const char *key)
{
	Q_UNUSED(wallet);
	Q_UNUSED(key);
	QMap<QString, QString> m;
	return QSMtoQSM(m, &wsm);
}

extern "C" int passmanWalletBackendInit(LibraryInterface *intf)
{
	intf->intfPluginUnixName = "dummy";
	intf->intfPluginName     = "Dummy Wallet Backend";
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
