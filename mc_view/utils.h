#ifndef UTILS_H
#define UTILS_H

#pragma once 

#include <exception>
#include <QtCore\QString>

QString getMacAddress();
QString getCPUInfo();
QString getHDDInfo();
QByteArray getHardwareID(const QByteArray &);
void* loadData(unsigned long *);

//====================================
// Helper class for manage errors
//
class RuntimeError : public std::runtime_error {
public:
	RuntimeError(QString what) : std::runtime_error(what.toStdString()) {}
};

#endif // UTILS_H