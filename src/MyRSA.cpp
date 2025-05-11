#include "MyRSA.h"
#include <cryptopp/osrng.h>
#include <cryptopp/nbtheory.h>
#include <cryptopp/sha.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

using namespace CryptoPP;
using json = nlohmann::json;

// Вспомогательная функция: Integer → string
std::string intToString(const Integer& val) {
    std::ostringstream oss;
    oss << val;
    std::string s = oss.str();
    if (!s.empty() && s.back() == '.') s.pop_back();
    return s;
}

// Генерация простого числа
Integer generatePrime(int bits) {
    AutoSeededRandomPool rng;
    Integer candidate;
    do {
        candidate = Integer(rng, bits);
    } while (!IsPrime(candidate));
    return candidate;
}

// Конструктор
MyRSA::MyRSA(const std::string& keyDir, unsigned int bits)
        : keyDirectory(keyDir), rsaKeySize(bits) {
    loadOrGenerateKeys();
}

// Загрузка или генерация ключей
void MyRSA::loadOrGenerateKeys() {
    const std::string path = keyDirectory + "/rsa_key.json";
    bool needToRegenerate = true;

    if (std::filesystem::exists(path)) {
        try {
            if (loadFromJSON(path)) {
                std::ifstream file(path);
                json key;
                file >> key;
                unsigned int tmp = key["key_size"];

                if (tmp == rsaKeySize) {
                    needToRegenerate = false;
                } else {
                    std::cout << "🔄 Несоответствие длины ключа. Генерируем новые...\n";
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ Ошибка при загрузке ключей: " << e.what() << "\n";
        }
    }

    if (needToRegenerate) {
        p = generatePrime(rsaKeySize / 2);
        q = generatePrime(rsaKeySize / 2);
        n = p * q;
        phi = (p - 1) * (q - 1);
        e = Integer(65537);
        while (GCD(e, phi) != Integer::One()) e += 2;
        d = e.InverseMod(phi);

        saveToJSON(path, rsaKeySize);
        std::cout << "Новые ключи (" << rsaKeySize << " бит) сохранены в: " << path << "\n";
    }
}

// Сохранение ключей
void MyRSA::saveToJSON(const std::string& path, unsigned int key_size) const {
    json j = {
            {"key_size", key_size},
            {"p", intToString(p)},
            {"q", intToString(q)},
            {"n", intToString(n)},
            {"phi", intToString(phi)},
            {"e", intToString(e)},
            {"d", intToString(d)}
    };

    std::ofstream out(path);
    out << j.dump(4);
}

// Загрузка ключей
bool MyRSA::loadFromJSON(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return false;

    try {
        json j;
        in >> j;
        key_size = j["key_size"]; // TODO:: нормальное название дать, посоветуйся
        p = Integer(j["p"].get<std::string>().c_str());
        q = Integer(j["q"].get<std::string>().c_str());
        n = Integer(j["n"].get<std::string>().c_str());
        phi = Integer(j["phi"].get<std::string>().c_str());
        e = Integer(j["e"].get<std::string>().c_str());
        d = Integer(j["d"].get<std::string>().c_str());
        return true;
    } catch (...) {
        return false;
    }
}

// SHA256 → Integer → mod n
Integer MyRSA::hashToIntegerModN(const std::string& message) const {
    SHA256 hash;
    std::string digest;

    StringSource(message, true, new HashFilter(hash, new StringSink(digest)));
    return Integer(reinterpret_cast<const byte*>(digest.data()), digest.size()) % n;
}

// Подпись
std::string MyRSA::sign(const std::string& message) const {
    Integer msg = hashToIntegerModN(message);
    Integer signature = a_exp_b_mod_c(msg, d, n);

    std::string encoded;
    Base64Encoder encoder(new StringSink(encoded), false);
    signature.Encode(encoder, signature.MinEncodedSize());
    encoder.MessageEnd();

    return encoded;
}

// Проверка подписи
bool MyRSA::verify(const std::string& message, const std::string& base64Signature) const {
    std::string decoded;
    StringSource(base64Signature, true, new Base64Decoder(new StringSink(decoded)));

    Integer signature;
    signature.Decode(reinterpret_cast<const byte*>(decoded.data()), decoded.size());

    Integer expected = hashToIntegerModN(message);
    Integer recovered = a_exp_b_mod_c(signature, e, n);

    std::cout << "Ожидаемое:  " << expected << "\n";
    std::cout << "Полученное: " << recovered << "\n";

    return expected == recovered;
}
