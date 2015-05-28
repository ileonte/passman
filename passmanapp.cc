#include "passmanapp.h"

#include <QMessageBox>
#include <QSettings>
#include <QIcon>

#include "wallets/wallet_stringlist.h"

PassManApp::PassManApp(int &argc, char **argv) : QApplication(argc, argv),
      appLock_(QDir::tempPath() + "/passman.lock"), activeBackend_(0),
      wallet_(nullptr)
{
	QApplication::setWindowIcon(QIcon::fromTheme("security-high"));

	passwordData_ = new PasswordModel(this);

	mainWindow_ = new MainWindow();
	searchWidget_ = new SearchWidget();
	passwordManager_ = new PasswordManager();

	gsSearch_   = new GlobalShortcut(this);
	gsQuit_     = new GlobalShortcut(this);
	gsSettings_ = new GlobalShortcut(this);
	gsManage_   = new GlobalShortcut(this);

	gsSearch_->setTriggeredWhenFocused(true);
	gsQuit_->setTriggeredWhenFocused(true);
	gsSettings_->setTriggeredWhenFocused(true);
	gsManage_->setTriggeredWhenFocused(true);

	connect(gsSearch_, &GlobalShortcut::activated, searchWidget_, &SearchWidget::toggle);
	connect(gsQuit_, &GlobalShortcut::activated, this, &PassManApp::quit);
	connect(gsSettings_, &GlobalShortcut::activated, mainWindow_, &MainWindow::toggle);
	connect(gsManage_, &GlobalShortcut::activated, this, &PassManApp::showPasswordManager);
}

PassManApp::~PassManApp()
{
	QSettings setts;
	auto intf = activeBackend_ > 0 ? backends_[activeBackend_] : nullptr;

	setts.setValue("walletBackend", intf ? intf->intfPluginUnixName : "");
	setts.setValue("walletName", walletName_);
	setts.setValue("shortcutSearch", gsSearch_->active() ? gsSearch_->keySequence().toString() : QString());
	setts.setValue("shortcutQuit", gsQuit_->active() ? gsQuit_->keySequence().toString() : QString());
	setts.setValue("shortcutSettings", gsSettings_->active() ? gsSettings_->keySequence().toString() : QString());
	setts.setValue("shortcutAddPass", gsManage_->active() ? gsManage_->keySequence().toString() : QString());

	if (intf && wallet_)
		intf->closeWallet(wallet_);
}

bool PassManApp::init()
{
	QSettings setts;
	QString savedBackend(setts.value("walletBackend").toString());
	QString savedWallet(setts.value("walletName").toString());

	appLock_.setStaleLockTime(0);
	if (!appLock_.tryLock()) {
		auto mb = new QMessageBox(QMessageBox::Critical,
		                          tr("Already running!"),
		                          tr("An instance of this application is already running."),
		                          QMessageBox::Ok);
		mb->show();
		return true;
	}

	setQuitOnLastWindowClosed(false);

	loadBackends();
	for (int i = 1; i < backends_.size(); i++) {
		if (savedBackend == backends_[i]->intfPluginUnixName) {
			setActiveBackend(i);
			break;
		}
	}

	if (activeBackend_ > 0)
		setWalletName(savedWallet);

	QString qss = setts.value("shortcutSearch", QString()).toString();
	if (!qss.isEmpty()) {
		QKeySequence qs(qss);
		if (!qs.isEmpty()) {
			gsSearch_->setKeySequence(qs);
			gsSearch_->setActive(true);
		}
	}

	qss = setts.value("shortcutQuit", QString()).toString();
	if (!qss.isEmpty()) {
		QKeySequence qs(qss);
		if (!qs.isEmpty()) {
			gsQuit_->setKeySequence(qs);
			gsQuit_->setActive(true);
		}
	}

	qss = setts.value("shortcutSettings", QString()).toString();
	if (!qss.isEmpty()) {
		QKeySequence qs(qss);
		if (!qs.isEmpty()) {
			gsSettings_->setKeySequence(qs);
			gsSettings_->setActive(true);
		}
	}

	qss = setts.value("shortcutAddPass", QString()).toString();
	if (!qss.isEmpty()) {
		QKeySequence qs(qss);
		if (!qs.isEmpty()) {
			gsManage_->setKeySequence(qs);
			gsManage_->setActive(true);
		}
	}

	mainWindow_->populateWidgets();

	return true;
}

QStringList PassManApp::backendNames() const
{
	QStringList ret;
	foreach (WalletBackend::LibraryInterface *intf, backends_) {
		ret.append(intf ? intf->intfPluginName : "");
	}
	return ret;
}

QStringList PassManApp::backendUnixNames() const
{
	QStringList ret;
	foreach (WalletBackend::LibraryInterface *intf, backends_) {
		ret.append(intf ? intf->intfPluginUnixName : "");
	}
	return ret;
}

int PassManApp::backendCount() const
{
	return backends_.size();
}

int PassManApp::activeBackend() const
{
	return activeBackend_;
}

