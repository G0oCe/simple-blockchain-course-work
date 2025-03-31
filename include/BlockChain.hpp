#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include "Block.hpp"
#include "hash.hpp"
#include "crypto_signer.hpp"
#include "json.hh"

using json = nlohmann::json;

class BlockChain {
public:
    BlockChain(crypto_signer& signer, int genesis = 1);
    Block getBlock(int index);
    int getNumOfBlocks(void);
    int addBlock(int index, std::string prevHash, std::string hash, std::string nonce, std::vector<std::string>& merkle);
    std::string getLatestBlockHash(void);
    std::string toJSON(void);
    int replaceChain(json chain);

private:
    std::vector<std::unique_ptr<Block>> blockchain;
    crypto_signer& signer;  // Ссылка на класс подписи
};

BlockChain::BlockChain(crypto_signer& signer, int genesis) : signer(signer) {
    if (genesis == 0) {
        std::vector<std::string> v = {"Genesis Block!"};
        auto hash_nonce_pair = findHash(0, "00000000000000", v);
        std::string signature = signer.sign(hash_nonce_pair.first);  // подписываем хеш
        blockchain.push_back(std::make_unique<Block>(0, "00000000000000", hash_nonce_pair.first, hash_nonce_pair.second, v, signature));
        std::cout << "Created blockchain!\n";
    }
}

Block BlockChain::getBlock(int index) {
    for (const auto& block : blockchain) {
        if (block->getIndex() == index) {
            return *block;
        }
    }
    throw std::invalid_argument("Index does not exist.");
}

int BlockChain::getNumOfBlocks(void) {
    return blockchain.size();
}

int BlockChain::addBlock(int index, std::string prevHash, std::string hash, std::string nonce, std::vector<std::string>& merkle) {
    std::string header = std::to_string(index) + prevHash + getMerkleRoot(merkle) + nonce;
    if (sha256(header) == hash && hash.substr(0,2) == "00" && index == blockchain.size()) {
        std::string signature = signer.sign(hash);  // Подписываем хэш блока
        blockchain.push_back(std::make_unique<Block>(index, prevHash, hash, nonce, merkle, signature));
        std::cout << "Block hashes match --- Adding Block " << hash << "\n";
        return 1;
    }
    std::cout << "Hash doesn't match criteria\n";
    return 0;
}

std::string BlockChain::getLatestBlockHash(void) {
    return blockchain.back()->getHash();
}

std::string BlockChain::toJSON() {
    json j;
    j["length"] = blockchain.size();
    for (const auto& block : blockchain) {
        j["data"][block->getIndex()] = block->toJSON();
    }
    return j.dump(3);
}

int BlockChain::replaceChain(json chain) {
    while (blockchain.size() > 1) {
        blockchain.pop_back();
    }
    for (int a = 1; a < chain["length"].get<int>(); a++) {
        auto block = chain["data"][a];
        std::vector<std::string> data = block["data"].get<std::vector<std::string>>();
        std::string signature = block["signature"];
        blockchain.push_back(std::make_unique<Block>(
                block["index"], block["previousHash"], block["hash"], block["nonce"], data, signature
        ));
    }
    return 1;
}

#endif
