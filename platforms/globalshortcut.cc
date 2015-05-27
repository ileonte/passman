#include "globalshortcut.h"
#if defined(Q_OS_LINUX)
#include "platforms/x11/globalshortcut_x11.h"
#endif

GlobalShortcut::GlobalShortcut(QObject *parent) : QObject(parent)
{
	impl_ = new GlobalShortcutImpl(this);
	connect(impl_, &GlobalShortcutImpl::activated, [this]() {
		emit activated();
	});
}

GlobalShortcut::~GlobalShortcut()
{
	setActive(false);
}

bool GlobalShortcut::active() const
{
	return impl_->active();
}

void GlobalShortcut::setActive(bool onoff)
{
	impl_->setActive(onoff);
}

QKeySequence GlobalShortcut::keySequence() const
{
	return impl_->keySequence();
}

void GlobalShortcut::setKeySequence(const QKeySequence &qks)
{
	impl_->setKeySequence(qks);
}

bool GlobalShortcut::triggeredWhenFocused() const
{
	return impl_->triggeredWhenFocused();
}

void GlobalShortcut::setTriggeredWhenFocused(bool yesno)
{
	impl_->setTriggeredWhenFocused(yesno);
}
