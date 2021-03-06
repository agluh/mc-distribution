#ifndef UTILS_H
#define UTILS_H

#pragma once 

#include <exception>
#include <cassert>
#include <QtCore\QString>
#include <QtCore\QFuture>
#include <QtConcurrent\QtConcurrent>

bool isCorrectPDF(const QString &);

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