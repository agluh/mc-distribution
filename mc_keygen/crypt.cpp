#include <QtCore\QString>
#include <QtCore\QDebug>
#include <QtCore\QFile>
#include <QtCore\QDir>
#include <cassert>

#include "../mc_common/CryptoUtils.h"
#include "utils.h"

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
void parseRequest(const QByteArray &data, QString *watermarkText, QString *commentsText, QString *linkText, QByteArray *iv, QByteArray *key, QByteArray *hid) {
	assert(watermarkText != nullptr);
	assert(commentsText != nullptr);
	assert(linkText != nullptr);
	assert(iv != nullptr);
	assert(key != nullptr);
	assert(hid != nullptr);

	int offset = 0;
	
	// Read AES IV
	*iv = data.mid(0, CryptoUtils::AESiv);
	offset += iv->size();
	if (iv->size() != CryptoUtils::AESiv) {
		throw RuntimeError("Incorrect data");
	}

	// Read AES key
	int len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);
	QByteArray key_ = data.mid(offset, len);
	if (!len || key_.size() != len) {
		throw RuntimeError("Incorrect data");
	}
	offset += key_.size();

	// Decrypt AES key
	*key = CryptoUtils::decryptRSA(key_, ":/private.dat");

	// Read watermark
	len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);

	if (len != 0) {
		QByteArray watermark_ = data.mid(offset, len);
		if (watermark_.size() != len) {
			throw RuntimeError("Incorrect data");
		}
		offset += watermark_.size();

		// Decrypt watermark
		*watermarkText = QString(CryptoUtils::decryptAES_CBC(watermark_, *key, *iv));
	}
	else {
		*watermarkText = QObject::tr("<None>");
	}

	// Read comments
	len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);

	if (len != 0) {
		QByteArray comments_ = data.mid(offset, len);
		if (comments_.size() != len) {
			throw RuntimeError("Incorrect data");
		}
		offset += comments_.size();

		// Decrypt comments
		*commentsText = QString(CryptoUtils::decryptRSA(comments_, ":/private.dat"));
	}
	else {
		*commentsText = QObject::tr("<None>");
	}

	// Read link
	len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);

	if (len != 0) {
		QByteArray link_ = data.mid(offset, len);
		if (link_.size() != len) {
			throw RuntimeError("Incorrect data");
		}
		offset += link_.size();

		*linkText = QString(link_);
	}
	else {
		*linkText = QObject::tr("<None>");
	}

	// Read hardware ID
	len = 0;
	memcpy(&len, data.data() + offset, sizeof(len));
	offset += sizeof(len);
	QByteArray hid_ = data.mid(offset, len);
	if (!len || hid_.size() != len) {
		throw RuntimeError("Incorrect data");
	}

	// Decrypt hardware ID
	*hid = CryptoUtils::decryptRSA(hid_, ":/private.dat");
}

/*
	Use first chars of HID as key and encrypt original AES key with it
*/
QString generateResponse(const QByteArray &hid, const QByteArray &keyData) {
	QByteArray key = hid.mid(0, CryptoUtils::AESkey);
	QByteArray iv = CryptoUtils::generateBlock(CryptoUtils::AESiv);

	QByteArray buff = CryptoUtils::encryptAES_CBC(keyData, key, iv);
	
	QByteArray data;
	data.append(iv); // add IV at first
	data.append(buff);

	return QString(data.toBase64());
}