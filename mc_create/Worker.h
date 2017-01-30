#ifndef WORKER_H
#define WORKER_H

#pragma once 

#include <QtCore\QString>
#include <QtCore\QFuture>
#include <QtConcurrent\QtConcurrent>

class Worker {
public:
	Worker(const QString &pdfFile, const QString &outputFileName, const QString &link, const QString &watermark, const QString &comments) :
		m_pdfFile(pdfFile), m_outputFileName(outputFileName), m_link(link), m_watermark(watermark), m_comments(comments) 
	{}

	void doWork(QFutureInterface<void> &future);

protected:
	QByteArray createData();
	void* loadViewerData(unsigned long *size);
	bool updateViewerResources(const char *file, void *data, unsigned long size);

private:
	QString m_pdfFile;
	QString m_outputFileName;
	QString m_link;
	QString m_watermark;
	QString m_comments;
};

#endif // WORKER_H