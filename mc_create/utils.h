#ifndef UTILS_H
#define UTILS_H

#pragma once 

#include <exception>
#include <cassert>
#include <QtCore\QString>
#include <QtCore\QFuture>
#include <QtConcurrent\QtConcurrent>

QByteArray createData(const QString &, const QString &, const QString &, const QString &);
bool isCorrectPDF(const QString &);
void* loadViewerData(unsigned long *);
bool updateViewerResources(const char*, void *, unsigned long);
void processData(const QString &, QFile *, const QString &, const QString &, const QString &);

//====================================
// Helper class for manage errors
//
class RuntimeError : public QtConcurrent::Exception {
public:
	RuntimeError(QString what) : m_what(what.toStdString()) {}
	RuntimeError(std::string what) : m_what(what) {}
	void raise() const { throw *this; }
	RuntimeError* clone() const { return new RuntimeError(this->m_what); }
	const char* what() const { return m_what.c_str(); }
private:
	std::string m_what;
};

#endif // UTILS_H