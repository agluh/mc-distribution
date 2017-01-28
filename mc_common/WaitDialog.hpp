#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#pragma once 

#include <QtCore\QString>
#include <QtWidgets\QDialog>
#include <QtWidgets\QLabel>
#include <QtWidgets\QHBoxLayout>


//====================================
// Simple class to show wating messages
//
class WaitDialog : public QDialog {
	Q_OBJECT

public:
	WaitDialog(const QString &msg, const QString &title, QWidget* parent = 0) : QDialog(parent) {
		setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
		setWindowTitle(title);
		QLabel *central = new QLabel(msg);
		central->adjustSize();
		QHBoxLayout *layout = new QHBoxLayout;
		layout->addWidget(central);

		layout->setSizeConstraint(QLayout::SetFixedSize);
		setLayout(layout);
	}
};

#endif // WAITDIALOG_H