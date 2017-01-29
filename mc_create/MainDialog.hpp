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

#include "../mc_common/WaitDialog.hpp"
#include "utils.h"

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
		m_pdfFileLabel = new QLineEdit(tr("Select PDF file"));
		m_pdfFileLabel->setEnabled(false);
		m_pdfFileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

		QPushButton *pdfBrowseButton = new QPushButton(tr("&Browse"));
		pdfBrowseButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		connect(pdfBrowseButton, SIGNAL(clicked()), SLOT(onBrowsePdfButton()));

		// Browse for output file
		m_outputFileLabel = new QLineEdit(tr("Select output file"));
		m_outputFileLabel->setEnabled(false);
		m_outputFileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

		QPushButton *outputBrowseButton = new QPushButton(tr("B&rowse"));
		outputBrowseButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		connect(outputBrowseButton, SIGNAL(clicked()), SLOT(onBrowseOutputButton()));

		// Creator link
		m_creatorLink = new QLineEdit;
		m_creatorLink->setMaxLength(m_maxLinkChars);
		m_creatorLinkLabel = new QLabel;
		m_creatorLinkLabel->setBuddy(m_creatorLink);
		connect(m_creatorLink, SIGNAL(textChanged(const QString&)), this, SLOT(onLinkTextChanged(const QString&)));
		
		// Watermark text
		m_watermarkText = new QLineEdit;
		m_watermarkText->setMaxLength(m_maxWatermarkChars);
		m_watermarkTextLabel = new QLabel;
		m_watermarkTextLabel->setBuddy(m_watermarkText);
		connect(m_watermarkText, SIGNAL(textChanged(const QString&)), this, SLOT(onWatermarkTextChanged(const QString&)));

		// Comments
		m_fileComments = new QPlainTextEdit;
		m_fileComments->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
		connect(m_fileComments, SIGNAL(textChanged()), this, SLOT(onCommentsTextChanged()));
		m_fileCommentsLabel = new QLabel;
		m_fileCommentsLabel->setBuddy(m_fileComments);

		// Buttons and links
		QLabel *mailLink = new QLabel;
		mailLink->setText("<a href=\"mailto:alexg-nn@mail.ru\">alexg-nn@mail.ru</a>");
		mailLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
		mailLink->setOpenExternalLinks(true);
		mailLink->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

		m_createButton = new QPushButton(tr("&Create"));
		m_createButton->setEnabled(false);
		m_createButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		connect(m_createButton, SIGNAL(released()), SLOT(onCreateButton()));

		// Layout
		QHBoxLayout *pdfBrowseLayout = new QHBoxLayout();
		pdfBrowseLayout->addWidget(m_pdfFileLabel);
		pdfBrowseLayout->addWidget(pdfBrowseButton);
		pdfBrowseLayout->setSpacing(10);

		QHBoxLayout *outputBrowseLayout = new QHBoxLayout();
		outputBrowseLayout->addWidget(m_outputFileLabel);
		outputBrowseLayout->addWidget(outputBrowseButton);
		outputBrowseLayout->setSpacing(10);

		QHBoxLayout *extraElementsLayout = new QHBoxLayout();
		extraElementsLayout->addWidget(mailLink);
		extraElementsLayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
		extraElementsLayout->addWidget(m_createButton);

		QFormLayout *layout = new QFormLayout;
		layout->addRow(pdfBrowseLayout);
		layout->addRow(outputBrowseLayout);
		layout->addRow(m_creatorLinkLabel, m_creatorLink);
		layout->addRow(m_watermarkTextLabel, m_watermarkText);
		layout->addRow(m_fileCommentsLabel, m_fileComments);
		layout->addItem(extraElementsLayout);
		layout->setRowWrapPolicy(QFormLayout::WrapAllRows);
		setLayout(layout);

		// Force counters to be updated
		emit m_creatorLink->textChanged(m_creatorLink->text());
		emit m_watermarkText->textChanged(m_watermarkText->text());
		emit m_fileComments->textChanged();
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

		// Trim all texts and make sure the are not meaningless
		QString trimmedLinkText = m_creatorLink->text().trimmed();
		QString trimmedWatermarkText = m_watermarkText->text().trimmed();
		QString trimmedCommentsText = m_fileComments->toPlainText().trimmed();

		if (trimmedLinkText.isEmpty() || trimmedWatermarkText.isEmpty() || trimmedCommentsText.isEmpty()) {
			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Error"));
			msgBox.setText(tr("Text fields can not be empty or contains only spaces."));
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.exec();

			return;
		}
		
		// Prepare to run operation
		WaitDialog splash(tr("Operation in progress"), tr("Please wait"), this);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), &splash, SLOT(close()));
		QFuture<void> future = QtConcurrent::run(processData, m_pdfFile, m_outputFile, trimmedLinkText, trimmedWatermarkText, trimmedCommentsText);
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
		m_createButton->setEnabled(
			!m_pdfFile.isEmpty() && 
			!m_outputFile.isEmpty() && 
			!m_creatorLink->text().isEmpty() &&
			!m_watermarkText->text().isEmpty() &&
			!m_fileComments->toPlainText().isEmpty()
		);
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
	QLabel *m_creatorLinkLabel;
	QLabel *m_watermarkTextLabel;
	QLabel *m_fileCommentsLabel;

	const unsigned short m_maxWatermarkChars = 128;
	const unsigned short m_maxLinkChars = 128;
	const unsigned short m_maxCommentsChars = 256;
};

#endif // MAINDIALOG_H