#include "vector.h"
#include <fstream>
#include <filesystem>
#include "hashTab.h"
#include "json.hpp"
#include "readingCons.h"
#include "Schema.h"
#include "func.h"
using json = nlohmann::json;
namespace fs = std::filesystem;
Schema schema;

// Функция для чтения JSON файла
Schema readSchemaFromFile(const string& filePath) {
    ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        throw runtime_error("Unable to open file: " + filePath);
    }

    json j;
    inputFile >> j;

    Schema schema;
    schema.name = j["name"];
    schema.tuplesLimit = j["tuples_limit"];
    auto structure = j["structure"];
    for (auto it = structure.begin(); it != structure.end(); it++) {
        Vector<string> columns;
        for (const string& column : it.value()) {
            columns.pushBack(column);
        }
        schema.structure.put(it.key(), columns);
    }
    return schema;
}


void createSchemaDirectories() {
    // Создаем директорию с именем схемы
    fs::create_directory(schema.name);
     Vector<string> tables = schema.structure.keys();//вектор с именами таблиц
        for (int i = 0; i < tables.size(); i++) {
            string tableName = tables.get(i);
            Vector<string> cols = schema.structure.get(tableName);
            string tablePath = schema.name + "/" + tableName; // Путь до таблицы
            if (!fs::exists(tablePath)) {
                fs::create_directory(tablePath);
                       } else {
                cerr << "Table directory " << tablePath << " already exists" << endl;
            }
            // Создаем файл 1.csv в каждой директории таблицы
            string csvFilePath = tablePath + "/1.csv";
            std::ofstream csvFile(csvFilePath);
            if (csvFile.is_open()) {
                // Добавляем заголовки колонок в CSV файл
                csvFile << tableName << "_pk,";
                for (size_t j = 0; j < cols.size(); j++) {
                    csvFile << cols.data[j] << ",";
                }
                csvFile << endl;
                csvFile.close();
            }

            // Создаем файл для первичного ключа
            string pkFilePath = tablePath + "/" + tableName + "_pk_sequence";
            std::ofstream pkFile(pkFilePath);
            if (pkFile.is_open()) {
                pkFile << "1"; // Начальное значение первичного ключа
                pkFile.close();
            }

            // Создаем файл блокировки
            string lockFilePath = tablePath + "/" + tableName + "_lock";
            std::ofstream lockFile(lockFilePath);
            lockFile.close();
        }
    }


int main() {
    try {
    cout << "Reading schema from JSON file..." << endl;
    schema = readSchemaFromFile("scheme.json");

    cout<<"Creating directories and files..."<<endl;
        createSchemaDirectories();

    cout << "directories and files created successfully." << endl;
    }
    catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
    }
    cout<<"DBMS is ready for using "<<endl;
    while(true){
        cout << "Enter SQL request: ";
        string command;
        getline(cin, command);

    // Разбор команды
    menu(command);
}



    return 0;
}
