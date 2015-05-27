#ifndef WALLET_STRINGLIST_H
#define WALLET_STRINGLIST_H

#include <QStringList>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace WalletBackend {
	/*
	 * String list routines
	 */
	struct StringList {
		char **items;
		int    count;
	};

	static inline void stringListInit(StringList *list)
	{
		memset(list, 0, sizeof(*list));
	}

	static inline void stringListEmpty(StringList *list)
	{
		for (int i = 0; i < list->count; i++)
			free(list->items[i]);
		free(list->items);
		memset(list, 0, sizeof(*list));
	}

	static inline void stringListAdd(StringList *list, const char *string)
	{
		if (string) {
			list->items = (char **)realloc(list->items, (list->count + 1) * sizeof(char *));
			list->items[list->count] = strdup(string);
			list->count++;
		}
	}

	static inline QStringList WSLtoQSL(StringList *wsl)
	{
		QStringList ret;
		for (int i = 0; i < wsl->count; i++)
			ret.append(wsl->items[i]);
		return ret;
	}

	static inline StringList *QSLtoWSL(const QStringList &qsl, StringList *wsl)
	{
		stringListEmpty(wsl);
		for (int i = 0; i < qsl.size(); i++)
			stringListAdd(wsl, qsl.at(i).toUtf8().data());
		return wsl;
	}
}

#endif // WALLET_STRINGLIST_H
