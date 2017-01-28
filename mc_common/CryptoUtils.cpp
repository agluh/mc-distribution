#include <QtCore\QString>
#include <QtCore\QByteArray>
#include <QtCore\QFile>
#include <QtCore\QCryptographicHash>

#include <modes.h>
#include <rsa.h>
#include <osrng.h>
#include <base64.h>
#include <hex.h>
#include <files.h>

#include "CryptoUtils.h"

QByteArray CryptoUtils::encryptAES_ECB(const QByteArray &data, const QByteArray &key) {
	std::string ciphertext;
	CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption encryption((const byte*)key.data(), key.length());

	CryptoPP::StringSource encryptor((const byte*)data.data(), data.size(), true,
		new CryptoPP::StreamTransformationFilter(encryption,
			new CryptoPP::Base64Encoder(
				new CryptoPP::StringSink(ciphertext), false
			),
			CryptoPP::StreamTransformationFilter::DEFAULT_PADDING
		)
	);

	QByteArray buff = QByteArray::fromStdString(ciphertext);
	return QByteArray::fromBase64(buff);
}

QByteArray CryptoUtils::decryptAES_ECB(const QByteArray &cipher, const QByteArray &key) {
	std::string decrypted;
	CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption decryption((const byte*)key.data(), key.length());

	CryptoPP::StringSource decryptor((const byte*)cipher.data(), cipher.size(), true,
		new CryptoPP::StreamTransformationFilter(decryption,
				new CryptoPP::StringSink(decrypted), CryptoPP::StreamTransformationFilter::DEFAULT_PADDING
		)
	);

	return QByteArray::fromStdString(decrypted);
}

QByteArray CryptoUtils::encryptAES_CBC(const QByteArray &data, const QByteArray &key, const QByteArray &iv) {
	std::string ciphertext;
	CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption((const byte*)key.data(), key.length(), (const byte*)iv.data());

	CryptoPP::StringSource encryptor((const byte*)data.data(), data.size(), true,
		new CryptoPP::StreamTransformationFilter(encryption,
			new CryptoPP::Base64Encoder(
				new CryptoPP::StringSink(ciphertext), false
			),
			CryptoPP::StreamTransformationFilter::DEFAULT_PADDING
		)
	);

	QByteArray buff = QByteArray::fromStdString(ciphertext);
	return QByteArray::fromBase64(buff);
}

QByteArray CryptoUtils::decryptAES_CBC(const QByteArray &cipher, const QByteArray &key, const QByteArray &iv) {
	std::string decrypted;
	CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption((const byte*)key.data(), key.length(), (const byte*)iv.data());

	CryptoPP::StringSource decryptor((const byte*)cipher.data(), cipher.size(), true,
		new CryptoPP::StreamTransformationFilter(decryption,
			new CryptoPP::StringSink(decrypted), CryptoPP::StreamTransformationFilter::DEFAULT_PADDING
		)
	);

	return QByteArray::fromStdString(decrypted);
}

QByteArray CryptoUtils::encryptRSA(const QByteArray &data, const QString &publicKeyBase64) {
	std::string ciphertext;
	CryptoPP::AutoSeededRandomPool rnd;
	CryptoPP::StringSource publicKeySrc(CryptoUtils::loadKey(publicKeyBase64), true, new CryptoPP::Base64Decoder);
	CryptoPP::RSAES_OAEP_SHA_Encryptor rsaEncryptor(publicKeySrc);

	CryptoPP::StringSource encryptor((const byte*)data.data(), data.size(), true,
		new CryptoPP::PK_EncryptorFilter(rnd, rsaEncryptor,
			new CryptoPP::Base64Encoder(
				new CryptoPP::StringSink(ciphertext), false
			)
		)
	);

	QByteArray buff = QByteArray::fromStdString(ciphertext);
	return QByteArray::fromBase64(buff);
}

QByteArray CryptoUtils::decryptRSA(const QByteArray &cipher, const QString &privateKeyBase64) {
	std::string decrypted;
	CryptoPP::AutoSeededRandomPool rnd;
	CryptoPP::StringSource privateKeySrc(CryptoUtils::loadKey(privateKeyBase64), true, new CryptoPP::Base64Decoder);
	CryptoPP::RSAES_OAEP_SHA_Decryptor rsaDecryptor(privateKeySrc);
		
	CryptoPP::StringSource decryptor((const byte*)cipher.data(), cipher.size(), true,
		new CryptoPP::PK_DecryptorFilter(rnd, rsaDecryptor, 
			new CryptoPP::StringSink(decrypted)
		)
	);
		
	return QByteArray::fromStdString(decrypted);
}

QByteArray CryptoUtils::loadKey(const QString &source) {
	QFile file(source);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QByteArray keyDump = file.readAll();
	file.close();
	return keyDump;
}

QByteArray CryptoUtils::generateBlock(BlockSize size) {
	CryptoPP::SecByteBlock block(0x00, size);
	CryptoPP::AutoSeededRandomPool rnd;
	rnd.GenerateBlock(block, block.size());

	return QByteArray((char*)block.BytePtr(), block.size());
}

QByteArray CryptoUtils::fileChecksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm) {
	QFile f(fileName);
	if (f.open(QFile::ReadOnly)) {
		QCryptographicHash hash(hashAlgorithm);
		if (hash.addData(&f)) {
			return hash.result();
		}
	}
	return QByteArray();
}

QByteArray CryptoUtils::beautifyBase64(const QByteArray &inArray) {
	const int inSize = inArray.size();
	const int outEst = inSize + (inSize / 32) + 4;
	QByteArray outArray;
	outArray.reserve(outEst);

	const char* inPtr = inArray.data();
	for (int inCrs = 0; inCrs <= inSize - 64; inCrs += 64)
	{
		outArray += QByteArray::fromRawData(inPtr + inCrs, 64);
		outArray += "\n";
	}

	const int rem = inSize % 64;
	if (rem > 0)
	{
		outArray += QByteArray::fromRawData(inPtr + inSize - rem, rem);
		outArray += "\n";
	}

	return (outArray);
}