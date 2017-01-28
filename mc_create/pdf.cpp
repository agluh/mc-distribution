#include <poppler-qt5.h>

bool isCorrectPDF(const QString &pdfFile) {
	Poppler::Document* document = Poppler::Document::load(pdfFile);
	if (!document) {
		return false;
	}

	if (document->isLocked()) {
		delete document;
		return false;
	}

	delete document;
	return true;
}