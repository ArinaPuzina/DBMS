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

//������ � ������� JSON ����� � ��������� Schema
Schema readSchemaFromFile(const string& filePath) {
    ifstream inputFile(filePath);//o�������� ����
    if (!inputFile.is_open()) {
        throw runtime_error("Unable to open file: " + filePath);
    }

    json j;//������ � JSON � ��� ����� ������, �������������� � ���� ��� ����� � ��������
    inputFile >> j;//c�������� JSON

    Schema schema;
    schema.name = j["name"];
    schema.structure = j["structure"].get<map<string, vector<string>>>();
    schema.tuplesLimit = j["tuples_limit"];
    return schema;
}

//����������� ��������� �����
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