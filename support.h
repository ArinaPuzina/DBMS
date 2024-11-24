#ifndef SUPPORT_H_INCLUDED
#define SUPPORT_H_INCLUDED
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <regex>
#include "readingCons.h"
#include "vector.h"
using namespace std;

namespace fs = filesystem;

regex pageRegex(".+\\d+\\.csv$");
void writeList(string path, string tableName, Vector<string> cols) {
    ofstream file(path);

    file << tableName << "_pk,";
    for (int i = 0; i < cols.size(); i++) {
        if (i == cols.size() - 1) {
            file << cols.get(i);
        } else {
            file << cols.get(i) << ",";
        }
    }
    file << endl;
    file.close();
    }
//���� � ������ csv
Vector<string> getCSVFromDir(string dirPath) {
    Vector<string> csvFiles;
    smatch match;
    for (const auto & entry : fs::directory_iterator(dirPath)) {
            string filepath = entry.path().string();
            if (regex_match(filepath, match, pageRegex)) {
                csvFiles.pushBack(filepath);

            }
    }
    return csvFiles;
}
Vector<string> listCSVFiles(const string& filepath) {
    Vector<string> csvFiles;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
            if (entry.path().extension() == ".csv") {
                string fullPath = entry.path().generic_string();
                fullPath.erase(std::remove(fullPath.begin(), fullPath.end(), ' '), fullPath.end());
                csvFiles.pushBack(fullPath);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        cerr << "������ ������� � ������: " << e.what() << endl;
    }
    return csvFiles;
}

//���������� ���������
string readCSVHeader(const string& pathToTable) {
    Vector<string> csvFiles = listCSVFiles(pathToTable);
    if (csvFiles.size() == 0) {
        throw runtime_error("�� ������� ������ CSV � ����������: " + pathToTable);
    }

    ifstream file(csvFiles.get(0));
    if (!file.is_open()) {
        throw runtime_error("�� ������� ������� ����: " + csvFiles.get(0));
    }

    string header;
    getline(file, header);
    file.close();
    return header;
}
//������� ������ ������� �� �����
int findColumnIndex(const string& tabCol, const string& pathToTable) {
    size_t dotPos = tabCol.find('.');
    if (dotPos == string::npos) {
        throw invalid_argument("������������ ������ tabCol, ��������� table.column: " + tabCol);
    }
    string columnName = tabCol.substr(dotPos + 1);

    string header = readCSVHeader(pathToTable);
    Vector<string> columns = split(header, ",");
    for (int i = 0; i < columns.size(); ++i) {
        if (columns.get(i) == columnName) {
            return i;
        }
    }

    throw runtime_error("������� '" + columnName + "' �� ������� � �����: " + pathToTable);
}
Vector<string> readAllDataRowsFromCSV(const string& pathToTable, string& header) {
    Vector<string> dataRows;
    Vector<string> csvFiles = listCSVFiles(pathToTable);

    bool skipHeader = true;  // ���������� ������ ������ ������ � ������ �����

    for (const auto& filePath : csvFiles) {
        ifstream file(filePath);
        if (!file.is_open()) {
            throw runtime_error("�� ������� ������� ����: " + filePath);
        }

        string line;
        bool isFirstLine = skipHeader;
        while (getline(file, line)) {
            if (isFirstLine) {
                header = line;  // ��������� ��������� �� ������� �����
                isFirstLine = false;
                continue;
            }
            dataRows.pushBack(line);
        }
        file.close();
        skipHeader = false;
    }
    return dataRows;
}

void writeDataToCSV(const string& pathToTable, const string& header, const Vector<string>& dataRows) {
    string pagePath = pathToTable + "/1.csv";
    pagePath.erase(std::remove(pagePath.begin(), pagePath.end(), ' '), pagePath.end());
    ofstream outFile(pagePath);
    if (!outFile.is_open()) {
        throw runtime_error("�� ������� ������� ���� ��� ������: " + pagePath);
    }

    outFile << header << endl;
    for (int i = 0; i < dataRows.size(); ++i) {
        outFile << dataRows.get(i) << endl;
    }
    outFile.close();
}
//������� ��� ������ CSV-����� � ��������� ������ �����
Vector<Vector<string>> readCSV(string filename) {
    Vector<Vector<string>> data;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return data;
    }

    string line;
    while (getline(file, line)) {
        Vector<string> row;
        stringstream lineStream(line);
        string cell;

        while (getline(lineStream, cell, ',')) {
            row.pushBack(cell);
        }

        data.pushBack(row);
    }

    file.close();
    return data;
}
//��������� csv
void writeCSV(const string& filename, const Vector<Vector<string>>& data) {
    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    for (int i = 0; i < data.size(); i++) {
        Vector<string> row = data.get(i);
        for (size_t i = 0; i < row.size(); ++i) {
            file << row.get(i);
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}
#endif // SUPPORT_H_INCLUDED
