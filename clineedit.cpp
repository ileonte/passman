#include "clineedit.h"

CLineEdit::CLineEdit(QWidget *parent) : QLineEdit(parent), eatEscapes_(false)
{
}

void CLineEdit::keyPressEvent(QKeyEvent *ev)
{
	if (ev->key() == Qt::Key_Escape) {
		if (eatEscapes_) {
			emit escPressed();
			ev->accept();
		} else {
			QLineEdit::keyPressEvent(ev);
		}
	} else if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
		emit enterPressed();
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
