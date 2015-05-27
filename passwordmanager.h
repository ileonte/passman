#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QWidget>
#include <QDialog>
#include <QTreeView>
#include <QPushButton>
#include <QItemSelection>
#include <QLineEdit>

#include "passwordmodel.h"
#include "clineedit.h"

class PasswordManager : public QWidget
{
public:
	explicit PasswordManager(QWidget *parent = 0);
	virtual ~PasswordManager();

public slots:
	void show();

private:
	Q_SLOT void dataViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	Q_SLOT void deletePasswords();

	Q_SLOT void refreshAddForm(const QString &text);
	Q_SLOT void addPassword();

	CLineEdit *filterEdit_;
	QTreeView *dataView_;
	QPushButton *btnDelete_;

	QLineEdit *edUrl_;
	QLineEdit *edUser_;
	QLineEdit *edPass_;
	QPushButton *btnAdd_;

	QPushButton *btnClose_;

	PasswordFilterModel *dataFilter_;
};

#endif // PASSWORDMANAGER_H
