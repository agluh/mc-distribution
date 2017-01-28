#ifndef UTILS_H
#define UTILS_H

#pragma once 

#include <exception>
#include <QtCore\QString>
#include <QtCore\QByteArray>

void parseRequest(const QByteArray &data, QString *watermarkText, QString *commentsText, QString *linkText, QByteArray *iv, QByteArray *key, QByteArray *hid);
QString generateResponse(const QByteArray &hid, const QByteArray &key);

//====================================
// Helper class for manage errors
//
class RuntimeError : public std::runtime_error {
public:
	RuntimeError(QString what) : std::runtime_error(what.toStdString()) {}
};

#endif // UTILS_H