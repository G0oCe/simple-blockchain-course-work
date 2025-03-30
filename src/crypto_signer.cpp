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

    if (std::filesystem::exists(privPath) && std::filesystem::exists(pubPath)) {
        FileSource privateFile(privPath.c_str(), true);
        privateKey.BERDecode(privateFile);

        FileSource publicFile(pubPath.c_str(), true);
        publicKey.BERDecode(publicFile);
    } else {
        privateKey.GenerateRandomWithKeySize(rng, rsaKeySize);
        publicKey = RSA::PublicKey(privateKey);

        FileSink privateFile(privPath.c_str());
        privateKey.DEREncode(privateFile);
        privateFile.MessageEnd();

        FileSink publicFile(pubPath.c_str());
        publicKey.DEREncode(publicFile);
        publicFile.MessageEnd();
    }
}

std::string crypto_signer::sign(const std::string& message) const {
    using namespace CryptoPP;
    AutoSeededRandomPool rng;

    RSASS<PKCS1v15, SHA256>::Signer signer(privateKey);
    std::string signature;

    StringSource(message, true,
                 new SignerFilter(rng, signer,
                                  new StringSink(signature)
                 )
    );

    return signature;
}

bool crypto_signer::verify(const std::string& message, const std::string& signature) const {
    using namespace CryptoPP;

    bool result = false;

    RSASS<PKCS1v15, SHA256>::Verifier verifier(publicKey);

    SignatureVerificationFilter verifierFilter(
            verifier,
            new ArraySink((byte*)&result, sizeof(result)),
            SignatureVerificationFilter::PUT_RESULT | SignatureVerificationFilter::SIGNATURE_AT_BEGIN
    );

    verifierFilter.Put(reinterpret_cast<const byte*>(signature.data()), signature.size());
    verifierFilter.Put(reinterpret_cast<const byte*>(message.data()), message.size());
    verifierFilter.MessageEnd();

    return result;
}
