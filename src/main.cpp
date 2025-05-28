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

#include "hash.hpp"
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
        std::cout << "Blockchain generation mode activated.\n";

        std::ifstream gen_settings_file("../gen_settings.json");
        if (!gen_settings_file.is_open()) {
            std::cerr << "Не удалось открыть gen_settings.json\n";
            return 1;
        }
        nlohmann::json gen_config;
        gen_settings_file >> gen_config;
        gen_settings_file.close();

        // Чтение параметров генерации
        int data_length = gen_config.value("dataLength", 5);
        int block_count = gen_config.value("blockCount", 10);
        int sender_len = gen_config.value("senderAddressLength", 10);
        int receiver_len = gen_config.value("receiverAddressLength", 10);
        int amount_len = gen_config.value("amountLength", 5);
        std::string addr_charset = gen_config.value("addressCharSet", "0123456789abcdef");
        std::string amount_charset = gen_config.value("amountCharSet", "0123456789");
        bool anon = gen_config.value("anon", false); // Читаем флаг 'anon'

        std::unique_ptr<BlockChain> generated_bc; // Объявляем unique_ptr здесь
        std::string output_filename;

        if (anon) {
            std::cout << "Generating ANONYMOUS blockchain...\n";
            std::cout << "Settings: Sender=" << sender_len << ", Receiver=" << receiver_len << ", Amount=" << amount_len << "\n";

            std::string genesis_data = generateP2P(sender_len, addr_charset, receiver_len, amount_len, amount_charset);
            generated_bc = std::make_unique<BlockChain>(signer, 0, genesis_data);

            for (int i = generated_bc->getNumOfBlocks() - 1; i < block_count - 1; ++i) {
                std::vector<std::string> block_data;
                block_data.push_back(generateP2P(sender_len, addr_charset, receiver_len, amount_len, amount_charset));
                block_data.push_back(generateEntropyValue());

                std::string prev_hash = generated_bc->getLatestBlockHash();
                int next_block_index = generated_bc->getNumOfBlocks();
                auto pair_hash_nonce = findHash(next_block_index, prev_hash, block_data);

                if (pair_hash_nonce.first == "fail") {
                    std::cerr << "Failed to find hash for block " << next_block_index << ".\n"; break;
                }
                generated_bc->addBlock(next_block_index, prev_hash, pair_hash_nonce.second, block_data);
                std::cout << "Generated and added block " << next_block_index << "\n";
            }

            std::ostringstream filename_s;
            filename_s << "../generated/blockchain_anon_" << block_count << ".json";
            output_filename = filename_s.str();

        } else { // Если не анонимный режим
            std::cout << "Generating REGULAR blockchain...\n";
            std::cout << "Settings: DataLength=" << data_length << "\n";

            std::string genesis_data = generateRandomStringWithCharset(data_length, addr_charset); // Используем addr_charset для не-анонимных данных
            generated_bc = std::make_unique<BlockChain>(signer, 0, genesis_data);

            for (int i = generated_bc->getNumOfBlocks() - 1; i < block_count - 1; ++i) {
                std::vector<std::string> block_data;
                block_data.push_back(generateRandomStringWithCharset(data_length, addr_charset)); // Исправлено
                block_data.push_back(generateEntropyValue());

                std::string prev_hash = generated_bc->getLatestBlockHash();
                int next_block_index = generated_bc->getNumOfBlocks();
                auto pair_hash_nonce = findHash(next_block_index, prev_hash, block_data);

                if (pair_hash_nonce.first == "fail") {
                    std::cerr << "Failed to find hash for block " << next_block_index << ".\n"; break;
                }
                generated_bc->addBlock(next_block_index, prev_hash, pair_hash_nonce.second, block_data);
                std::cout << "Generated and added block " << next_block_index << "\n";
            }

            std::ostringstream filename_s;
            filename_s << "../generated/blockchain_" << data_length << "_" << block_count << ".json";
            output_filename = filename_s.str();
        }

        // Сохранение блокчейна
        std::ofstream out_file(output_filename);
        if (out_file.is_open()) {
            out_file << generated_bc->toJSON();
            out_file.close();
            std::cout << "Generated blockchain saved to: " << output_filename << "\n";
        } else {
            std::cerr << "Failed to open file for saving: " << output_filename << "\n";
        }
        return 0; // Завершаем работу после генерации
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
