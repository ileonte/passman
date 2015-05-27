#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QFrame>

#include "passwordmodel.h"

class CLineEdit;
class SearchWidget : public QFrame
{
	Q_OBJECT

public:
	explicit SearchWidget(QWidget *parent = 0);

signals:

public slots:
	void toggle();
	void show();

protected:

private:
	Q_SLOT void copyPasswordToClipboard();
	Q_SLOT void goUp();
	Q_SLOT void goDown();

	CLineEdit *searchField_;
	QTreeView *searchResults_;

	PasswordFilterModel *dataFilter_;
};

#endif // SEARCHWIDGET_H
