#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#pragma once 

#include <QtCore\QString>
#include <QtCore\QCryptographicHash>
#include <QtCore\QByteArray>

//====================================
// Helper class for crypto
//
class CryptoUtils {
public:

	enum BlockSize {
		AESkey = 32,
		AESiv = 16
	};

	static QByteArray encryptAES_ECB(const QByteArray &data, const QByteArray &key);
	static QByteArray decryptAES_ECB(const QByteArray &cipher, const QByteArray &key);
	static QByteArray encryptAES_CBC(const QByteArray &data, const QByteArray &key, const QByteArray &iv);
	static QByteArray decryptAES_CBC(const QByteArray &cipher, const QByteArray &key, const QByteArray &iv);
	static QByteArray encryptRSA(const QByteArray &data, const QString &publicKeyBase64);
	static QByteArray decryptRSA(const QByteArray &cipher, const QString &privateKeyBase64);
	static QByteArray generateBlock(BlockSize size);
	static QByteArray fileChecksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm);
	static QByteArray beautifyBase64(const QByteArray &input);

protected:
	static QByteArray loadKey(const QString &source);
};

#endif // CRYPTOUTILS_H