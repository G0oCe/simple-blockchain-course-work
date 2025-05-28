// author: tko
#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <sstream>
#include <climits>

void print_hex(const char *label, const uint8_t *v, size_t len) {
    size_t i;

    printf("%s: ", label);
    for (i = 0; i < len; ++i) {
        printf("%02x", v[i]);
    }
    printf("\n");
}

string getMerkleRoot(const vector<string> &merkle) {
    printf("\nFinding Merkle Root.... \n");
    if (merkle.empty())
        return "";
    else if (merkle.size() == 1){
        return sha256(merkle[0]);
    }

    vector<string> new_merkle = merkle;

    while (new_merkle.size() > 1) {
        if ( new_merkle.size() % 2 == 1 )
            new_merkle.push_back(merkle.back());

        vector<string> result;

        for (int i=0; i < new_merkle.size(); i += 2){
            string var1 = sha256(new_merkle[i]);
            string var2 = sha256(new_merkle[i+1]);
            string hash = sha256(var1+var2);
            // printf("---hash(hash(%s), hash(%s)) => %s\n",new_merkle[0].c_str(),new_merkle[1].c_str(),hash.c_str());
            result.push_back(hash);
        }
        new_merkle = result;
    }
    return new_merkle[0];

}
pair<string,string> findHash(int index, string prevHash, vector<string> &merkle) {
    string header = to_string(index) + prevHash + getMerkleRoot(merkle);
    unsigned int nonce;
    for (nonce = 0; nonce < 100000; nonce++ ) {
        string blockHash = sha256(header + to_string(nonce));
        if (blockHash.substr(0,2) == "00"){
            // cout << "nonce: " << nonce;
            // cout << "header: " << header;
            return make_pair(blockHash,to_string(nonce));
            break;
        }
    }
    return make_pair("fail","fail");
}

// Function to generate a random ASCII string of a given length
std::string generateRandomStringWithCharset(size_t length, std::string CHARACTERS) {
// In your C++ data generation code
    //const std::string CHARACTERS = "012345";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.length() - 1);

    std::string random_string;
    random_string.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }
    return random_string;
}

// Function to generate a random entropy value (e.g., as a string)
std::string generateEntropyValue() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned long long> distrib(0, ULLONG_MAX);
    return std::to_string(distrib(gen));
}

std::string generateP2P(
        size_t senderLen, const std::string& addrChars,
        size_t receiverLen,
        size_t amountLen, const std::string& amountChars
) {
    std::string sender = generateRandomStringWithCharset(senderLen, addrChars);
    std::string receiver = generateRandomStringWithCharset(receiverLen, addrChars);
    std::string amount = generateRandomStringWithCharset(amountLen, amountChars);
    return sender + receiver + amount;
}
#endif