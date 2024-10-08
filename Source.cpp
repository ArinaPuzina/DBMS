#include <iostream>
#include <fstream>
#include "json.hpp"
#include <map>
#include <vector>
using json = nlohmann::json;
using namespace std;

struct Schema {
    string name;
    map<string, vector<string>> structure;
    int tuplesLimit;
};

//чтение и парсинг JSON файла в структуру Schema
Schema readSchemaFromFile(const string& filePath) {
    ifstream inputFile(filePath);//oткрываем файл
    if (!inputFile.is_open()) {
        throw runtime_error("Unable to open file: " + filePath);
    }

    json j;//объект в JSON Ч это набор данных, представленных в виде пар Ђключ Ч значениеї
    inputFile >> j;//cчитываем JSON

    Schema schema;
    schema.name = j["name"];
    schema.structure = j["structure"].get<map<string, vector<string>>>();
    schema.tuplesLimit = j["tuples_limit"];
    return schema;
}

//отображение структуры схемы
void printSchema(const Schema& schema) {
    cout << "Schema Name: " << schema.name << endl;
    cout << "Tuples Limit: " << schema.tuplesLimit << endl;
    cout << "Structure:" << endl;
    for (const auto& table : schema.structure) {
        cout << "Table: " << table.first << " Columns: ";
        for (const auto& column : table.second) {
            cout << column << " ";
        }
        cout << endl;
    }
}

int main() {
    setlocale(LC_ALL, "rus");
    try {
        Schema schema = readSchemaFromFile("scheme.json");
        printSchema(schema);
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
}