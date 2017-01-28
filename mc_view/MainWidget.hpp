#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#pragma once 

#include <QtCore\QDebug>
#include <QtWidgets\QWidget>
#include <QtGui\QPainter>
#include <QtGui\QPaintEvent>

#include <poppler-qt5.h>

//====================================
// Class for main winget
//
class MainWidget : public QWidget {
	Q_OBJECT

public:
	MainWidget(Poppler::Document *document, const QString &watermarkText, QWidget *parent = 0) : m_watermarkText(watermarkText), m_pdfDocument(document), QWidget(parent), m_watermarkPixmap(nullptr), m_watermarkOffset(0.0), m_currentPage(0), m_currentZoom(1.0) {
		setBackgroundRole(QPalette::Base);
		setAutoFillBackground(true);

		m_pdfDocument->setRenderBackend(Poppler::Document::SplashBackend);
		m_pdfDocument->setRenderHint(Poppler::Document::Antialiasing);
		m_pdfDocument->setRenderHint(Poppler::Document::TextAntialiasing);
	}

	~MainWidget() {
		if (m_watermarkPixmap != nullptr) {
			delete m_watermarkPixmap;
		}
	}

	void setSettings(int pageNum, double zoom) {
		m_currentPage = pageNum;
		m_currentZoom = zoom;

		Poppler::Page* pdfPage = m_pdfDocument->page(m_currentPage);
		if (pdfPage == 0) {
			return;
		}

		if (m_watermarkPixmap != nullptr) {
			delete m_watermarkPixmap;
		}

		m_pageThumb = pdfPage->renderToImage(this->logicalDpiX() * m_currentZoom, this->logicalDpiY() * m_currentZoom, -1, -1, -1, -1, Poppler::Page::Rotate0);
		QSize size = m_pageThumb.size();
		m_watermarkOffset = 0.5 * size.height();

		double len = 0.70710678118 * (size.width() + size.height());
		m_watermarkPixmap = new QPixmap(len, len);
		m_watermarkPixmap->fill(Qt::transparent);

		QString watermark_text;
		for (int i = 0; i < 300; i++) {
			watermark_text += "\t\t" + m_watermarkText;
		}

		QPainter *p = new QPainter(m_watermarkPixmap);
		p->setPen(QPen(QColor(0, 0, 0, 100)));
		p->setFont(QFont("Arial", 32 * m_currentZoom, QFont::Bold));
		p->drawText(0, 0, len, len, Qt::AlignCenter | Qt::TextWrapAnywhere, watermark_text);
		p->end();
	}

protected:
	void paintEvent(QPaintEvent *event) {
		QPainter painter(this);
		draw(&painter);
	}

	void draw(QPainter *painter) {
		// Draw PDF content
		painter->drawImage(QPoint(0, 0), m_pageThumb);
		
		// Draw watermarks
		painter->translate(-m_watermarkOffset, m_watermarkOffset);
		painter->rotate(-45);
		painter->setCompositionMode(QPainter::CompositionMode::CompositionMode_SoftLight);
		painter->drawPixmap(0, 0, *m_watermarkPixmap);
		painter->end();

	}

private:
	QImage m_pageThumb;
	QPixmap *m_watermarkPixmap;

	int m_currentPage;
	double m_currentZoom;
	double m_watermarkOffset;
	QString m_watermarkText;

	Poppler::Document *m_pdfDocument;
};

#endif // MAINWIDGET_H