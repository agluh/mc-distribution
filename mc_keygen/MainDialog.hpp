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
		m_infoText = new QPlainTextEdit;
		m_infoText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
		m_infoText->setFixedWidth(200);
		m_infoText->setReadOnly(true);
		
		QPalette textEditPaletteNormal = m_infoText->palette();
		QPalette textEditPaletteReadOnly = m_infoText->palette();
		textEditPaletteReadOnly.setColor(QPalette::Active, QPalette::Base, textEditPaletteNormal.color(QPalette::Disabled, QPalette::Base));
		textEditPaletteReadOnly.setColor(QPalette::Inactive, QPalette::Base, textEditPaletteNormal.color(QPalette::Disabled, QPalette::Base));
		m_infoText->setPalette(textEditPaletteReadOnly);

		QLabel *infoTextLabel = new QLabel(tr("&Information about the client:"));
		infoTextLabel->setBuddy(m_infoText);

		m_requestText = new QPlainTextEdit;
		m_requestText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		m_requestText->setPlaceholderText(tr("Put here data recieved from the client"));
		connect(m_requestText, SIGNAL(textChanged()), this, SLOT(onRequestTextChanged()));

		QLabel *requestTextLabel = new QLabel(tr("&Data from the client:"));
		requestTextLabel->setBuddy(m_requestText);

		m_responseText = new QPlainTextEdit;
		m_responseText->setReadOnly(true);
		m_responseText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		m_responseText->setFixedHeight(100);

		QLabel *responseTextLabel = new QLabel(tr("&Generated key for the client:"));
		responseTextLabel->setBuddy(m_responseText);

		// Link
		QLabel *mailLink = new QLabel;
		mailLink->setText("<a href=\"mailto:alexg-nn@mail.ru\">alexg-nn@mail.ru</a>");
		mailLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
		mailLink->setOpenExternalLinks(true);
		mailLink->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

		// Layout
		QVBoxLayout *requestVertLayout = new QVBoxLayout();
		requestVertLayout->addWidget(requestTextLabel);
		requestVertLayout->addWidget(m_requestText);
		requestVertLayout->setSpacing(5);

		QVBoxLayout *responseVertLayout = new QVBoxLayout();
		responseVertLayout->addWidget(responseTextLabel);
		responseVertLayout->addWidget(m_responseText);
		responseVertLayout->setSpacing(5);

		QVBoxLayout *editsVertlayout = new QVBoxLayout();
		editsVertlayout->addItem(requestVertLayout);
		editsVertlayout->addItem(responseVertLayout);
		editsVertlayout->setSpacing(10);

		QVBoxLayout *infoVertLayout = new QVBoxLayout();
		infoVertLayout->addWidget(infoTextLabel);
		infoVertLayout->addWidget(m_infoText);
		infoVertLayout->setSpacing(5);

		QHBoxLayout *mainHorLayout = new QHBoxLayout();
		mainHorLayout->addItem(editsVertlayout);
		mainHorLayout->addItem(infoVertLayout);
		mainHorLayout->setSpacing(10);

		QHBoxLayout *extraElementsLayout = new QHBoxLayout();
		extraElementsLayout->addWidget(mailLink);
		extraElementsLayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));

		QVBoxLayout *layout = new QVBoxLayout();
		layout->addItem(mainHorLayout);
		layout->addItem(extraElementsLayout);

		setLayout(layout);
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
};

#endif // MAINDIALOG_H