#ifndef WALLET_STRINGMAP_H
#define WALLET_STRINGMAP_H

#include <QMap>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace WalletBackend {
	/*
	 * String map routines
	 */
	struct StringMap {
		char **keys;
		char **vals;
		int    count;
	};

	static inline void stringMapInit(StringMap *wsm)
	{
		memset(wsm, 0, sizeof(*wsm));
	}

	static inline void stringMapEmptry(StringMap *wsm)
	{
		for (int i = 0; i < wsm->count; i++) {
			free(wsm->keys[i]);
			free(wsm->vals[i]);
		}
		free(wsm->keys);
		free(wsm->vals);
		memset(wsm, 0, sizeof(*wsm));
	}

	static inline void stringMapAdd(StringMap *wsm, const char *key, const char *val)
	{
		if (key && val) {
			wsm->keys = (char **)realloc(wsm->keys, (wsm->count + 1) * sizeof(char *));
			wsm->vals = (char **)realloc(wsm->vals, (wsm->count + 1) * sizeof(char *));
			wsm->keys[wsm->count] = strdup(key);
			wsm->vals[wsm->count] = strdup(val);
			wsm->count++;
		}
	}

	static inline QMap<QString, QString> WSMtoQSM(StringMap *wsm)
	{
		QMap<QString, QString> ret;
		for (int i = 0; i < wsm->count; i++)
			ret.insert(wsm->keys[i], wsm->vals[i]);
		return ret;
	}

	static inline StringMap *QSMtoQSM(const QMap<QString, QString> &qsm, StringMap *wsm)
	{
		stringMapEmptry(wsm);
		for (QMap<QString, QString>::const_iterator it = qsm.begin(); it != qsm.end(); it++)
			stringMapAdd(wsm, it.key().toUtf8().data(), it.value().toUtf8().data());
		return wsm;
	}
}

#endif // WALLET_STRINGMAP_H
