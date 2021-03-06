#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once 

#include <QtCore\QDebug>
#include <QtCore\QBuffer>
#include <QtWidgets\QMainWindow>
#include <QtWidgets\QVBoxLayout>
#include <QtWidgets\QPushButton>
#include <QtWidgets\QScrollArea>
#include <QtWidgets\QComboBox>
#include <QtWidgets\QLabel>
#include <QtWidgets\QSpacerItem>
#include <QtPrintSupport\QPrinter>
#include <QtPrintSupport\QPrintDialog>
#include <QtWidgets\QMessageBox>

#include <minmax.h>
#include <poppler-qt5.h>

#include "MainWidget.hpp"

//====================================
// Class for main window
//
class MainWindow : public QMainWindow {
	Q_OBJECT;

public:
	MainWindow(Poppler::Document *document, const QString &watermarkText, const QString &linkText) 
		: m_pdfDocument(document), m_watermarkText(watermarkText), m_currentPage(0), m_currentZoom(1.0) 
	{
		setWindowTitle(tr("Viewer for protected PDF files"));

		// Setup window layout
		m_mainWidget = new MainWidget(document, watermarkText, this);

		m_scrollArea = new QScrollArea(this);
		m_scrollArea->setBackgroundRole(QPalette::Dark);
		m_scrollArea->setWidget(m_mainWidget);
		m_scrollArea->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

		// Buttons
		m_prevPageButton = new QPushButton("<<", this);
		m_nextPageButton = new QPushButton(">>", this);
		m_printButton = new QPushButton(tr("&Print..."), this);
		connect(m_prevPageButton, SIGNAL(released()), this, SLOT(prevButtonPressed()));
		connect(m_nextPageButton, SIGNAL(released()), this, SLOT(nextButtonPressed()));
		connect(m_printButton, SIGNAL(released()), this, SLOT(printButtonPressed()));

		// Link
		m_linkText = new QLabel(linkText, this);
		m_linkText->setTextFormat(Qt::RichText);
		m_linkText->setTextInteractionFlags(Qt::TextBrowserInteraction);
		m_linkText->setOpenExternalLinks(true);
		m_linkText->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		m_linkText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

		// Zoom combobox
		m_zoomCombobox = new QComboBox(this);
		m_zoomCombobox->addItem("50%", 0.5);
		m_zoomCombobox->addItem("60%", 0.6);
		m_zoomCombobox->addItem("75%", 0.75);
		m_zoomCombobox->addItem("100%", 1.0);
		m_zoomCombobox->addItem("125%", 1.25);
		m_zoomCombobox->addItem("150%", 1.5);
		m_zoomCombobox->addItem("175%", 1.75);
		m_zoomCombobox->addItem("200%", 2.0);
		m_zoomCombobox->setCurrentIndex(3);

		connect(m_zoomCombobox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[=](int index){ zoomChanged(index); });

		// Page counter
		m_pageCounter = new QLabel(this);
		m_pageCounter->setFrameStyle(QFrame::NoFrame);
		m_pageCounter->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		
		// Layout
		m_hlayout = new QHBoxLayout();
		m_hlayout->addWidget(m_zoomCombobox);
		m_hlayout->addWidget(m_printButton);
		m_hlayout->addWidget(m_linkText);
		m_hlayout->addWidget(m_prevPageButton);
		m_hlayout->addWidget(m_pageCounter);
		m_hlayout->addWidget(m_nextPageButton);
		m_hlayout->setContentsMargins(8, 0, 8, 8);
		m_hlayout->setSpacing(10);

		m_vlayout = new QVBoxLayout();
		m_vlayout->addWidget(m_scrollArea);
		m_vlayout->addItem(m_hlayout);
		m_vlayout->setContentsMargins(0, 0, 0, 0);

		m_placeholder = new QWidget(this);
		m_placeholder->setLayout(m_vlayout);
		setCentralWidget(m_placeholder);

		updatePage();
	}

	~MainWindow() {
		// iterate through list of children then call delete
		QObjectList children = this->children();
		for (auto it = children.begin(); it != children.end(); it++) {
			delete *it;
		}
	}

private slots:
	void prevButtonPressed() {
		m_currentPage--;
		if (m_currentPage < 0)
			m_currentPage = 0;

		updatePage();
	}

	void nextButtonPressed() {
		m_currentPage++;
		if (m_currentPage > m_pdfDocument->numPages() - 1)
			m_currentPage = m_pdfDocument->numPages() - 1;

		updatePage();
	}

	void printButtonPressed() {
		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::NativeFormat);

		QPrintDialog printDialog(&printer, this);
		printDialog.setOptions(QAbstractPrintDialog::PrintCurrentPage | QAbstractPrintDialog::PrintPageRange);
		if (printDialog.exec() == QDialog::Accepted) {
			int pageCount = m_pdfDocument->numPages();
			int firstPage = 0;
			int lastPage = 0;

			if (printer.printRange() == QPrinter::AllPages) {
				lastPage = pageCount - 1;
			}

			if (printer.printRange() == QPrinter::PageRange) {
				firstPage = printer.fromPage() - 1;
				lastPage = min(printer.toPage() - 1, pageCount - 1);
			}

			if (printer.printRange() == QPrinter::CurrentPage) {
				firstPage = lastPage = m_currentPage;
			}

			print(&printer, firstPage, lastPage);
		}
		
	}

	void zoomChanged(int index) {
		m_currentZoom = m_zoomCombobox->itemData(index).toDouble();
		updatePage();
	}

