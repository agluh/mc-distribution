#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#pragma once 

#include <QtCore\QDebug>
#include <QtWidgets\QDialog>
#include <QtWidgets\QFormLayout>
#include <QtWidgets\QHBoxLayout>
#include <QtWidgets\QVBoxLayout>
#include <QtWidgets\QPlainTextEdit>
#include <QtWidgets\QPushButton>
#include <QtWidgets\QLabel>
#include <QtWidgets\QFileDialog>
#include <QtWidgets\QLineEdit>

#include "utils.h"

//====================================
// Class for main dialog
//
class MainDialog : public QDialog {
	Q_OBJECT

public:
	MainDialog(QWidget* parent = 0) : QDialog(parent) {
		setWindowTitle(tr("Generate key for protected PDF"));
		setFixedSize(700, 400);
		setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

		// Edits
		m_infoText = new QPlainTextEdit(this);
		m_infoText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
		m_infoText->setFixedWidth(200);
		m_infoText->setReadOnly(true);
		
		QPalette textEditPaletteNormal = m_infoText->palette();
		QPalette textEditPaletteReadOnly = m_infoText->palette();
		textEditPaletteReadOnly.setColor(QPalette::Active, QPalette::Base, textEditPaletteNormal.color(QPalette::Disabled, QPalette::Base));
		textEditPaletteReadOnly.setColor(QPalette::Inactive, QPalette::Base, textEditPaletteNormal.color(QPalette::Disabled, QPalette::Base));
		m_infoText->setPalette(textEditPaletteReadOnly);

		m_infoTextLabel = new QLabel(tr("&Information about the client:"), this);
		m_infoTextLabel->setBuddy(m_infoText);

		m_requestText = new QPlainTextEdit(this);
		m_requestText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		m_requestText->setPlaceholderText(tr("Put here data recieved from the client"));
		connect(m_requestText, SIGNAL(textChanged()), this, SLOT(onRequestTextChanged()));

		m_requestTextLabel = new QLabel(tr("&Data from the client:"), this);
		m_requestTextLabel->setBuddy(m_requestText);

		m_responseText = new QPlainTextEdit(this);
		m_responseText->setReadOnly(true);
		m_responseText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		m_responseText->setFixedHeight(100);

		m_responseTextLabel = new QLabel(tr("&Generated key for the client:"), this);
		m_responseTextLabel->setBuddy(m_responseText);

		// Link
		m_mailLinkLabel = new QLabel(this);
		m_mailLinkLabel->setText("<a href=\"mailto:alexg-nn@mail.ru\">alexg-nn@mail.ru</a>");
		m_mailLinkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
		m_mailLinkLabel->setOpenExternalLinks(true);
		m_mailLinkLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

		// Layout
		m_requestVertLayout = new QVBoxLayout();
		m_requestVertLayout->addWidget(m_requestTextLabel);
		m_requestVertLayout->addWidget(m_requestText);
		m_requestVertLayout->setSpacing(5);

		m_responseVertLayout = new QVBoxLayout();
		m_responseVertLayout->addWidget(m_responseTextLabel);
		m_responseVertLayout->addWidget(m_responseText);
		m_responseVertLayout->setSpacing(5);

		m_editsVertlayout = new QVBoxLayout();
		m_editsVertlayout->addItem(m_requestVertLayout);
		m_editsVertlayout->addItem(m_responseVertLayout);
		m_editsVertlayout->setSpacing(10);

		m_infoVertLayout = new QVBoxLayout();
		m_infoVertLayout->addWidget(m_infoTextLabel);
		m_infoVertLayout->addWidget(m_infoText);
		m_infoVertLayout->setSpacing(5);

		m_mainHorLayout = new QHBoxLayout();
		m_mainHorLayout->addItem(m_editsVertlayout);
		m_mainHorLayout->addItem(m_infoVertLayout);
		m_mainHorLayout->setSpacing(10);

		m_extraElementsLayout = new QHBoxLayout();
		m_extraElementsLayout->addWidget(m_mailLinkLabel);
		m_extraElementsLayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));

		m_layout = new QVBoxLayout(this);
		m_layout->addItem(m_mainHorLayout);
		m_layout->addItem(m_extraElementsLayout);

		setLayout(m_layout);
	}

	~MainDialog() {
		// iterate through list of children then call delete
		QObjectList children = this->children();
		for (auto it = children.begin(); it != children.end(); it++) {
			delete *it;
		}
	}

private slots:
	void onRequestTextChanged() {
		QString text = m_requestText->toPlainText();
		if (text.isEmpty()) {
			m_infoText->clear();
			m_responseText->clear();
			return;
		}

		try {
			QByteArray data = QByteArray::fromBase64(text.toLatin1());
			QString watermark, comments, link;
			QByteArray iv, key, hid;
			parseRequest(data, &watermark, &comments, &link, &iv, &key, &hid);
			
			QString info = QString(tr("Link: %1<br/>Watermark: %2<br/>Notes:<br/>%3<br/><br/>---- Debug data ----<br/><br/>Hardware ID: %4<br/>AES IV: %5<br/>AES key: %6"))
				.arg(link)
				.arg(watermark)
				.arg(comments)
				.arg(QString(hid.toHex()))
				.arg(QString(iv.toHex()))
				.arg(QString(key.toHex()));

			info = info.replace("<br/>", "\r\n");
			
			m_infoText->setPlainText(info);

			QString response = generateResponse(hid, key);
			m_responseText->setPlainText(response);
		}
		catch (...) {
			m_infoText->setPlainText(tr("Entered data is invalid"));
			m_responseText->clear();
		}
	}

private:
	QPlainTextEdit *m_infoText;
	QPlainTextEdit *m_requestText;
	QPlainTextEdit *m_responseText;

	QLabel *m_infoTextLabel;
	QLabel *m_requestTextLabel;
	QLabel *m_responseTextLabel;
	QLabel *m_mailLinkLabel;

	QVBoxLayout *m_requestVertLayout;
	QVBoxLayout *m_responseVertLayout;
	QVBoxLayout *m_editsVertlayout;
	QVBoxLayout *m_infoVertLayout;
	QHBoxLayout *m_mainHorLayout;
	QHBoxLayout *m_extraElementsLayout;
	QVBoxLayout *m_layout;
};

#endif // MAINDIALOG_H