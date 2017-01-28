#include <QtCore\QString>
#include <QtCore\QDebug>
#include <QtCore\QFile>
#include <QtWidgets\QMessageBox>
#include <cassert>

#include "../mc_common/CryptoUtils.h"
#include "utils.h"

/*
	Structure of output data:

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
	DWORD	|	-			|	Length of encrypted PDF data
	byte[]	|	AES			|	Encrypted PDF data

*/
QByteArray createData(const QString &pdfFileName, const QString &creatorLink, const QString &watermarkText, const QString &fileComments) {
	QByteArray output;

	// Generate a random AES key
	QByteArray keyAES = CryptoUtils::generateBlock(CryptoUtils::AESkey);
	QByteArray ivAES = CryptoUtils::generateBlock(CryptoUtils::AESiv);

	// Convert link string to byte array
	QByteArray creatorLinkData = creatorLink.toUtf8();

	// Use RSA encryption for AES key
	QByteArray keyAESEncrypted = CryptoUtils::encryptRSA(keyAES, ":/public.dat");

	// Encrypt watermark
	QByteArray watermarkTextEncrypted = CryptoUtils::encryptAES_CBC(watermarkText.toUtf8(), keyAES, ivAES);

	// Use RSA encryption for comment
	QByteArray fileCommentsEncrypted = CryptoUtils::encryptRSA(fileComments.toUtf8(), ":/public.dat");

	// Open PDF file and read it content to the buffer
	QByteArray pdfContent;
	QFile pdfFile(pdfFileName);
	if (!pdfFile.exists() || !pdfFile.open(QIODevice::ReadOnly)) {
		throw RuntimeError(QObject::tr("Unable to read PDF file."));
	}
	
	pdfContent = pdfFile.readAll();
	pdfFile.close();

	// Encrypt PDF file content
	QByteArray pdfContentEncrypted = CryptoUtils::encryptAES_CBC(pdfContent, keyAES, ivAES);

	// Append IV
	output.append(ivAES);
		
	// Append size of encrypted AES key and key itself
	int len = keyAESEncrypted.size();
	output.append((char*)&len, sizeof(len));
	output.append(keyAESEncrypted);

	// Append size of encrypted watermark and watermark itself
	len = watermarkTextEncrypted.size();
	output.append((char*)&len, sizeof(len));
	output.append(watermarkTextEncrypted);

	// Append size of encrypted comment and comment itself
	len = fileCommentsEncrypted.size();
	output.append((char*)&len, sizeof(len));
	output.append(fileCommentsEncrypted);

	// Append size of creator link and link itself
	len = creatorLinkData.size();
	output.append((char*)&len, sizeof(len));
	output.append(creatorLinkData);

	// Append size PDF data and data itself
	len = pdfContentEncrypted.size();
	output.append((char*)&len, sizeof(len));
	output.append(pdfContentEncrypted);

	return output;
}