#pragma once
#include <cryptopp/integer.h>
#include <string>
#include "json.hh"

class MyRSA {
private:
    CryptoPP::Integer p, q, n, phi, e, d;
    unsigned int rsaKeySize;
    unsigned int key_size;
    std::string keyDirectory;

    void loadOrGenerateKeys();
    void saveToJSON(const std::string& path, unsigned int key_size) const;
    bool loadFromJSON(const std::string& path);
    CryptoPP::Integer hashToIntegerModN(const std::string& message) const;

public:
    MyRSA(const std::string& keyDir, unsigned int bits);
    std::string sign(const std::string& message) const;
    bool verify(const std::string& message, const std::string& base64Signature) const;
};
