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
        cerr << "Ошибка доступа к файлам: " << e.what() << endl;
    }
    return csvFiles;
}
string findColumnIndex(const string& tabCol) {
    size_t pos = tabCol.find("column");  // Находим позицию слова "column"
    if (pos != string::npos) {
        return tabCol.substr(pos + 6);  // Возвращаем подстроку после "column"
    }
    return "-1";  // Если "column" не найдено, возвращаем пустую строку
}

string readCSVHeader(const string& pathToTable) {
    Vector<string> csvFiles = listCSVFiles(pathToTable);
    if (csvFiles.size() == 0) {
        throw runtime_error("Не найдено файлов CSV в директории: " + pathToTable);
    }

    ifstream file(csvFiles.get(0));
    if (!file.is_open()) {
        throw runtime_error("Не удалось открыть файл: " + csvFiles.get(0));
    }

    string header;
    getline(file, header);  // Читаем первую строку как заголовок
    file.close();
    return header;
}
Vector<string> readAllDataRowsFromCSV(const string& pathToTable, string& header) {
    Vector<string> dataRows;
    Vector<string> csvFiles = listCSVFiles(pathToTable);

    bool skipHeader = true;  // Пропустить первую строку только в первом файле

    for (const auto& filePath : csvFiles) {
        ifstream file(filePath);
        if (!file.is_open()) {
            throw runtime_error("Не удалось открыть файл: " + filePath);
        }

        string line;
        bool isFirstLine = skipHeader;
        while (getline(file, line)) {
            if (isFirstLine) {
                header = line;  // Сохраняем заголовок из первого файла
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
        throw runtime_error("Не удалось открыть файл для записи: " + pagePath);
    }

    outFile << header << endl;
    for (int i = 0; i < dataRows.size(); ++i) {
        outFile << dataRows.get(i) << endl;
    }
    outFile.close();
}
#endif // SUPPORT_H_INCLUDED
