#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#pragma once 

#include <QtCore\QString>
#include <QtCore\QDebug>
#include <QtCore\QDir>
#include <QtCore\QThread>
#include <QtCore\QFuture>
#include <QtWidgets\QDialog>
#include <QtWidgets\QFormLayout>
#include <QtWidgets\QHBoxLayout>
#include <QtWidgets\QPlainTextEdit>
#include <QtWidgets\QPushButton>
#include <QtWidgets\QLabel>
#include <QtWidgets\QFileDialog>
#include <QtWidgets\QLineEdit>
#include <QtWidgets\QMessageBox>
#include <QtConcurrent\QtConcurrent>

#include <cassert>

#include "runextensions.h"
#include "../mc_common/WaitDialog.hpp"
#include "utils.h"
#include "Worker.h"

//====================================
// Class for main dialog
//
class MainDialog : public QDialog {
	Q_OBJECT

public:
	MainDialog(QWidget* parent = 0) : QDialog(parent) {
		setWindowTitle(tr("Create new protected PDF"));
		setFixedSize(400, 300);
		setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

		// Browse for PDF file
		m_pdfFileLabel = new QLineEdit(tr("Select PDF file"), this);
		m_pdfFileLabel->setEnabled(false);
		m_pdfFileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

		m_pdfBrowseButton = new QPushButton(tr("&Browse"), this);
		m_pdfBrowseButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		connect(m_pdfBrowseButton, SIGNAL(clicked()), SLOT(onBrowsePdfButton()));

		// Browse for output file
		m_outputFileLabel = new QLineEdit(tr("Select output file"), this);
		m_outputFileLabel->setEnabled(false);
		m_outputFileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

		m_outputBrowseButton = new QPushButton(tr("B&rowse"), this);
		m_outputBrowseButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		connect(m_outputBrowseButton, SIGNAL(clicked()), SLOT(onBrowseOutputButton()));

		// Creator link
		m_creatorLink = new QLineEdit(this);
		m_creatorLink->setMaxLength(m_maxLinkChars);
		m_creatorLinkLabel = new QLabel(this);
		m_creatorLinkLabel->setBuddy(m_creatorLink);
		connect(m_creatorLink, SIGNAL(textChanged(const QString&)), this, SLOT(onLinkTextChanged(const QString&)));
		
		// Watermark text
		m_watermarkText = new QLineEdit(this);
		m_watermarkText->setMaxLength(m_maxWatermarkChars);
		m_watermarkTextLabel = new QLabel(this);
		m_watermarkTextLabel->setBuddy(m_watermarkText);
		connect(m_watermarkText, SIGNAL(textChanged(const QString&)), this, SLOT(onWatermarkTextChanged(const QString&)));

		// Comments
		m_fileComments = new QPlainTextEdit(this);
		m_fileComments->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
		connect(m_fileComments, SIGNAL(textChanged()), this, SLOT(onCommentsTextChanged()));
		m_fileCommentsLabel = new QLabel(this);
		m_fileCommentsLabel->setBuddy(m_fileComments);

		// Buttons and links
		m_mailLinkLabel = new QLabel(this);
		m_mailLinkLabel->setText("<a href=\"mailto:alexg-nn@mail.ru\">alexg-nn@mail.ru</a>");
		m_mailLinkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
		m_mailLinkLabel->setOpenExternalLinks(true);
		m_mailLinkLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

		m_createButton = new QPushButton(tr("&Create"), this);
		m_createButton->setEnabled(false);
		m_createButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		connect(m_createButton, SIGNAL(released()), SLOT(onCreateButton()));

		// Layout
		m_pdfBrowseLayout = new QHBoxLayout();
		m_pdfBrowseLayout->addWidget(m_pdfFileLabel);
		m_pdfBrowseLayout->addWidget(m_pdfBrowseButton);
		m_pdfBrowseLayout->setSpacing(10);

		m_outputBrowseLayout = new QHBoxLayout();
		m_outputBrowseLayout->addWidget(m_outputFileLabel);
		m_outputBrowseLayout->addWidget(m_outputBrowseButton);
		m_outputBrowseLayout->setSpacing(10);

		m_extraElementsLayout = new QHBoxLayout();
		m_extraElementsLayout->addWidget(m_mailLinkLabel);
		m_extraElementsLayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
		m_extraElementsLayout->addWidget(m_createButton);

		m_layout = new QFormLayout(this);
		m_layout->addRow(m_pdfBrowseLayout);
		m_layout->addRow(m_outputBrowseLayout);
		m_layout->addRow(m_creatorLinkLabel, m_creatorLink);
		m_layout->addRow(m_watermarkTextLabel, m_watermarkText);
		m_layout->addRow(m_fileCommentsLabel, m_fileComments);
		m_layout->addItem(m_extraElementsLayout);
		m_layout->setRowWrapPolicy(QFormLayout::WrapAllRows);
		
		setLayout(m_layout);

		// Force counters to be updated
		emit m_creatorLink->textChanged(m_creatorLink->text());
		emit m_watermarkText->textChanged(m_watermarkText->text());
		emit m_fileComments->textChanged();
	}

