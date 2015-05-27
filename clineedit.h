#ifndef CLINEEDIT_H
#define CLINEEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>

class CLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	explicit CLineEdit(QWidget *parent = nullptr);

	bool eatEscapes() const;

signals:
	void escPressed();
	void enterPressed();
	void up();
	void down();
	void pageUp();
	void pageDown();
	void focusLost();

public slots:
	void setEatEscapes(bool yesno);

protected:
	virtual void keyPressEvent(QKeyEvent *ev);
	virtual void focusOutEvent(QFocusEvent *ev);

private:
	bool eatEscapes_;
};

#endif // CLINEEDIT_H
