#include <QtWidgets\QApplication>
#include <QtCore\QTranslator>
#include <QtCore\QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

#include "MainDialog.hpp"

//====================================
// Entry point
//
int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/resources/main.png"));

	QTranslator translator;
	translator.load(":/gen/translations/translations_ru.qm");
	a.installTranslator(&translator);

	MainDialog dialog;
	dialog.show();

	return a.exec();
}