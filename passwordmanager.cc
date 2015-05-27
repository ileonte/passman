#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QItemSelectionModel>
#include <QUrl>
#include <QMessageBox>

#include "passwordmanager.h"
#include "passmanapp.h"
#include "platforms/platformhelper.h"

static inline QFrame *hLine()
{
	auto frm = new QFrame;
	frm->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	return frm;
}

PasswordManager::PasswordManager(QWidget *parent) : QWidget(parent)
{
	setWindowTitle(tr("Password manager"));
	setMinimumSize(700, 550);

	dataFilter_ = new PasswordFilterModel(myApp->dataModel());

	auto layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);

	auto header = new QWidget(this);
	{
		header->setStyleSheet("background-color: white");

		auto hl = new QHBoxLayout(header);
		auto lb = new QLabel(tr("Password manager"));
		QFont f(lb->font());
		f.setBold(true);
		f.setPixelSize(16);
		lb->setFont(f);
		hl->addWidget(lb);

		hl->addStretch();

		lb = new QLabel(header);
		lb->setPixmap(QIcon::fromTheme("security-high").pixmap(48, 48));
		lb->setMinimumSize(48, 48);
		hl->addWidget(lb);
	}
	layout->addWidget(header);

	auto mainWidget = new QWidget(this);
	{
		auto mainLayout = new QGridLayout(mainWidget);

		auto hl = new QHBoxLayout;
		filterEdit_ = new CLineEdit(mainWidget);
		auto lb = new QLabel(tr("&Filter:"), mainWidget);
		lb->setBuddy(filterEdit_);
		hl->addWidget(lb);
		hl->addWidget(filterEdit_);
		mainLayout->addLayout(hl, 0, 0);

		dataView_ = new QTreeView(this);
		dataView_->setSelectionBehavior(QTreeView::SelectRows);
		dataView_->setSelectionMode(QTreeView::ExtendedSelection);
		mainLayout->addWidget(dataView_, 1, 0);

		auto vl = new QVBoxLayout;
		btnDelete_ = new QPushButton(tr("&Delete"));
		btnDelete_->setEnabled(false);
		vl->addWidget(btnDelete_);
		vl->addStretch();
		mainLayout->addLayout(vl, 1, 1);

		mainLayout->addWidget(hLine(), 2, 0, 1, 2);

		auto addWidget = new QWidget();
		{
			auto addLayout = new QGridLayout(addWidget);
			addLayout->setMargin(0);

			edUrl_  = new QLineEdit();
			edUser_ = new QLineEdit();
			edPass_ = new QLineEdit();

			lb = new QLabel(tr("&Url:"));
			lb->setBuddy(edUrl_);
			addLayout->addWidget(lb, 0, 0);
			addLayout->addWidget(edUrl_, 0, 1);

			lb = new QLabel(tr("User &name:"));
			lb->setBuddy(edUser_);
			addLayout->addWidget(lb, 1, 0);
			addLayout->addWidget(edUser_, 1, 1);

			lb = new QLabel(tr("&Password:"));
			lb->setBuddy(edPass_);
			addLayout->addWidget(lb, 2, 0);
			addLayout->addWidget(edPass_, 2, 1);
		}
		mainLayout->addWidget(addWidget, 3, 0);

		vl = new QVBoxLayout;
		btnAdd_ = new QPushButton(tr("&Add"));
		btnAdd_->setEnabled(false);
		vl->addWidget(btnAdd_);
		mainLayout->addLayout(vl, 3, 1);
	}
	layout->addWidget(mainWidget);

	layout->addWidget(hLine());

	auto w = new QWidget();
	{
		auto hl = new QHBoxLayout(w);
		btnClose_ = new QPushButton(tr("&Close"));
		hl->addStretch();
		hl->addWidget(btnClose_);
	}
	layout->addWidget(w);

	connect(filterEdit_, &CLineEdit::textChanged, dataFilter_, &PasswordFilterModel::setFilters);

	connect(btnDelete_, &QPushButton::clicked, this, &PasswordManager::deletePasswords);
	connect(btnClose_, &QPushButton::clicked, this, &PasswordManager::close);
	connect(btnAdd_, &QPushButton::clicked, this, &PasswordManager::addPassword);

	connect(edUrl_, &QLineEdit::textEdited, this, &PasswordManager::refreshAddForm);
	connect(edUser_, &QLineEdit::textEdited, this, &PasswordManager::refreshAddForm);
	connect(edPass_, &QLineEdit::textEdited, this, &PasswordManager::refreshAddForm);

	dataView_->setModel(dataFilter_);
	connect(dataView_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PasswordManager::dataViewSelectionChanged);

	refreshAddForm("");
}

PasswordManager::~PasswordManager()
{
}

void PasswordManager::show()
{
	QScreen *s = platformHelper()->getActiveWindowScreen();

	filterEdit_->setText(QString());
	dataFilter_->setFilters(QString());

	QWidget::show();
	if (s) move(s->geometry().center() - rect().center());
	platformHelper()->popUp(this);
	filterEdit_->setFocus();
}

void PasswordManager::dataViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(deselected);
	btnDelete_->setDisabled(selected.isEmpty());
}

void PasswordManager::deletePasswords()
{
	QStringList keys;

	foreach (auto idx, dataView_->selectionModel()->selectedIndexes()) {
		QString key = myApp->dataModel()->data(idx, PasswordModel::WalletKeyName).toString();
		if (!key.isEmpty()) keys.append(key);
	}

	myApp->walletDeletePasswords(keys);
	btnDelete_->setEnabled(false);
}

void PasswordManager::refreshAddForm(const QString &text)
{
	Q_UNUSED(text);

	QUrl url(edUrl_->text().trimmed());
	if (url.scheme().isEmpty() || url.host().isEmpty()) {
		edUser_->setEnabled(false);
		edPass_->setEnabled(false);
		btnAdd_->setEnabled(false);
		return;
	}

	edPass_->setEnabled(true);
	if (!url.userName().trimmed().isEmpty()) {
		edUser_->setEnabled(false);
		edUser_->setText(url.userName());
	} else {
		edUser_->setEnabled(true);
	}

	if (!edPass_->text().isEmpty() && !edUser_->text().isEmpty())
		btnAdd_->setEnabled(true);
	else
		btnAdd_->setEnabled(false);
}

void PasswordManager::addPassword()
{
	QUrl url(edUrl_->text().trimmed());
	QUrl dst;

	dst.setScheme(url.scheme());
	dst.setUserName(edUser_->text().trimmed());
	dst.setHost(url.host());

	if (myApp->walletSetPassword(dst.toString(), edPass_->text())) {
		myApp->refreshWalletData();

		edUrl_->setText("");
		edUser_->setText("");
		edPass_->setText("");
		refreshAddForm("");
	} else {
		QMessageBox::critical(this, tr("Error"), tr("Failed to add password to wallet"), QMessageBox::Ok);
	}
}
