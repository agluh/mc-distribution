#include <QtCore\QString>
#include <QtCore\QDebug>

#include "utils.h"

void processData(const QString &pdfFile, QFile *outputFile, const QString &link, const QString &watermark, const QString &comments) {
	assert(outputFile != nullptr);
	QString outputFileName = outputFile->fileName();

	// Try to load viewer executable files from resources
	unsigned long dataSize = 0;
	void* pViewerData = loadViewerData(&dataSize);
	if (!pViewerData || dataSize == 0) {
		throw RuntimeError(QObject::tr("Unable to load data from resources."));
	}

	// Create protected data
	QByteArray data = createData(pdfFile, link, watermark, comments);

	// Write output viewer file
	if (!outputFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		throw RuntimeError(QObject::tr("Unable to create output file %1.").arg(outputFileName));
	}

	if (outputFile->write((const char*)pViewerData, dataSize) != dataSize) {
		throw RuntimeError(QObject::tr("Unable to write data to output file %1.").arg(outputFileName));
	}

	outputFile->flush();
	outputFile->close();

	// Write protected data as resourse to the output file
	if (!updateViewerResources(outputFileName.toStdString().c_str(), data.data(), data.size())) {
		throw RuntimeError(QObject::tr("Unable to update data of output file %1.").arg(outputFileName));
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
		arguments.append(outputFileName);

		QProcess::execute(upx, arguments);
	}
}