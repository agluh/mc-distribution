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

		m_requestEdit = new QPlainTextEdit();
		m_requestEdit->setReadOnly(true);
		m_requestEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

		m_responseEdit = new QPlainTextEdit();
		m_responseEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		QObject::connect(m_responseEdit, SIGNAL(textChanged()), this, SLOT(onResponseChanged()));

		m_okButton = new QPushButton(tr("&OK"));
		m_okButton->setEnabled(false);
		QObject::connect(m_okButton, SIGNAL(released()), this, SLOT(onOkButton()));

		m_cancelButton = new QPushButton(tr("&Cancel"));
		m_cancelButton->setDefault(true);
		QObject::connect(m_cancelButton, SIGNAL(released()), this, SLOT(reject()));

		QLabel *siteLink = new QLabel;
		siteLink->setTextFormat(Qt::RichText);
		siteLink->setText(manager->getLinkText());
		siteLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
		siteLink->setOpenExternalLinks(true);

		QHBoxLayout *hlayout = new QHBoxLayout;
		hlayout->addWidget(siteLink);
		hlayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
		hlayout->addWidget(m_okButton); 
		hlayout->addWidget(m_cancelButton);

		QFormLayout *formLayout = new QFormLayout;
		formLayout->addRow(tr("&Request data for the key:"), m_requestEdit);
		formLayout->addRow(tr("&Key to unlock:"), m_responseEdit);
		formLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
		formLayout->addItem(hlayout);

		setLayout(formLayout);
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
};

#endif // MAINDIALOG_H