protected:

	void print(QPrinter *printer, int fromPage, int toPage) {
		int resolution = printer->resolution();
		if (resolution > m_maxPrintResolution) {
			QMessageBox::StandardButton reply = QMessageBox::warning(this, QObject::tr("Print"), QObject::tr("Value of selected printing resolution %1dpi is too hight. Maximum allowed value is %2dpi.<br/><br/>Do you want to scale it down to allowed value and continue printing?").arg(resolution).arg(m_maxPrintResolution), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
			if (reply == QMessageBox::No) {
				return;
			}
			printer->setResolution(m_maxPrintResolution);
			resolution = m_maxPrintResolution;
		}
		QRectF printerPageRect = printer->pageRect(QPrinter::DevicePixel);

		QPainter painter;
		painter.begin(printer);

		for (int page = fromPage; page <= toPage; page++) {
			Poppler::Page* pdfPage = m_pdfDocument->page(page);
			if (pdfPage == nullptr) {
				return;
			}

			// Draw PDF content
			QImage img = pdfPage->renderToImage(resolution, resolution, -1, -1, -1, -1, Poppler::Page::Rotate0);
			painter.drawImage(printerPageRect, img);
			delete pdfPage; 
			pdfPage = nullptr;

			// Prepare watermark if it exists
			if (m_watermarkText.length()) {
				QSize size = img.size();
				int watermarkOffset = 0.5 * size.height();

				double len = 0.70710678118 * (size.width() + size.height());
				QPixmap *watermarkPixmap = new QPixmap(len, len);
				if (watermarkPixmap == nullptr) {
					return;
				}

				watermarkPixmap->fill(Qt::transparent);

				QString watermark_text;
				for (int i = 0; i < 300; i++) {
					watermark_text += "\t\t" + m_watermarkText;
				}

				QPainter *p = new QPainter(watermarkPixmap);
				if (p == nullptr) {
					return;
				}

				p->setPen(QPen(QColor(0, 0, 0, 100)));
				p->setFont(QFont("Arial", 32 * resolution / 72, QFont::Bold));
				p->drawText(0, 0, len, len, Qt::AlignCenter | Qt::TextWrapAnywhere, watermark_text);
				p->end();

				delete p; 
				p = nullptr;

				// Draw watermarks
				painter.save();
				painter.translate(-watermarkOffset, watermarkOffset);
				painter.rotate(-45);
				painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_SoftLight);
				painter.drawPixmap(0, 0, *watermarkPixmap);
				painter.restore();

				if (watermarkPixmap != nullptr) {
					delete watermarkPixmap;
					watermarkPixmap = nullptr;
				}
			}

			if (page != toPage) {
				printer->newPage();
			}
		}

		painter.end();
	}

	void updatePage() {
		QString counterString = QString::asprintf("%d/%d", m_currentPage + 1, m_pdfDocument->numPages());
		m_pageCounter->setText(counterString);

		QSize pageSize = m_pdfDocument->page(m_currentPage)->pageSize();
		pageSize.setWidth(pageSize.width() * this->logicalDpiX() / 72 * m_currentZoom);
		pageSize.setHeight(pageSize.height() * this->logicalDpiY() / 72 * m_currentZoom);

		m_mainWidget->setSettings(m_currentPage, m_currentZoom);
		m_mainWidget->setFixedWidth(pageSize.width());
		m_mainWidget->setFixedHeight(pageSize.height());

		m_prevPageButton->setDisabled(m_currentPage == 0);
		m_nextPageButton->setDisabled(m_currentPage == m_pdfDocument->numPages() - 1);

		m_mainWidget->update();
	}

	void keyPressEvent(QKeyEvent *event) {
		switch (event->key()) {
		case Qt::Key_Left:
			prevButtonPressed();
			break;

		case Qt::Key_Right:
			nextButtonPressed();
			break;
		}

		QMainWindow::keyPressEvent(event);
	}

private:
	MainWidget *m_mainWidget;
	QComboBox *m_zoomCombobox;
	QPushButton *m_prevPageButton;
	QPushButton *m_nextPageButton;
	QPushButton *m_printButton;
	QLabel *m_pageCounter;
	QLabel *m_linkText;
	QString m_watermarkText;
	QScrollArea *m_scrollArea;
	QHBoxLayout *m_hlayout;
	QVBoxLayout *m_vlayout;
	QWidget* m_placeholder;

	Poppler::Document *m_pdfDocument;
	int m_currentPage;
	double m_currentZoom;

	const short m_maxPrintResolution = 300;
};

#endif MAINWINDOW_H