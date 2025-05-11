//author: tko
#ifndef BLOCK_H
#define BLOCK_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include "common.hpp"

#include "json.hh"
using json = nlohmann::json;

class Block {
public:
    Block(int index, string prevHash, string hash, string nonce, vector<string> data, std::string signature);
    Block(int index, string prevHash, string hash, string nonce, vector<string> data, std::string signature, std::string header);

    string getPreviousHash(void);
    string getHash(void);
    int getIndex(void);
    vector<string> getData(void);
    std::string getSignature(void);
    string getHeader(void);

    void toString(void);
    json toJSON(void);
private:
    int index;
    string previousHash;
    string blockHash;
    string nonce;
    vector<string> data;
    std::string signature;
    std::string header;
};

// Constructor (обычный, создаёт header сам)
Block::Block(int index, string prevHash, string hash, string nonce, vector<string> data, std::string signature) {
    printf("\nInitializing Block: %d ---- Hash: %s \n", index, hash.c_str());
    this->index = index;
    this->previousHash = prevHash;
    this->blockHash = hash;
    this->nonce = nonce;
    this->data = data;
    this->signature = signature;
    this->header = std::to_string(index) + prevHash + getMerkleRoot(data) + nonce;
}

// Constructor с заданным header (например, при загрузке из JSON)
Block::Block(int index, string prevHash, string hash, string nonce, vector<string> data, std::string signature, std::string header) {
    printf("\nInitializing Block: %d ---- Hash: %s \n", index, hash.c_str());
    this->index = index;
    this->previousHash = prevHash;
    this->blockHash = hash;
    this->nonce = nonce;
    this->data = data;
    this->signature = signature;
    this->header = header;
}

int Block::getIndex(void) {
    return this->index;
}

string Block::getPreviousHash(void) {
    return this->previousHash;
}

string Block::getHash(void) {
    return this->blockHash;
}

vector<string> Block::getData(void){
    return this->data;
}

string Block::getSignature(void) {
    return this->signature;
}

string Block::getHeader(void) {
    return this->header;
}

// Prints Block data
void Block::toString(void) {
    string dataString;
    for (int i=0; i < data.size(); i++)
        dataString += data[i] + ", ";
    printf("\n-------------------------------\n");
    printf("Block %d\nHash: %s\nPrevious Hash: %s\nContents: %s",
           index,this->blockHash.c_str(),this->previousHash.c_str(),dataString.c_str());
    printf("\n-------------------------------\n");
}

json Block::toJSON(void) {
    json j;
    j["index"] = this->index;
    j["hash"] = this->blockHash;
    j["previousHash"] = this->previousHash;
    j["nonce"] = this->nonce;
    j["data"] = this->data;
    j["signature"] = this->signature;
    j["header"] = this->header;
    return j;
}

#endif
