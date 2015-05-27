#ifndef WALLET_H
#define WALLET_H

#include <qglobal.h>
#include <QDebug>

#if defined(Q_OS_LINUX)
#include <dlfcn.h>
#endif

/*
 *
 * WARNING: the wallet backend interface IS NOT REENTRANT, NOT THREAD-SAFE AND 100% NOT POLITICALLY-CORRECT
 *
 */

namespace WalletBackend {
	/*
	 * Wallet backend interface
	 */

	struct StringList;
	struct StringMap;
	struct Buffer;

	typedef void *Wallet;

	typedef enum _WALLET_ENTRY_TYPE {
		WALLET_ENTRY_UNKNOWN = 0,
		WALLET_ENTRY_PASSWORD,
		WALLET_ENTRY_STREAM,
		WALLET_ENTRY_MAP
	} EntryType;

	typedef StringList* (* walletList_fn)(void);

	typedef Wallet (* walletOpen_fn)(const char *appName, const char *walletName, unsigned long wid);
	typedef void (* walletClose_fn)(Wallet wallet);

	typedef StringList* (* walletFolderList_fn)(Wallet wallet);
	typedef int (* walletHasFolder_fn)(Wallet wallet, const char *folderName);
	typedef int (* walletSetFolder_fn)(Wallet wallet, const char *folderName);
	typedef int (* walletDelFolder_fn)(Wallet wallet, const char *folderName);
	typedef int (* walletMakeFolder_fn)(Wallet wallet, const char *folderName);
	typedef const char * (* walletCurrentFolder_fn)(Wallet wallet);

	typedef StringList* (* walletEntryList_fn)(Wallet wallet);
	typedef int (* walletEntryExists_fn)(Wallet wallet, const char *key);
	typedef Buffer* (* walletReadEntry_fn)(Wallet wallet, const char *key);
	typedef int (* walletWriteEntry_fn)(Wallet wallet, EntryType type, const char *key, const void *data, int size);
	typedef EntryType (* walletEntryType_fn)(Wallet wallet, const char *key);
	typedef int (* walletRenameEntry_fn)(Wallet wallet, const char *oldName, const char *newName);
	typedef int (* walletDelEntry_fn)(Wallet wallet, const char *key);

	typedef int (* walletWritePassword_fn)(Wallet wallet, const char *key, const char *passwd);
	typedef const char * (* walletReadPassword_fn)(Wallet wallet, const char *key);

	typedef int (* walletWriteMap_fn)(Wallet wallet, const char *key, StringMap *map);
	typedef StringMap* (* walletReadMap_fn)(Wallet wallet, const char *key);

	typedef struct _WALLET_INTF {
		const char *intfPluginUnixName;
		const char *intfPluginName;
		const char *intfPluginVersion;

		walletList_fn          listWallets;

		walletOpen_fn          openWallet;
		walletClose_fn         closeWallet;

		walletFolderList_fn    folderList;
		walletHasFolder_fn     folderExists;
		walletSetFolder_fn     folderSet;
		walletDelEntry_fn      folderDelete;
		walletMakeFolder_fn    folderCreate;
		walletCurrentFolder_fn folderCurrent;

		walletEntryList_fn     entryList;
		walletEntryExists_fn   entryExists;
		walletReadEntry_fn     entryRead;
		walletWriteEntry_fn    entryWrite;
		walletEntryType_fn     entryType;
		walletRenameEntry_fn   entryRename;
		walletDelEntry_fn      entryDelete;

		walletReadPassword_fn  passRead;
		walletWritePassword_fn passWrite;

		walletReadMap_fn       mapRead;
		walletWriteMap_fn      mapWrite;
	} LibraryInterface;

	typedef int (* passmanWalletBackendInit_fn)(LibraryInterface *intf);

	/*
	 * Wallet backend lib functions
	 */
	static inline long _wlt_open_lib(const char *path)
	{
	#if defined(Q_OS_LINUX)
		dlerror();
		long ret = (long)dlopen(path, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
		if (!ret)
			qDebug() << "Failed to load" << path << ":" << dlerror();
		return ret;
	#else
		return 0;
	#endif
	}

	static inline void *_wlt_get_sym(long lib, const char *name)
	{
	#if defined(Q_OS_LINUX)
		return dlsym((void *)lib, name);
	#else
		return NULL;
	#endif
	}

	static inline int loadLibrary(const char *libName, LibraryInterface *intf)
	{
		passmanWalletBackendInit_fn initFn;
		int r;
		long lib = _wlt_open_lib(libName);
		if (!lib)
			return -1;

#define WLT_SYM(where, name) { \
	where = (decltype(where))dlsym((void *)lib, name); \
	if (!where) { \
		qCritical("Failed to load wallet backend library %s: could not find symbol '%s'", libName, name); \
		return -1; \
	} \
}

#define WLT_CHECK(name) { \
	if (!intf->name) { \
		qDebug() << "Failed to load wallet: missing init symbol" << #name; \
		return -1; \
	} \
	qDebug("WALLET_SYM(%s): %p", #name, intf->name); \
}

		WLT_SYM(initFn, "passmanWalletBackendInit");
		if ((r = initFn(intf)) != 0) {
			qCritical("Failed to load wallet backend library %s: init function returned an error (%d)", libName, r);
			return -1;
		}

		WLT_CHECK(intfPluginUnixName);
		WLT_CHECK(intfPluginName);
		WLT_CHECK(intfPluginVersion);

		WLT_CHECK(listWallets);

		WLT_CHECK(openWallet);
		WLT_CHECK(closeWallet);

		WLT_CHECK(folderList);
		WLT_CHECK(folderExists);
		WLT_CHECK(folderSet);
		WLT_CHECK(folderDelete);
		WLT_CHECK(folderCreate);
		WLT_CHECK(folderCurrent);

		WLT_CHECK(entryList);
		WLT_CHECK(entryExists);
		WLT_CHECK(entryRead);
		WLT_CHECK(entryWrite);
		WLT_CHECK(entryType);
		WLT_CHECK(entryRename);
		WLT_CHECK(entryDelete);

		WLT_CHECK(passRead);
		WLT_CHECK(passWrite);

		WLT_CHECK(mapRead);
		WLT_CHECK(mapWrite);

#undef WLT_SYM
#undef WLT_CHECK

		return 0;
	}
}

#endif // WALLET_H
