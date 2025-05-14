// author: tko (упрощён для локального использования)
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <memory>

#include "hash.hpp"
#include "Block.hpp"
#include "common.hpp"
#include "BlockChain.hpp"
#include "json.hh"
#include "MyRSA.h"

int main() {
    // Чтение конфигурации
    std::ifstream config_file("../config.json");
    if (!config_file.is_open()) {
        std::cerr << "Не удалось открыть config.json\n";
        return 1;
    }

    nlohmann::json config;
    config_file >> config;

    unsigned int key_size = config["key_size"];
    std::string key_path = config["key_path"];
    bool verify_blocks = config.value("verify_blocks", false);

    // Инициализация и (при необходимости) генерация ключей
    MyRSA signer(key_path, key_size);

    bool generate_new_blockchain = config.value("generate", false); // Read the 'generate' flag

    if (generate_new_blockchain) {
        // std::cout << "Blockchain generation mode activated.\n";

        std::ifstream gen_settings_file("../gen_settings.json");
        if (!gen_settings_file.is_open()) {
             std::cerr << "Не удалось открыть gen_settings.json\n";
            return 1;
        }
        nlohmann::json gen_config;
        gen_settings_file >> gen_config;
        gen_settings_file.close();

        int data_length = gen_config.value("dataLength", 5); // Default to 5 if not found
        int block_count = gen_config.value("blockCount", 10); // Default to 10 if not found

        //std::cout << "Generating blockchain with " << block_count << " blocks, each with data length " << data_length << ".\n";

        BlockChain generated_bc(signer, 0, generateRandomASCIIString(data_length));

        if (block_count == 0) {
            //std::cout << "Block count is 0. No blocks will be generated beyond potential implicit genesis.\n";
        } else if (generated_bc.getNumOfBlocks() == 1 && block_count == 1) {
            //std::cout << "Genesis block created. Total blocks target: 1.\n";
        }


        for (int i = generated_bc.getNumOfBlocks() - 1; i < block_count - 1; ++i) { // Start from current number of blocks
            std::vector<std::string> block_data;
            block_data.push_back(generateRandomASCIIString(data_length));
            block_data.push_back(generateEntropyValue()); // Add entropy

            std::string prev_hash = generated_bc.getLatestBlockHash();
            int next_block_index = generated_bc.getNumOfBlocks();

            auto pair_hash_nonce = findHash(next_block_index, prev_hash, block_data);

            if (pair_hash_nonce.first == "fail") {
                std::cerr << "Failed to find a suitable hash for block " << next_block_index << ". Stopping generation.\n";
                break;
            }

            generated_bc.addBlock(next_block_index, prev_hash, pair_hash_nonce.second, block_data);
            // std::cout << "Generated and added block " << next_block_index << "\n";
        }

        // Construct filename and save the generated blockchain
        std::ostringstream filename_s;
        filename_s << "../generated/blockchain_" << data_length << "_" << block_count << ".json";
        std::string output_filename = filename_s.str();

        std::ofstream out_file(output_filename);
        if (out_file.is_open()) {
            out_file << generated_bc.toJSON();
            out_file.close();
            // std::cout << "Generated blockchain saved to: " << output_filename << "\n";
        } else {
            std::cerr << "Failed to open file for saving generated blockchain: " << output_filename << "\n";
        }

        return 0;
    }

    std::cout << "Добро пожаловать в локальную версию блокчейна! Для выхода — Ctrl+C\n";

    std::unique_ptr<BlockChain> bc;

    // Проверяем наличие blockchain.json
    std::ifstream blockchain_input("../blockchain.json");
    if (blockchain_input) {
        nlohmann::json chain_json;
        blockchain_input >> chain_json;

        bc = std::make_unique<BlockChain>(signer, 0); // создаём, чтобы заменить цепочку
        if (bc->replaceChain(chain_json, verify_blocks)) {
            std::cout << "Загружен существующий блокчейн из файла.\n";
        } else {
            std::cerr << "Ошибка при валидации блокчейна из файла. Завершение.\n";
            return 1;
        }
    } else {
        // Файл не найден — создаём новый с genesis-блоком
        bc = std::make_unique<BlockChain>(signer, 0);
        std::cout << "Создан новый блокчейн с genesis-блоком.\n";
    }

    // CLI — ввод и просмотр блоков
    for (int i = 0; i < 20; ++i) {
        std::vector<std::string> v;
        int temp = 0;

        std::cout << "\n(1) Посмотреть блок\n(2) Добавить блок\n";
        int valid = std::scanf("%d", &temp);

        if ((valid == 1) && (temp == 1)) {
            std::cout << "Введите индекс блока для просмотра: ";
            std::scanf("%d", &temp);
            try {
                Block block = bc->getBlock(temp);
                block.toString();

                if (verify_blocks) {
                    std::string header = block.getHeader();
                    std::string hash = sha256(header);
                    std::string recomputedSig = signer.sign(hash);

                    std::cout << "hash при проверке: " << hash << "\n";
                    std::cout << "expected sig: " << block.getSignature() << "\n";
                    std::cout << "recomputed:   " << recomputedSig << "\n";

                    if (signer.verify(hash, block.getSignature())) {
                        std::cout << "✅ Подпись блока корректна\n";
                    } else {
                        std::cout << "❌ Подпись блока НЕ прошла\n";
                    }
                }

            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        } else if (temp == 2) {
            char tmp[201];
            std::cout << "\nДОБАВЛЕНИЕ БЛОКА\nВведите сообщение: ";
            std::scanf("%200s", tmp);
            std::string str(tmp);
            std::cout << "Введено: '" << str << "'\n";
            v.push_back(str);

            int in = 0;
            std::cout << "Введите любое число, чтобы подтвердить добавление: ";
            std::scanf("%d", &in);

            try {
                auto pair = findHash(bc->getNumOfBlocks(), bc->getLatestBlockHash(), v);
                bc->addBlock(bc->getNumOfBlocks(), bc->getLatestBlockHash(), pair.second, v);
            } catch (const std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
    }

    std::cout << "\nРабота завершена.\n";
    return 0;
}
