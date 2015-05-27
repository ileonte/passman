#ifndef PASSWORDMODEL_H
#define PASSWORDMODEL_H

#include <QRegularExpression>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QVariant>
#include <QModelIndex>

class PasswordModel;
class PasswordFilterModel;

class PasswordModel : public QAbstractListModel {
	Q_OBJECT

public:
	struct Item {
		QString proto;
		QString user;
		QString host;
		QString key;
	};

	enum PasswordRoles {
		WalletKeyName = Qt::UserRole + 1
	};

	explicit PasswordModel(QObject *parent = 0);
	virtual ~PasswordModel();

	virtual int rowCount(const QModelIndex &parent) const;
	virtual int columnCount(const QModelIndex &parent) const;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	virtual QVariant data(const QModelIndex &index, int role) const;

public slots:
	void setPasswordData(const QStringList &urls);

private:
	QVector<Item> passwordData_;
	QStringList columnNames_;

	friend class PasswordFilterModel;
};

class PasswordFilterModel : public QSortFilterProxyModel {
	Q_OBJECT

public:
	explicit PasswordFilterModel(PasswordModel *parent = 0);
	virtual ~PasswordFilterModel();

	virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
	virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
public slots:

	void setFilters(const QString &str);

private:
	void configureFilter(const QString &filterData);

	QRegularExpression protoFilter_;
	QRegularExpression userFilter_;
	QRegularExpression hostFilter_;
};

#endif // PASSWORDMODEL_H
