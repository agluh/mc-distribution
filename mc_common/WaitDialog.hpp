#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#pragma once 

#include <QtCore\QString>
#include <QtCore\QDebug>
#include <QtWidgets\QDialog>
#include <QtWidgets\QLabel>
#include <QtWidgets\QProgressBar>
#include <QtWidgets\QVBoxLayout>


//====================================
// Simple class to show wating messages
//
class WaitDialog : public QDialog {
	Q_OBJECT

public:
	WaitDialog(const QString &msg, const QString &title, QWidget* parent = 0) : QDialog(parent) {
		setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
		setWindowTitle(title);
		
		m_label = new QLabel(msg, this);
		m_label->adjustSize();

		m_progressbar = new QProgressBar(this);

		QVBoxLayout *layout = new QVBoxLayout;
		layout->addWidget(m_label);
		layout->addWidget(m_progressbar);
		layout->setSizeConstraint(QLayout::SetFixedSize);

		setLayout(layout);
	}

private Q_SLOTS:
	void progressRangeChanged(int min, int max) {
		m_progressbar->setRange(min, max);
	}

	void progressTextChanged(const QString &text) {
		m_label->setText(text);
	}

	void progressValueChanged(int value) {
		m_progressbar->setValue(value);
	}

private:

	QLabel *m_label;
	QProgressBar* m_progressbar;
};

#endif // WAITDIALOG_H