void PassManApp::setActiveBackend(int idx)
{
	auto cintf = activeBackend_ > 0 ? backends_[activeBackend_] : nullptr;
	auto intf = (idx > 0 && idx < backends_.size()) ? backends_[idx] : nullptr;

	if (cintf && wallet_) {
		cintf->closeWallet(wallet_);
		wallet_ = nullptr;
	}

	if (intf) {
		walletNames_ = WalletBackend::WSLtoQSL(intf->listWallets());
		walletNames_.insert(0, "");

		activeBackend_ = idx;
		walletName_ = "";
	} else {
		walletNames_.empty();
		activeBackend_ = 0;
		walletName_ = "";
		wallet_ = nullptr;
	}

	emit activeBackendChanged(activeBackend_);
	emit walletNameChanged(walletName_);
}

QString PassManApp::walletName() const
{
	return walletName_;
}

void PassManApp::setWalletName(const QString &name)
{
	auto cintf = activeBackend_ > 0 ? backends_[activeBackend_] : nullptr;

	qDebug() << "OPENENING WALLET" << name;

	if (cintf) {
		QStringList wallets = WalletBackend::WSLtoQSL(cintf->listWallets());
		if (wallets.indexOf(name) < 0) {
			if (wallet_) cintf->closeWallet(wallet_);
			walletName_ = "";
			wallet_ = nullptr;
		} else {
			auto w = cintf->openWallet(qPrintable(myApp->applicationName()),
			                           qPrintable(name),
			                           0);
			if (w) {
				if (!cintf->folderExists(w, "PassMan"))
					cintf->folderCreate(w, "PassMan");

				if (cintf->folderSet(w, "PassMan")) {
					if (wallet_) cintf->closeWallet(wallet_);
					wallet_ = w;
					walletName_ = name;
				} else {
					cintf->closeWallet(w);
				}
			}
		}
	} else {
		walletName_ = "";
		wallet_ = nullptr;
	}

	refreshWalletData();
	emit walletNameChanged(walletName_);
}

QStringList PassManApp::walletNames() const
{
	return walletNames_;
}

QStringList PassManApp::walletPasswordEntries() const
{
	QStringList ret;
	auto cintf = activeBackend_ > 0 ? backends_[activeBackend_] : nullptr;

	if (cintf && wallet_) {
		QStringList r = WalletBackend::WSLtoQSL(cintf->entryList(wallet_));
		foreach (auto kn, r) {
			if (cintf->entryType(wallet_, kn.toUtf8().data()) == WalletBackend::WALLET_ENTRY_PASSWORD)
				ret.append(kn);
		}
	}

	return ret;
}

bool PassManApp::walletGetPassword(const QString &key, QString &password) const
{
	auto cintf = activeBackend_ > 0 ? backends_[activeBackend_] : nullptr;

	if (cintf && wallet_) {
		const char *szp = cintf->passRead(wallet_, key.toUtf8().data());
		if (!szp) return false;

		password = szp;
		return true;
	}

	return false;
}

bool PassManApp::walletSetPassword(const QString &key, const QString &password) const
{
	auto cintf = activeBackend_ > 0 ? backends_[activeBackend_] : nullptr;

	if (cintf && wallet_)
		return cintf->passWrite(wallet_, key.toUtf8().data(), password.toUtf8().data()) ? false : true;

	return false;
}

void PassManApp::walletDeletePasswords(const QStringList &keys)
{
	if (keys.size()) {
		auto cintf = activeBackend_ > 0 ? backends_[activeBackend_] : nullptr;

		if (cintf && wallet_) {
			foreach (auto key, keys) {
				cintf->entryDelete(wallet_, key.toUtf8().data());
			}
		}

		passwordData_->setPasswordData(walletPasswordEntries());
	}
}

void PassManApp::loadBackends()
{
#if defined(WALLET_PLUGIN_DIR)
	QDir pluginDir(WALLET_PLUGIN_DIR);
#else
	QDir pluginDir(myApp->applicationDirPath() + "/wallets");
#endif
	WalletBackend::LibraryInterface wintf;
	WalletBackend::LibraryInterface *aintf;

	qDebug() << pluginDir;

	backends_.append(nullptr);

	foreach (QString fname, pluginDir.entryList()) {
		if (!QLibrary::isLibrary(fname))
			continue;

		if (WalletBackend::loadLibrary(pluginDir.absoluteFilePath(fname).toUtf8().data(), &wintf) < 0)
			continue;

		aintf = new WalletBackend::LibraryInterface;
		*aintf = wintf;
		backends_.append(aintf);
	}

	qDebug() << backendNames();
}

GlobalShortcut *PassManApp::shortcutSearch() const
{
	return gsSearch_;
}

GlobalShortcut *PassManApp::shortcutQuit() const
{
	return gsQuit_;
}

GlobalShortcut *PassManApp::shortcutSettings() const
{
	return gsSettings_;
}

GlobalShortcut *PassManApp::shortcutManage() const
{
	return gsManage_;
}

PasswordModel *PassManApp::dataModel() const
{
	return passwordData_;
}

void PassManApp::showPasswordManager()
{
	passwordManager_->show();
}

void PassManApp::refreshWalletData()
{
	passwordData_->setPasswordData(walletPasswordEntries());
}
