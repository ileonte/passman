#ifndef WALLET_BUFFER_H
#define WALLET_BUFFER_H

#include <QByteArray>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace WalletBackend {
	/*
	 * Buffer routines
	 */
	struct Buffer {
		void *data;
		int   size;
	};

	void bufferInit(Buffer *wbf)
	{
		memset(wbf, 0, sizeof(*wbf));
	}

	void bufferEmpty(Buffer *wbf)
	{
		free(wbf->data);
		memset(wbf, 0, sizeof(*wbf));
	}

	void bufferSet(Buffer *wbf, const void *data, int size)
	{
		if (size) {
			wbf->data = malloc(size);
			memcpy(wbf->data, data, size);
			wbf->size = size;
		}
	}

	static inline QByteArray WBFtoQBA(Buffer *wbf)
	{
		QByteArray ret;
		if (wbf->size) ret.append((const char *)wbf->data, wbf->size);
		return ret;
	}

	static inline Buffer *QBAtoWBF(const QByteArray &qba, Buffer *wbf)
	{
		bufferInit(wbf);
		bufferSet(wbf, qba.data(), qba.size());
		return wbf;
	}
}

#endif // WALLET_BUFFER_H
