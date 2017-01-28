#include <QtCore\QString>
#include <QtCore\QFile>
#include <QtCore\QDebug>
#include <QtGui\QtGui>
#include <QtWidgets\QApplication>
#include <QtWidgets\QWidget>
#include <QtWidgets\QMessageBox>
#include <QtCore\QtPlugin>
#include <cassert>

#include "utils.h"
#include "FileParser.h"
#include "../mc_common/CryptoUtils.h"

FileParser::FileParser(const QByteArray &data) : m_document(nullptr) {
	int offset = 0;

	// Read AES IV
	m_iv = data.mid(0, CryptoUtils::AESiv);
	if (m_iv.size() != CryptoUtils::AESiv) {
		throw RuntimeError(QObject::tr("File is incorrect."));
	}
	offset += m_iv.size();

	// Read AES key
	int len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);
	m_key = data.mid(offset, len);
	if (!len || m_key.size() != len) {
		throw RuntimeError(QObject::tr("File is incorrect."));
	}
	offset += m_key.size();

	// Read watermark
	len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);
	m_watermark = data.mid(offset, len);
	if (!len || m_watermark.size() != len) {
		throw RuntimeError(QObject::tr("File is incorrect."));
	}
	offset += m_watermark.size();

	// Read comments
	len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);
	m_comments = data.mid(offset, len);
	if (!len || m_comments.size() != len) {
		throw RuntimeError(QObject::tr("File is incorrect."));
	}
	offset += m_comments.size();

	// Read link
	len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);
	m_link = data.mid(offset, len);
	if (!len || m_link.size() != len) {
		throw RuntimeError(QObject::tr("File is incorrect."));
	}
	offset += m_link.size();

	// Read PDF data
	len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);
	m_pdfData = data.mid(offset, len);
	if (!len || m_pdfData.size() != len) {
		throw RuntimeError(QObject::tr("File is incorrect."));
	}
	offset += m_pdfData.size();

	assert(offset == data.size());

	// Generate hash from file
	QCryptographicHash hash(QCryptographicHash::Sha256);
	hash.addData(data);
	m_hash = hash.result();

	m_hid = getHardwareID(m_hash);
}

/*
	Structure of request data:

	Size	|   Encryption	|		Description
	----------------------------------------------------------
	byte[16]|	-			|	AES IV
	DWORD	|	-			|	Length of AES key
	byte[]	|	RSA			|	AES key
	DWORD	|	-			|	Length of encrypted watermark
	byte[]	|	AES			|	Encrypted watermark data
	DWORD	|	-			|	Length of encrypted comment
	byte[]	|	RSA			|	Encrypted comment
	DWORD	|	-			|	Length of link data
	byte[]	|	-			|	Link data
	DWORD	|	-			|	Length of encrypted hardware ID
	byte[]	|	RSA			|	Encrypted hardware ID

*/
QString FileParser::createRequestData() const {
	QByteArray data;
	data.append(m_iv);

	int len = m_key.size();
	data.append((const char*)&len, sizeof(len));
	data.append(m_key);

	len = m_watermark.size();
	data.append((const char*)&len, sizeof(len));
	data.append(m_watermark);

	len = m_comments.size();
	data.append((const char*)&len, sizeof(len));
	data.append(m_comments);

	len = m_link.size();
	data.append((const char*)&len, sizeof(len));
	data.append(m_link);

	QByteArray hid = CryptoUtils::encryptRSA(m_hid, ":/public.dat");
	len = hid.size();
	data.append((const char*)&len, sizeof(len));
	data.append(hid);

	return QString(CryptoUtils::beautifyBase64(data.toBase64()));
}

void FileParser::parseResponse(const QByteArray &response) {
	QByteArray iv = response.mid(0, CryptoUtils::AESiv);	// Get IV from response
	QByteArray key = m_hid.mid(0, CryptoUtils::AESkey);		// Use first chars of HID as key
	
	QByteArray keyEncrypted = response.mid(iv.size());
	m_keyDecrypted = CryptoUtils::decryptAES_CBC(keyEncrypted, key, iv);

	QByteArray dataDecrypted = CryptoUtils::decryptAES_CBC(m_pdfData, m_keyDecrypted, m_iv);

	// Try to read PDF data
	m_document = Poppler::Document::loadFromData(dataDecrypted);
}

QString FileParser::getWatermarkText() const {
	return CryptoUtils::decryptAES_CBC(m_watermark, m_keyDecrypted, m_iv);
}

Poppler::Document* FileParser::getPdfDocument() const {
	return m_document;
}