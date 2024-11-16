#include "vector.h"
#include <fstream>
#include <filesystem>
#include "hashTab.h"
#include "json.hpp"
#include "readingCons.h"
#include "Schema.h"
#include "func.h"
#include <locale>

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace std;
Schema schema;

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
    fs::create_directory(schema.name);
    Vector<string> tables = schema.structure.keys();
    for (int i = 0; i < tables.size(); i++) {
        string tableName = tables.get(i);
        Vector<string> cols = schema.structure.get(tableName);
        string tablePath = schema.name + "/" + tableName;
        if (!fs::exists(tablePath)) {
            fs::create_directory(tablePath);
        } else {
            cerr << "Table directory " << tablePath << " already exists" << endl;
        }

        // Создание файла 1.csv, если его ещё нет
        string csvFilePath = tablePath + "/1.csv";
        if (!fs::exists(csvFilePath)) {
            ofstream csvFile(csvFilePath);
            if (csvFile.is_open()) {
                csvFile << tableName << "_pk,";
                for (size_t j = 0; j < cols.size(); j++) {
                    csvFile << cols.data[j] << ",";
                }
                csvFile << endl;
                csvFile.close();
            }
        }

        // Создание файла _pk_sequence, если его ещё нет
        string pkFilePath = tablePath + "/" + tableName + "_pk_sequence";
        if (!fs::exists(pkFilePath)) {
            ofstream pkFile(pkFilePath);
            if (pkFile.is_open()) {
                pkFile << "1"; // Начальная последовательность
                pkFile.close();
            }
        }

        // Создание файла _lock, если его ещё нет
        string lockFilePath = tablePath + "/" + tableName + "_lock";
        if (!fs::exists(lockFilePath)) {
            ofstream lockFile(lockFilePath);
            lockFile.close();
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    try {

        cout << "Reading schema from JSON file..." << endl;
        schema = readSchemaFromFile("scheme.json");
        cout << "Creating directories and files..." << endl;
        createSchemaDirectories();

        cout << "directories and files created successfully." << endl;
    }
    catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
    }
    cout << "DBMS is ready for using " << endl;
    while (true) {
        cout << "Enter SQL request: ";
        string command;
        getline(cin, command);


        menu(command);
    }

    return 0;
}
