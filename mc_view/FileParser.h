#ifndef FILEPARSER_H
#define FILEPARSER_H

#pragma once 

#include <QtCore\QString>
#include <QtCore\QFile>

#include <poppler-qt5.h>

class FileParser {
public:
	FileParser(const QByteArray &data);
	~FileParser() { 
		if (m_document != nullptr) 
			delete m_document; 
	}

	QString getLinkText() const { return QString(m_link); }
	QString createRequestData() const;
	void parseResponse(const QByteArray &response);
	void parseResponse(const QString &response) {
		QByteArray responseData = QByteArray::fromBase64(response.toLatin1());
		parseResponse(responseData);
	}
	QString getWatermarkText() const;
	Poppler::Document* getPdfDocument() const;

private:
	QByteArray m_iv;
	QByteArray m_key;
	QByteArray m_watermark;
	QByteArray m_comments;
	QByteArray m_link;
	QByteArray m_hash;
	QByteArray m_hid;
	QByteArray m_keyDecrypted;

	QByteArray m_pdfData;

	Poppler::Document *m_document;
};

#endif // FILEPARSER_H