#include "clineedit.h"

CLineEdit::CLineEdit(QWidget *parent) : QLineEdit(parent), eatEscapes_(false),
	enterPressed_(false), enterMods_(Qt::NoModifier), escPressed_(false), count_(0)
{
}

void CLineEdit::keyPressEvent(QKeyEvent *ev)
{
	count_ += 1;

	if (ev->key() == Qt::Key_Escape) {
		if (eatEscapes_) {
			escPressed_ = true;
			ev->accept();
		} else {
			QLineEdit::keyPressEvent(ev);
		}
	} else if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
		enterPressed_ = true;
		enterMods_    = ev->modifiers();
		ev->accept();
	} else if (ev->key() == Qt::Key_Up) {
		emit up();
		ev->accept();
	} else if (ev->key() == Qt::Key_Down) {
		emit down();
		ev->accept();
	} else if (ev->key() == Qt::Key_PageUp) {
		emit pageUp();
		ev->accept();
	} else if (ev->key() == Qt::Key_PageDown) {
		emit pageDown();
		ev->accept();
	} else {
		QLineEdit::keyPressEvent(ev);
	}
}

void CLineEdit::keyReleaseEvent(QKeyEvent *ev)
{
	if (count_ > 0)
		count_ -= 1;

	if (!count_) {
		if (escPressed_) {
			emit escPressed();
			ev->accept();
		} else if (enterPressed_) {
			emit enterPressed(enterMods_);
			enterPressed_ = false;
			enterMods_    = Qt::NoModifier;
			ev->accept();
		} else {
			QLineEdit::keyReleaseEvent(ev);
		}
	} else {
		QLineEdit::keyReleaseEvent(ev);
	}
}

void CLineEdit::focusOutEvent(QFocusEvent *ev)
{
	QLineEdit::focusOutEvent(ev);
	if (ev->lostFocus())
		emit focusLost();
}

bool CLineEdit::eatEscapes() const
{
	return eatEscapes_;
}

void CLineEdit::setEatEscapes(bool yesno)
{
	eatEscapes_ = yesno;
}