	~MainDialog() {
		// iterate through list of children then call delete
		QObjectList children = this->children();
		for (auto it = children.begin(); it != children.end(); it++) {
			delete *it;
		}
	}

private slots:
	void onBrowsePdfButton() {
		QString selection = QFileDialog::getOpenFileName(this, tr("Select PDF file"), QDir::currentPath(), tr("PDF files (*.pdf)"));
		if (!selection.isEmpty()) {
			m_pdfFile = selection;
			QFontMetrics metrics(m_pdfFileLabel->font());
			QString elidedFilename = metrics.elidedText(m_pdfFile, Qt::ElideMiddle, m_pdfFileLabel->width());
			m_pdfFileLabel->setText(elidedFilename);
		}
		setButtonState();
	}

	void onBrowseOutputButton() {
		QString selection = QFileDialog::getSaveFileName(this, tr("Save protected PDF file as"), QDir::currentPath(), tr("Executable files (*.exe)"));
		if (!selection.isEmpty()) {
			m_outputFile = selection;
			QFontMetrics metrics(m_outputFileLabel->font());
			QString elidedDirname = metrics.elidedText(m_outputFile, Qt::ElideMiddle, m_outputFileLabel->width());
			m_outputFileLabel->setText(elidedDirname);
		}
		setButtonState();
	}

	void onLinkTextChanged(const QString &text) {
		m_creatorLinkLabel->setText(tr("&Link (%1):").arg(m_maxLinkChars - text.length()));
		setButtonState();
	}

	void onWatermarkTextChanged(const QString &text) {
		m_watermarkTextLabel->setText(tr("&Watermark (%1):").arg(m_maxWatermarkChars - text.length()));
		setButtonState();
	}

	void onCommentsTextChanged() {
		QString text = m_fileComments->toPlainText();
		m_fileCommentsLabel->setText(tr("&Notes (%1):").arg(m_maxCommentsChars - text.length()));
		if (text.length() > m_maxCommentsChars) {
			m_fileComments->setPlainText(text.left(m_maxCommentsChars));
			m_fileComments->moveCursor(QTextCursor::End);
		}
		setButtonState();
	}

	void onCreateButton() {
		// Check if PDF file is correct and not locked
		if (!isCorrectPDF(m_pdfFile)) {
			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Error"));
			msgBox.setText(tr("Selected PDF file is not correct or locked."));
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.exec();

			return;
		}
		
		// Prepare to run operation
		WaitDialog splash(tr("Operation in progress"), tr("Please wait"), this);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), &splash, SLOT(close()));
		connect(&watcher, SIGNAL(progressRangeChanged(int, int)), &splash, SLOT(progressRangeChanged(int, int)));
		connect(&watcher, SIGNAL(progressTextChanged(const QString &)), &splash, SLOT(progressTextChanged(const QString &)));
		connect(&watcher, SIGNAL(progressValueChanged(int)), &splash, SLOT(progressValueChanged(int)));

		Worker worker(m_pdfFile, m_outputFile, m_creatorLink->text().trimmed(), m_watermarkText->text().trimmed(), m_fileComments->toPlainText().trimmed());

		QFuture<void> future = QtConcurrent::run(&Worker::doWork, &worker);
		watcher.setFuture(future);
		splash.exec();

		try {
			future.waitForFinished();

			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Completed"));
			msgBox.setText(tr("The operation completed successfully."));
			msgBox.setIcon(QMessageBox::Information);
			msgBox.exec();

		}
		catch (RuntimeError &e) {
			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Error"));
			msgBox.setText(tr("An error has occurred: %1").arg(e.what()));
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.exec();
		}
		catch (QtConcurrent::UnhandledException &) {
			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Error"));
			msgBox.setText(tr("An unknown error has occurred."));
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.exec();
		}
	}

protected:
	void setButtonState() {
		m_createButton->setEnabled(!m_pdfFile.isEmpty() && !m_outputFile.isEmpty());
	}

	void keyPressEvent(QKeyEvent *e) {
		if (e->key() != Qt::Key_Escape) {
			QDialog::keyPressEvent(e);
		}
	}

private:
	QString m_pdfFile;
	QString m_outputFile;

	QLineEdit *m_pdfFileLabel;
	QLineEdit *m_outputFileLabel;
	QLineEdit *m_creatorLink;
	QLineEdit *m_watermarkText;

	QPlainTextEdit *m_fileComments;

	QPushButton *m_createButton;
	QPushButton *m_pdfBrowseButton;
	QPushButton *m_outputBrowseButton;

	QLabel *m_creatorLinkLabel;
	QLabel *m_watermarkTextLabel;
	QLabel *m_fileCommentsLabel;
	QLabel *m_mailLinkLabel;

	QHBoxLayout *m_pdfBrowseLayout;
	QHBoxLayout *m_outputBrowseLayout;
	QHBoxLayout *m_extraElementsLayout;
	QFormLayout *m_layout;
	
	const unsigned short m_maxWatermarkChars = 128;
	const unsigned short m_maxLinkChars = 128;
	const unsigned short m_maxCommentsChars = 256;
};

#endif // MAINDIALOG_H