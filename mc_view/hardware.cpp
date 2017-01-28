#include <QtCore\QString>
#include <QtCore\QDir>
#include <QtCore\QByteArray>
#include <QtCore\QDebug>
#include <QtCore\QCryptographicHash>
#include <QtCore\QCoreApplication>

#include "../mc_common/CryptoUtils.h"
#include "utils.h"

/*
	Generates a hardware ID from info about CPU, 
	MAC adresses, HDD serial numbers and hash of data file.
	Returns Sha256 string.
*/
QByteArray getHardwareID(const QByteArray &hash) {
	QString mac = getMacAddress();
	QString cpu = getCPUInfo();
	QString hdd = getHDDInfo();

	QString buff = QString(hash.toBase64()) + ";" + mac + cpu + hdd;

	return QCryptographicHash::hash(buff.toLatin1(), QCryptographicHash::Sha256).toHex();
}