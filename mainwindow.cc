#include "mainwindow.h"

#include <QWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QIcon>
#include <QLabel>
#include <QDesktopWidget>
#include <QGroupBox>
#include <QFrame>
#include <QMessageBox>

#include "passmanapp.h"
#include "platforms/platformhelper.h"
#include "wallets/wallet_stringlist.h"

static inline void addHLine(QLayout *layout)
{
	auto frm = new QFrame;
	frm->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	layout->addWidget(frm);
}

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), trayIcon_(nullptr)
{
	setWindowIcon(QIcon::fromTheme("security-high"));
	setWindowTitle(tr("PassMan settings"));

	auto layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);

	auto header = new QWidget(this);
	{
		header->setStyleSheet("background-color: white");

		auto hl = new QHBoxLayout(header);
		auto lb = new QLabel(tr("Settings"));
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
		auto mainLayout = new QVBoxLayout(mainWidget);

		auto gb = new QGroupBox(tr("Wallet settings"));
		{
			auto hl = new QHBoxLayout(gb);

			cbBackends_ = new QComboBox(gb);
			cbWallets_ = new QComboBox(gb);

			auto lb = new QLabel(tr("Storage &backend:"));
			lb->setBuddy(cbBackends_);
			hl->addWidget(lb);
			hl->addWidget(cbBackends_);

			lb = new QLabel(tr("&Wallet:"));
			lb->setBuddy(cbWallets_);
			hl->addWidget(lb);
			hl->addWidget(cbWallets_);

			hl->addStretch();
		}
		mainLayout->addWidget(gb);

		gb = new QGroupBox(tr("Hotkey settings"));
		{
			auto gl = new QGridLayout(gb);

			kseSearch_   = new QKeySequenceEdit(gb);
			kseSettings_ = new QKeySequenceEdit(gb);
			kseQuit_     = new QKeySequenceEdit(gb);
			kseManage_  = new QKeySequenceEdit(gb);

			auto lb = new QLabel(tr("Global hotkey for &searching:"));
			lb->setBuddy(kseSearch_);
			gl->addWidget(lb, 0, 0);
			gl->addWidget(kseSearch_, 0, 1);

			lb = new QLabel(tr("Use a global hotkey for showing/hiding the se&ttings window:"));
			lb->setBuddy(kseSettings_);
			gl->addWidget(lb, 1, 0);
			gl->addWidget(kseSettings_, 1, 1);

			lb = new QLabel(tr("Use a global hotkey for q&uitting the application:"));
			lb->setBuddy(kseQuit_);
			gl->addWidget(lb, 2, 0);
			gl->addWidget(kseQuit_, 2, 1);

			lb = new QLabel(tr("Use a global hotkey for showing/hiding the &password manager:"));
			lb->setBuddy(kseManage_);
			gl->addWidget(lb, 3, 0);
			gl->addWidget(kseManage_, 3, 1);
		}
		mainLayout->addWidget(gb);
	}
	layout->addWidget(mainWidget);

	layout->addSpacing(30);
	layout->addStretch();

	addHLine(layout);

	auto w = new QWidget(this);
	{
		auto hl = new QHBoxLayout(w);

		btnAbout_ = new QPushButton(tr("&About"), w);
		btnQuit_  = new QPushButton(tr("&Quit"), w);
		btnManage_   = new QPushButton(tr("&Manage passwords"), w);
		btnClose_ = new QPushButton(tr("&Close"), w);

		hl->addWidget(btnAbout_);
		hl->addWidget(btnQuit_);
		hl->addStretch();
		hl->addWidget(btnManage_);
		hl->addWidget(btnClose_);
	}
	layout->addWidget(w);

	trayIcon_ = new QSystemTrayIcon(this);
	trayIcon_->setIcon(QIcon::fromTheme("security-high"));
	trayIcon_->show();

	connect(cbBackends_, SIGNAL(activated(int)), myApp, SLOT(setActiveBackend(int)));
	connect(myApp, &PassManApp::activeBackendChanged, cbBackends_, &QComboBox::setCurrentIndex);
	connect(myApp, &PassManApp::activeBackendChanged, this, &MainWindow::activeBackendChanged);

	connect(myApp, &PassManApp::walletNameChanged, this, &MainWindow::walletNameChanged);
	connect(cbWallets_, SIGNAL(activated(QString)), myApp, SLOT(setWalletName(QString)));

	connect(kseSearch_, &QKeySequenceEdit::keySequenceChanged, [this](const QKeySequence &ks) {
		setGlobalShortcut(myApp->shortcutSearch(), ks);
	});
	connect(kseQuit_, &QKeySequenceEdit::keySequenceChanged, [this](const QKeySequence &ks) {
		setGlobalShortcut(myApp->shortcutQuit(), ks);
	});
	connect(kseSettings_, &QKeySequenceEdit::keySequenceChanged, [this](const QKeySequence &ks) {
		setGlobalShortcut(myApp->shortcutSettings(), ks);
	});
	connect(kseManage_, &QKeySequenceEdit::keySequenceChanged, [this](const QKeySequence &ks) {
		setGlobalShortcut(myApp->shortcutManage(), ks);
	});

	connect(btnClose_, &QPushButton::clicked, this, &MainWindow::close);
	connect(btnQuit_, &QPushButton::clicked, myApp, &PassManApp::quit);
	connect(btnManage_, &QPushButton::clicked, myApp, &PassManApp::showPasswordManager);
	connect(btnAbout_, &QPushButton::clicked, [this]() {
		QMessageBox::about(this, tr("PassMan"), tr("PassMan Password Manager v0.1.1"));
	});

	connect(trayIcon_, &QSystemTrayIcon::activated, this, &MainWindow::systrayActivated);
}

void MainWindow::populateWidgets()
{
	cbBackends_->addItems(myApp->backendNames());
	cbBackends_->setCurrentIndex(myApp->activeBackend());

	getGlobalShortcut(myApp->shortcutSearch(), kseSearch_);
	getGlobalShortcut(myApp->shortcutQuit(), kseQuit_);
	getGlobalShortcut(myApp->shortcutSettings(), kseSettings_);
	getGlobalShortcut(myApp->shortcutManage(), kseManage_);
}

void MainWindow::activeBackendChanged()
{
	walletNames_ = myApp->walletNames();

	cbWallets_->clear();
	cbWallets_->addItems(walletNames_);
}

void MainWindow::walletNameChanged(const QString &name)
{
	cbWallets_->setCurrentIndex(walletNames_.indexOf(name));
}

MainWindow::~MainWindow()
{
}

void MainWindow::show()
{
	QScreen *s = platformHelper()->getActiveWindowScreen();
	QWidget::show();

	if (s) move(s->geometry().center() - rect().center());
	platformHelper()->popUp(this);
	cbBackends_->setFocus();
}

void MainWindow::toggle()
{
	if (isVisible()) {
		cbBackends_->setFocus();
		hide();
	} else {
		show();
	}
}

void MainWindow::systrayActivated(QSystemTrayIcon::ActivationReason reason)
{
	Q_UNUSED(reason);
	toggle();
}

void MainWindow::setGlobalShortcut(GlobalShortcut *shortcut, const QKeySequence &ks)
{
	if (ks.count() == 1) {
		shortcut->setKeySequence(ks);
		shortcut->setActive(true);
	} else {
		shortcut->setActive(false);
	}
}

void MainWindow::getGlobalShortcut(GlobalShortcut *shortcut, QKeySequenceEdit *edit)
{
	if (shortcut->active())
		edit->setKeySequence(shortcut->keySequence());
	else
		edit->setKeySequence(QKeySequence());
}
