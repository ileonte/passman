#include <QUrl>

#include "passwordmodel.h"
#include "utils.h"

#define MATCH_ALL ".*"

PasswordModel::PasswordModel(QObject *parent) : QAbstractListModel(parent)
{
	columnNames_ << tr("Protocol") << tr("User") << tr("Host");
}

PasswordModel::~PasswordModel()
{
}

int PasswordModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return passwordData_.size();
}

int PasswordModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return columnNames_.size();
}

QVariant PasswordModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return columnNames_[section];
	return QVariant();
}

QVariant PasswordModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < passwordData_.size()) {
		const Item &it = passwordData_.at(index.row());
		if (role == Qt::DisplayRole) {
			switch (index.column()) {
				case 0: return it.proto;
				case 1: return it.user;
				case 2: return it.host;
				default: break;
			}
		}
		if (role == PasswordModel::WalletKeyName)
			return it.key;
	}

	return QVariant();
}

void PasswordModel::setPasswordData(const QStringList &urls)
{
	beginResetModel();
	{
		passwordData_.clear();
		foreach (auto us, urls) {
			QUrl url(us);
			if (!url.isValid()) continue;

			Item it;
			it.proto = url.scheme();
			it.user  = url.userName();
			it.host  = url.host();
			it.key   = us;

			passwordData_.append(it);
		}
	}
	endResetModel();
}

PasswordFilterModel::PasswordFilterModel(PasswordModel *parent) : QSortFilterProxyModel(parent),
      protoFilter_(MATCH_ALL, QRegularExpression::CaseInsensitiveOption),
      userFilter_(MATCH_ALL, QRegularExpression::CaseInsensitiveOption),
      hostFilter_(MATCH_ALL, QRegularExpression::CaseInsensitiveOption)
{
	setSourceModel(parent);
}

PasswordFilterModel::~PasswordFilterModel()
{
}

bool PasswordFilterModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
	Q_UNUSED(source_column);
	Q_UNUSED(source_parent);
	return true;
}

bool PasswordFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	Q_UNUSED(source_parent);

	auto pm = (const PasswordModel *)sourceModel();
	if (source_row < 0 || source_row >= pm->passwordData_.size()) return true;
	const PasswordModel::Item &it = pm->passwordData_.at(source_row);
	bool ret = true;

	if (!hostFilter_.pattern().isEmpty())  ret &= hostFilter_.match(it.host).hasMatch();
	if (!userFilter_.pattern().isEmpty())  ret &= userFilter_.match(it.user).hasMatch();
	if (!protoFilter_.pattern().isEmpty()) ret &= protoFilter_.match(it.proto).hasMatch();

	return ret;
}

void PasswordFilterModel::configureFilter(const QString &filterData)
{
	if (filterData.startsWith("h:", Qt::CaseInsensitive)) {
		QString fd = QRegularExpression::escape(filterData.mid(2));
		hostFilter_.setPattern(QString("%1%2").arg(MATCH_ALL).arg(fd));
		hostFilter_.optimize();
	} else if (filterData.startsWith("u:", Qt::CaseInsensitive)) {
		QString fd = QRegularExpression::escape(filterData.mid(2));
		userFilter_.setPattern(QString("%1%2").arg(MATCH_ALL).arg(fd));
		userFilter_.optimize();
	} else if (filterData.startsWith("p:", Qt::CaseInsensitive)) {
		QString fd = QRegularExpression::escape(filterData.mid(2));
		protoFilter_.setPattern(QString("%1%2").arg(MATCH_ALL).arg(fd));
		protoFilter_.optimize();
	} else {
		if (hostFilter_.pattern() == MATCH_ALL) {
			QString fd = QRegularExpression::escape(filterData);
			hostFilter_.setPattern(QString(".*%1").arg(fd));
			hostFilter_.optimize();
		} else if (userFilter_.pattern() == MATCH_ALL) {
			QString fd = QRegularExpression::escape(filterData);
			userFilter_.setPattern(QString(".*%1").arg(fd));
			userFilter_.optimize();
		} else if (protoFilter_.pattern() == MATCH_ALL) {
			QString fd = QRegularExpression::escape(filterData);
			protoFilter_.setPattern(QString(".*%1").arg(fd));
			protoFilter_.optimize();
		}
	}
}

void PasswordFilterModel::setFilters(const QString &str)
{
	QStringList parts;
	beginResetModel();
	do {
		hostFilter_.setPattern(MATCH_ALL);
		userFilter_.setPattern(MATCH_ALL);
		protoFilter_.setPattern(MATCH_ALL);
		if (str.isEmpty()) break;

		Utils::stringBreak(str, parts);
		foreach (auto part, parts) {
			if (!part.isEmpty()) configureFilter(part);
		}
	} while (0);
	endResetModel();
}
