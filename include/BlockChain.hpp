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
    int addBlock(int index, std::string prevHash, std::string nonce, std::vector<std::string>& merkle);
    std::string getLatestBlockHash(void);
    std::string toJSON(void);
    int replaceChain(json chain, bool verify);

private:
    std::vector<std::unique_ptr<Block>> blockchain;
    crypto_signer& signer;  // Ссылка на класс подписи
};

BlockChain::BlockChain(crypto_signer& signer, int genesis) : signer(signer) {
    if (genesis == 0) {
        std::vector<std::string> v = {"Genesis Block!"};

        // Создаём nonce и header, как для обычных блоков
        auto hash_nonce_pair = findHash(0, "00000000000000", v);

        std::string header = std::to_string(0) + "00000000000000" + getMerkleRoot(v) + hash_nonce_pair.second;
        std::string hash = sha256(header);
        std::string signature = signer.sign(header);

        blockchain.push_back(std::make_unique<Block>(
                0,
                "00000000000000",
                hash,
                hash_nonce_pair.second,
                v,
                signature
        ));

        std::cout << "✅ Genesis блок создан корректно\n";
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

// checks whether data fits with the right hash -> add block
int BlockChain::addBlock(int index, std::string prevHash, std::string nonce, std::vector<std::string>& merkle) {
    std::string header = std::to_string(index) + prevHash + getMerkleRoot(merkle) + nonce;
    std::string hash = sha256(header);

    // Проверка: валидность хэша и правильная позиция
    if (hash.substr(0, 2) == "00" && index == blockchain.size()) {
        std::string signature = signer.sign(header);

        blockchain.push_back(std::make_unique<Block>(index, prevHash, hash, nonce, merkle, signature));

        // Автоматическое сохранение в файл
        std::ofstream out("../blockchain.json");
        out << this->toJSON();
        out.close();
        
        std::cout <<"Block hashes match --- Adding Block "<< hash << std::endl;
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

int BlockChain::replaceChain(json chain, bool verify) {
    //remove all blocks except for the first block
    while (blockchain.size() > 1) {
        blockchain.pop_back();
    }

    for (int a = 1; a < chain["length"].get<int>(); a++) {
        auto block = chain["data"][a];
        std::vector<std::string> data = block["data"].get<std::vector<std::string>>();
        std::string header = std::to_string(block["index"].get<int>()) +
                             block["previousHash"].get<std::string>() +
                             getMerkleRoot(data) +
                             block["nonce"].get<std::string>();

        std::string signature = block["signature"];

        if (verify) {
            if (!signer.verify(header, signature)) {
                std::cout << "Подпись блока " << block["index"] << " некорректна! Отмена загрузки.\n";
                return 0;
            }
        }


        blockchain.push_back(std::make_unique<Block>(
                block["index"], block["previousHash"], block["hash"],
                block["nonce"], data, signature
        ));
    }

    std::cout << "✅ Цепочка успешно заменена на полученную из файла\n";
    return 1;
}


#endif
