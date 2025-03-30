#pragma once//

#include <string>
#include <cryptopp/rsa.h>

class crypto_signer {
public:
    explicit crypto_signer(const std::string& keyDir, unsigned int keySize = 512);

    std::string sign(const std::string& message) const;
    bool verify(const std::string& message, const std::string& signature) const;

private:
    std::string keyDirectory;
    unsigned int rsaKeySize;
    CryptoPP::RSA::PrivateKey privateKey;
    CryptoPP::RSA::PublicKey publicKey;

    void loadOrGenerateKeys();
};


