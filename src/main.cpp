// author: tko (упрощён для локального использования)
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

#include "hash.hpp"
#include "Block.hpp"
#include "common.hpp"
#include "BlockChain.hpp"
// убраны: requests.hpp, json.hh, client_http.hpp, server_http.hpp

using namespace std;

/*
 * Main function - простой CLI для работы с блокчейном
 */
int main() {
    printf("Добро пожаловать в локальную версию блокчейна! Для выхода — Ctrl+C\n");

    BlockChain bc;
    bc = BlockChain(0); // создаём genesis-блок

    // CLI — ввод и просмотр блоков
    for (int i = 0; i < 20; i++) {
        vector<string> v;
        int temp;

        printf("\n(1) Посмотреть блок\n(2) Добавить блок\n");
        int valid = scanf("%d", &temp);

        if ((valid == 1) && (temp == 1)) {
            printf("Введите индекс блока для просмотра: ");
            scanf("%d", &temp);
            try {
                bc.getBlock(temp).toString();
            } catch (const exception& e) {
                cout << e.what() << endl;
            }
        }
        else if (temp == 2) {
            char tmp[201];
            printf("\nДОБАВЛЕНИЕ БЛОКА\nВведите сообщение: ");
            scanf("%200s", tmp);
            string str = tmp;
            printf("Введено: '%s'\n", str.c_str());
            v.push_back(str);

            int in;
            printf("Введите любое число, чтобы подтвердить добавление: ");
            scanf("%d", &in);

            try {
                auto pair = findHash(bc.getNumOfBlocks(), bc.getLatestBlockHash(), v);
                bc.addBlock(bc.getNumOfBlocks(), bc.getLatestBlockHash(), pair.first, pair.second, v);
            } catch (const exception& e) {
                cout << e.what() << "\n" << endl;
            }
        }
    }

    printf("\nРабота завершена.\n");
    return 0;
}
