#include <QtCore\QString>
#include <QtCore\QFile>
#include <QtGui\QtGui>
#include <QtWidgets\QApplication>
#include <QtWidgets\QWidget>
#include <QtWidgets\QMessageBox>
#include <QtCore\QTranslator>
#include <QtCore\QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QWindowsPrinterSupportPlugin)

#include <poppler-qt5.h>
#include <cassert>

#include "MainWindow.hpp"
#include "MainDialog.hpp"
#include "utils.h"
#include "FileParser.h"

#include "resource.h"

//====================================
// Entry point
//
int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/resources/main.png"));

	QTranslator translator;
	translator.load(":/gen/translations/translations_ru.qm");
	a.installTranslator(&translator);

	QString exeFilename = QFileInfo(QCoreApplication::applicationFilePath()).completeBaseName();
	QString licFilename = exeFilename + ".lic";

	try {
		bool done = false;

		unsigned long dataSize = 0;
		void *pData = loadData(&dataSize);
		if (!pData || dataSize == 0) {
			throw RuntimeError(QObject::tr("Unable to load data from resources."));
		}

		QByteArray data((const char*)pData, dataSize);
		FileParser p(data);
		QFile lic(licFilename);

		try {
			// Try to find *.lic file and load key from it
			if (lic.exists() && lic.open(QIODevice::ReadOnly)) {
				QByteArray response = lic.readAll();
				lic.close();
				p.parseResponse(response);
				done = true;
			}
		}
		catch (...) {
			QMessageBox msgBox;
			msgBox.setWindowTitle(QObject::tr("Error"));
			msgBox.setText(QObject::tr("Licence file %1 is incorrect.").arg(licFilename));
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}

		if (!done) {
			MainDialog dialog(&p);
			dialog.setRequestText(p.createRequestData());
			if (dialog.exec() == QDialog::Accepted) {
				// Save *.lic file
				if (lic.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
					QString response = dialog.getResponseText();
					QByteArray responseData = QByteArray::fromBase64(response.toLatin1());
					lic.write(responseData);
					lic.flush();
					lic.close();
				}

				done = true;
			}
		}

		if (done) {
			MainWindow window(p.getPdfDocument(), p.getWatermarkText(), p.getLinkText());
			window.resize(600, 400);
			window.show();

			return a.exec();
		}
	}
	catch (RuntimeError &e) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(QObject::tr("Error"));
		msgBox.setText(QObject::tr("An error has occurred: %1").arg(e.what()));
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.exec();
	}
	catch (...) {
		QMessageBox msgBox;
		msgBox.setWindowTitle(QObject::tr("Error"));
		msgBox.setText(QObject::tr("An unknown error has occurred."));
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.exec();
	}

	return 1;
}