#include <QtCore\QString>
#include <QtCore\QDebug>
#include <QtCore\QFile>
#include <cassert>

#include "Worker.h"
#include "utils.h"
#include "../mc_common/CryptoUtils.h"

void Worker::doWork(QFutureInterface<void> &future) {
	future.setProgressRange(0, 6);

	QFile outputFile(m_outputFileName);

	// Try to load viewer executable files from resources
	future.setProgressValueAndText(1, QObject::tr("Load data from resources..."));
	unsigned long dataSize = 0;
	void* pViewerData = loadViewerData(&dataSize);
	if (!pViewerData || dataSize == 0) {
		throw RuntimeError(QObject::tr("Unable to load data from resources."));
	}

	// Create protected data
	future.setProgressValueAndText(2, QObject::tr("Process PDF file..."));
	QByteArray data = createData();

	// Write output viewer file
	if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		throw RuntimeError(QObject::tr("Unable to create output file %1.").arg(m_outputFileName));
	}

	future.setProgressValueAndText(3, QObject::tr("Write data to output file..."));
	if (outputFile.write((const char*)pViewerData, dataSize) != dataSize) {
		throw RuntimeError(QObject::tr("Unable to write data to output file %1.").arg(m_outputFileName));
	}

	outputFile.flush();
	outputFile.close();

	// Write protected data as resourse to the output file
	future.setProgressValueAndText(4, QObject::tr("Update data of output file..."));
	std::string str = std::string(m_outputFileName.toLocal8Bit().constData());
	if (!updateViewerResources(str.c_str(), data.data(), data.size())) {
		throw RuntimeError(QObject::tr("Unable to update data of output file %1.").arg(m_outputFileName));
	}

	// Try to find UPX util and pack file
	QString appDir = QCoreApplication::applicationDirPath();
	QString upx;

	QDirIterator it(appDir, QStringList() << "upx.exe", QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()){
		upx = it.next();
	}

	if (QFile(upx).exists()) {
		QStringList arguments;
		arguments.append("--best");
		arguments.append("--lzma");
		arguments.append(m_outputFileName);

		future.setProgressValueAndText(5, QObject::tr("Compress output file..."));
		QProcess::execute(upx, arguments);
	}

	future.setProgressValue(6);
}

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
QByteArray Worker::createData() {
	QByteArray output, link, watermark, comments;

	// Generate a random AES key
	QByteArray keyAES = CryptoUtils::generateBlock(CryptoUtils::AESkey);
	QByteArray ivAES = CryptoUtils::generateBlock(CryptoUtils::AESiv);

	// Use RSA encryption for AES key
	QByteArray key = CryptoUtils::encryptRSA(keyAES, ":/public.dat");

	// Convert link string to byte array
	if (m_link.length()) {
		link = m_link.toUtf8();
	}

	// Encrypt watermark
	if (m_watermark.length()) {
		watermark = CryptoUtils::encryptAES_CBC(m_watermark.toUtf8(), keyAES, ivAES);
	}

	// Use RSA encryption for comment
	if (m_comments.length()) {
		comments = CryptoUtils::encryptRSA(m_comments.toUtf8(), ":/public.dat");
	}

	// Open PDF file and read it content to the buffer
	QFile pdfFile(m_pdfFile);
	if (!pdfFile.exists() || !pdfFile.open(QIODevice::ReadOnly)) {
		throw RuntimeError(QObject::tr("Unable to read PDF file."));
	}

	QByteArray pdf = pdfFile.readAll();
	pdfFile.close();

	// Encrypt PDF file content
	QByteArray pdfEncrypted = CryptoUtils::encryptAES_CBC(pdf, keyAES, ivAES);

	// Append IV
	output.append(ivAES);

	// Append size of encrypted AES key and key itself
	int len = key.size();
	output.append((char*)&len, sizeof(len));
	output.append(key);

	// Append size of encrypted watermark and watermark itself
	len = watermark.size();
	output.append((char*)&len, sizeof(len));
	output.append(watermark);

	// Append size of encrypted comment and comment itself
	len = comments.size();
	output.append((char*)&len, sizeof(len));
	output.append(comments);

	// Append size of creator link and link itself
	len = link.size();
	output.append((char*)&len, sizeof(len));
	output.append(link);

	// Append size PDF data and data itself
	len = pdfEncrypted.size();
	output.append((char*)&len, sizeof(len));
	output.append(pdfEncrypted);

	return output;
}