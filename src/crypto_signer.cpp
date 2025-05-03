#include "crypto_signer.hpp"

#include <cryptopp/osrng.h>
#include <cryptopp/pssr.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <filesystem>
#include <sstream>

crypto_signer::crypto_signer(const std::string& keyDir, unsigned int keySize)
        : keyDirectory(keyDir), rsaKeySize(keySize) {
    loadOrGenerateKeys();
}

void crypto_signer::loadOrGenerateKeys() {
    using namespace CryptoPP;
    AutoSeededRandomPool rng;

    std::string privPath = keyDirectory + "private.pem";
    std::string pubPath = keyDirectory + "public.pem";

    bool keysExist = std::filesystem::exists(privPath) && std::filesystem::exists(pubPath);
    bool needToRegenerate = false;

    if (keysExist) {
        try {
            FileSource privateFile(privPath.c_str(), true);
            privateKey.BERDecode(privateFile);

            FileSource publicFile(pubPath.c_str(), true);
            publicKey.BERDecode(publicFile);

            // Проверяем длину ключа (в битах)
            if (privateKey.GetModulus().BitCount() != rsaKeySize) {
                std::cout << "Обнаружено несоответствие длины ключа. Создаем новые ключи...\n";
                needToRegenerate = true;
            }

        } catch (const std::exception& e) {
            std::cerr << "Ошибка при загрузке ключей: " << e.what() << "\n";
            needToRegenerate = true;
        }
    } else {
        needToRegenerate = true;
    }

    if (needToRegenerate) {
        privateKey.GenerateRandomWithKeySize(rng, rsaKeySize);
        publicKey = RSA::PublicKey(privateKey);

        FileSink privateFile(privPath.c_str());
        privateKey.DEREncode(privateFile);
        privateFile.MessageEnd();

        FileSink publicFile(pubPath.c_str());
        publicKey.DEREncode(publicFile);
        publicFile.MessageEnd();

        std::cout << "Новые ключи с длиной " << rsaKeySize << " бит сохранены.\n";
    }
}

std::string crypto_signer::sign(const std::string& message) const {
    using namespace CryptoPP;

    AutoSeededRandomPool rng;
    RSASS<PKCS1v15, SHA256>::Signer signer(privateKey);
    std::string signature, encoded;

    // Подпись
    StringSource(message, true,
                 new SignerFilter(rng, signer,
                                  new StringSink(signature)
                 )
    );

    // Кодирование в base64
    StringSource(signature, true,
                 new Base64Encoder(
                         new StringSink(encoded),
                         false  // без символов новой строки
                 )
    );

    return encoded;
}

bool crypto_signer::verify(const std::string& message, const std::string& base64_signature) const {
    using namespace CryptoPP;

    try {
        RSASS<PKCS1v15, SHA256>::Verifier verifier(publicKey);

        std::string decoded_signature;
        // Декодируем base64-подпись
        StringSource(base64_signature, true,
                     new Base64Decoder(
                             new StringSink(decoded_signature)
                     )
        );

        // Проверка подписи
        StringSource(
                decoded_signature + message,
                true,
                new SignatureVerificationFilter(
                        verifier, nullptr,
                        SignatureVerificationFilter::THROW_EXCEPTION |
                        SignatureVerificationFilter::SIGNATURE_AT_BEGIN
                )
        );

        return true;
    } catch (const CryptoPP::Exception& e) {
        return false;
    }
}
