#include <QListView>
#include <QVBoxLayout>
#include <QClipboard>
#include <QWindow>
#include <QScreen>
#include <QDebug>
#include <QItemSelectionModel>

#include "passmanapp.h"
#include "clineedit.h"
#include "searchwidget.h"
#include "platforms/platformhelper.h"

SearchWidget::SearchWidget(QWidget *parent) : QFrame(parent)
{
	setWindowFlags(Qt::Window|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	setMinimumWidth(500);
	setFrameShape(QFrame::Panel);
	setFrameShadow(QFrame::Raised);

	dataFilter_ = new PasswordFilterModel(myApp->dataModel());

	auto layout = new QVBoxLayout(this);

	searchField_ = new CLineEdit(this);
	searchField_->setEatEscapes(true);

	searchResults_ = new QTreeView(this);
	searchResults_->setSelectionBehavior(QTreeView::SelectRows);
	searchResults_->setSelectionMode(QTreeView::SingleSelection);

	connect(searchField_, &CLineEdit::escPressed, this, &SearchWidget::toggle);
	connect(searchField_, &CLineEdit::enterPressed, this, &SearchWidget::processPassword);
	connect(searchField_, &CLineEdit::textChanged, dataFilter_, &PasswordFilterModel::setFilters);
	connect(searchField_, &CLineEdit::up, this, &SearchWidget::goUp);
	connect(searchField_, &CLineEdit::down, this,&SearchWidget::goDown);
	connect(searchField_, &CLineEdit::focusLost, this, &SearchWidget::hide);

	layout->addWidget(searchField_);
	layout->addWidget(searchResults_);

	setFocusProxy(searchField_);
	searchResults_->setFocusProxy(searchField_);

	searchResults_->setModel(dataFilter_);
}

void SearchWidget::show()
{
	QScreen *s = platformHelper()->getActiveWindowScreen();

	searchField_->setText("");
	dataFilter_->setFilters(QString());

	QWidget::show();
	if (s) move(s->geometry().center().x() - rect().center().x(), 0);
	platformHelper()->removeTaskbarIcon(this);
	platformHelper()->popUp(this);
	searchField_->setFocus();
}

void SearchWidget::toggle()
{
	if (isVisible()) hide();
	else show();
}

void SearchWidget::processPassword(Qt::KeyboardModifiers modifiers)
{
	QModelIndexList selected = searchResults_->selectionModel()->selectedIndexes();
	if (selected.size()) {
		QString key = searchResults_->model()->data(selected[0], PasswordModel::WalletKeyName).toString();
		if (modifiers == Qt::NoModifier) {
			QString pass;
			if (myApp->walletGetPassword(key, pass))
				myApp->clipboard()->setText(pass);
			pass.fill(0);
		} else {
			myApp->delayedSendPasswordToActiveWindow(key);
		}

		searchResults_->selectionModel()->clear();

		hide();
	}
}

void SearchWidget::goDown()
{
	QModelIndexList selected = searchResults_->selectionModel()->selectedIndexes();
	if (selected.size()) {
		QModelIndex idx = myApp->dataModel()->index(selected[0].row() + 1, 0);
		if (idx.isValid())
			searchResults_->selectionModel()->select(idx, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
	} else {
		searchResults_->selectionModel()->select(myApp->dataModel()->index(0, 0), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
	}
}

void SearchWidget::goUp()
{
	QModelIndexList selected = searchResults_->selectionModel()->selectedIndexes();
	if (selected.size()) {
		QModelIndex idx = myApp->dataModel()->index(selected[0].row() - 1, 0);
		if (idx.isValid())
			searchResults_->selectionModel()->select(idx, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
	}
}
