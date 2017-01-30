#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#pragma once 

#include <QtCore\QDebug>
#include <QtWidgets\QDialog>
#include <QtWidgets\QMessageBox>
#include <QtWidgets\QFormLayout>
#include <QtWidgets\QPlainTextEdit>
#include <QtWidgets\QPushButton>
#include <QtWidgets\QLabel>
#include <cassert>

#include "utils.h"
#include "FileParser.h"

//====================================
// Class for main dialog
//
class MainDialog : public QDialog {
	Q_OBJECT

public:
	MainDialog(FileParser *manager, QWidget* parent = 0) : m_manager(manager), QDialog(parent) {
		assert(manager != nullptr);

		setWindowTitle(tr("Unlock protected PDF"));
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		setFixedSize(500, 400);

		m_requestEdit = new QPlainTextEdit(this);
		m_requestEdit->setReadOnly(true);
		m_requestEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

		m_responseEdit = new QPlainTextEdit(this);
		m_responseEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		QObject::connect(m_responseEdit, SIGNAL(textChanged()), this, SLOT(onResponseChanged()));

		m_okButton = new QPushButton(tr("&OK"), this);
		m_okButton->setEnabled(false);
		QObject::connect(m_okButton, SIGNAL(released()), this, SLOT(onOkButton()));

		m_cancelButton = new QPushButton(tr("&Cancel"), this);
		m_cancelButton->setDefault(true);
		QObject::connect(m_cancelButton, SIGNAL(released()), this, SLOT(reject()));

		m_linkLabel = new QLabel();
		m_linkLabel->setTextFormat(Qt::RichText);
		m_linkLabel->setText(manager->getLinkText());
		m_linkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
		m_linkLabel->setOpenExternalLinks(true);

		m_hlayout = new QHBoxLayout();
		m_hlayout->addWidget(m_linkLabel);
		m_hlayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
		m_hlayout->addWidget(m_okButton); 
		m_hlayout->addWidget(m_cancelButton);

		m_layout = new QFormLayout(this);
		m_layout->addRow(tr("&Request data for the key:"), m_requestEdit);
		m_layout->addRow(tr("&Key to unlock:"), m_responseEdit);
		m_layout->setRowWrapPolicy(QFormLayout::WrapAllRows);
		m_layout->addItem(m_hlayout);

		setLayout(m_layout);
	}

	~MainDialog() {
		// iterate through list of children then call delete
		QObjectList children = this->children();
		for (auto it = children.begin(); it != children.end(); it++) {
			delete *it;
		}
	}

	void setRequestText(const QString &text) {
		m_requestEdit->setPlainText(text);
	}

	QString getResponseText() const {
		return m_responseEdit->toPlainText();
	}

private slots:
	void onResponseChanged() {
		m_okButton->setEnabled(!m_responseEdit->toPlainText().isEmpty());
	}

	void onOkButton() {
		try {
			m_manager->parseResponse(m_responseEdit->toPlainText());
			accept();
		}
		catch (...) {
			QMessageBox msgBox;
			msgBox.setWindowTitle(QObject::tr("Error"));
			msgBox.setText(QObject::tr("Key is not valid."));
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.exec();
		}
	}

private:
	QPlainTextEdit *m_requestEdit;
	QPlainTextEdit *m_responseEdit;
	QPushButton *m_okButton;
	QPushButton *m_cancelButton;
	FileParser *m_manager;
	QLabel *m_linkLabel;
	QHBoxLayout *m_hlayout;
	QFormLayout *m_layout;
};

#endif // MAINDIALOG_H