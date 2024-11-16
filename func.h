#ifndef FUNC_H_INCLUDED
#define FUNC_H_INCLUDED

#include "support.h"



void Insert(const string& tableName, const Vector<string>& values) {
    string pathToTable = schema.name + "/" + tableName;
    string pkFilePath = pathToTable + "/" + tableName + "_pk_sequence";
    ifstream pkInput(pkFilePath);
    int currentKey = 0;
    if (pkInput) {
        pkInput >> currentKey;
    }
    pkInput.close();
    currentKey++;

    int index = 1;
    string currentFile;
    while (true) {
        currentFile = pathToTable + "/" + to_string(index) + ".csv";
        ifstream csvCheck(currentFile);
        int lineCount = 0;
        string csvLine;
        while (getline(csvCheck, csvLine)) {
            lineCount++;
        }

        if (lineCount < schema.tuplesLimit) {
            break;
        }
        index++;
    }

    ofstream csvOutput(currentFile, ios::app);
    if (!csvOutput.is_open()) {
        throw runtime_error("Unable to open file: " + currentFile);
    }

    csvOutput << currentKey;
    for (size_t i = 0; i < values.size(); ++i) {
        csvOutput << "," << values.get(i);
    }
    csvOutput << endl;
    csvOutput.close();

    //cохран€ем обновленный первичный ключ
    ofstream pkOutput(pkFilePath);
    pkOutput << currentKey;
    pkOutput.close();
}

void fDelete(Vector<string> tables, string condition) {
        if (condition == "") {
            for (int i = 0; i < tables.size(); i++) {
                string tableName = tables.get(i);

                Vector<string> pages = getCSVFromDir(schema.name + "/" + tableName);
                for (int j = 0; j < pages.size(); j++) {
                    cout<<
                    fs::remove(pages.get(j));
                }

                string pagePath = schema.name + "/" + tableName + "/1.csv";
                writeList(pagePath, tableName, schema.structure.get(tableName));
            }
}
}


void DeleteWhere(const string& tableName, const Vector<string>& ConditionsOR) {
    string pathToTable = schema.name + "/" + tableName;
    string header;
    Vector<string> elementsInVec = readAllDataRowsFromCSV(pathToTable, header);
    if (elementsInVec.size() == 0) {
        cerr << "не удалось загрузить данные из файла." << endl;
        return;
    }

    Vector<string> elementsOfRows;

    for (int i = 0; i < elementsInVec.size(); i++) {
        elementsOfRows = split(elementsInVec.get(i), ",");//раздел€ем строку поэлементно
        bool rowMatchesOr = false;

        //разбиваем по энд
        for (int orIndex = 0; orIndex < ConditionsOR.size(); ++orIndex) {
            Vector<string> andConditions = split(ConditionsOR.get(orIndex), "AND");
            bool rowMatchesAnd = true;

            //по равно
            for (const auto& condition : andConditions) {
                Vector<string> conditionParts = split(condition, "=");
                if (conditionParts.size() != 2) continue;

                string column = conditionParts.get(0);
                string expectedValue = conditionParts.get(1);

                int columnIndex = stoi(findColumnIndex(column));
                if (columnIndex == -1) {
                    rowMatchesAnd = false;
                    break;//условие провалилось идем к следующему
                }

                if (expectedValue.front() == '\'' && expectedValue.back() == '\'') {//если колона сравниваетс€ со значением
                    string constantValue = expectedValue.substr(1, expectedValue.size() - 2);
                    if (elementsOfRows.get(columnIndex) != constantValue) {
                        rowMatchesAnd = false;
                        break;//переходим к следующему OR
                    }
                } else {
                    //с колоной
                    int compareIndex = stoi(findColumnIndex(expectedValue));
                    if (compareIndex == -1 || elementsOfRows.get(columnIndex) != elementsOfRows.get(compareIndex)) {
                        rowMatchesAnd = false;
                        break;//-//-
                    }
                }
            }

            if (rowMatchesAnd) {
                rowMatchesOr = true;//если выполнено AND, OR тоже выполнен
                break;
            }
        }

        //если строка соответствует услови€м OR
        if (rowMatchesOr) {
            elementsInVec.remove(i);
            --i;
        }
    }
    Vector<string> pages = listCSVFiles(pathToTable);
    for (int j = 0; j < pages.size(); j++) {
        fs::remove(pages.get(j));
    }
    writeDataToCSV(pathToTable, header, elementsInVec);
}
// ‘ункци€ дл€ чтени€ CSV-файла в двумерный вектор строк
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
            // ƒобавл€ем зап€тую, кроме последнего элемента в строке
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

void select(Vector<string> columns, Vector<string> tables, string condition) {
        // ѕровер€ем, что таблица существует и это единственна€ таблица
        for (int i = 0; i < tables.size(); i++) {
            string tableName = tables.get(i);
            if (!schema.structure.contains(tableName)) {
                cerr << "Table '" << tableName << "' does not exist." << endl;
                return;
            }
        }

        // ѕровер€ем, что все таблицы есть в колонках
        Vector<string> tablesInCols;
        for (int j = 0; j < tables.size(); j++) {
            string tableName = tables.get(j);
            bool colFoundInTables = false;
            for (int i = 0; i < columns.size(); i++) {
                Vector<string> parts = split(columns.get(i), ".");
                if (parts.size() != 2) {
                    cerr << "incorrect column " + columns.get(i) << endl;
                    return;
                }
                string colTableName = parts.get(0);
                string column = parts.get(1);

                if (tableName == colTableName) {
                    colFoundInTables = true;
                    break;
                }
            }
            if (!colFoundInTables) {
                cerr << "table " + tableName + " not found in columns" << endl;
                return;
            }
        }

        if (condition == "") {
            if (tables.size() == 1) { // дл€ одной таблицы
                string tableName = tables.get(0);

                // ѕровер€ем, что все колонки относ€тс€ к таблице
                Vector<string> availableColumns = schema.structure.get(tableName);
                Vector<string> columnsToSelect;

                for (int i = 0; i < columns.size(); i++) {
                    string fullColumn = columns.get(i);
                    // –аздел€ем строку на таблицу и колонку (формат таблица.колонка)
                    size_t dotPos = fullColumn.find(".");
                    if (dotPos == string::npos) {
                        cerr << "Error: Column '" << fullColumn << "' is not in 'table.column' format." << endl;
                        return;
                    }

                    string requestedTable = fullColumn.substr(0, dotPos);
                    string requestedColumn = fullColumn.substr(dotPos + 1);

                    // ѕровер€ем, что таблица совпадает с той, что указана в запросе
                    if (requestedTable != tableName) {
                        cerr << "Error: Column '" << fullColumn << "' does not belong to table '" << tableName << "'." << endl;
                        return;
                    }

                    // ѕровер€ем, что колонка существует в таблице
                    if (availableColumns.find(requestedColumn) == -1) {
                        cerr << "Error: Column '" << requestedColumn << "' does not exist in table '" << tableName << "'." << endl;
                        return;
                    }

                    // только названи€ колонок
                    columnsToSelect.pushBack(requestedColumn);
                }

                // “еперь читаем CSV-файлы и выводим только запрошенные колонки
                Vector<string> pages = listCSVFiles(schema.name + "/" + tableName);
                for (int i = 0; i < pages.size(); i++) {
                    string pagePath = pages.get(i);
                    Vector<Vector<string>> page = readCSV(pagePath);
                    Vector<string> header = page.get(0);

                    // ќпредел€ем индексы нужных колонок
                    Vector<int> columnIndexes;
                    for (int j = 0; j < columnsToSelect.size(); j++) {
                        string col = columnsToSelect.get(j);
                        int index = header.find(col);
                        if (index != -1) {
                            columnIndexes.pushBack(index);
                        }
                    }

                    for (int j = 0; j < page.size(); j++) {
                        Vector<string> row = page.get(j);
                        // ¬ыводим только те колонки, которые были запрошены
                        for (int k = 0; k < columnIndexes.size(); k++) {
                            cout << row.get(columnIndexes.get(k));
                            if (k < columnIndexes.size() - 1) {
                                cout << ", ";
                            }
                        }
                        cout << endl;
                    }
                }
            } else { // дл€ нескольких таблиц CROSS JOIN
                Vector<Vector<Vector<string>>> tablesData;
                Vector<Vector<int>> columnIndexesList;

                // —обираем данные дл€ всех таблиц
                for (int i = 0; i < tables.size(); i++) {
                    string tableName = tables.get(i);
                    Vector<string> availableColumns = schema.structure.get(tableName);
                    Vector<string> columnsToSelect;

                    // ќпредел€ем, какие колонки нужно выбирать из каждой таблицы
                    for (int j = 0; j < columns.size(); j++) {
                        string fullColumn = columns.get(j);
                        size_t dotPos = fullColumn.find(".");
                        if (dotPos == string::npos) {
                            cerr << "Error: Column '" << fullColumn << "' is not in 'table.column' format." << endl;
                            return;
                        }

                        string requestedTable = fullColumn.substr(0, dotPos);
                        string requestedColumn = fullColumn.substr(dotPos + 1);
                        if (requestedTable == tableName) {
                            if (availableColumns.find(requestedColumn) == -1) {
                                cerr << "Error: Column '" << requestedColumn << "' does not exist in table '" << tableName << "'." << endl;
                                return;
                            }
                            columnsToSelect.pushBack(requestedColumn);
                        }
                    }

                    // „итаем CSV-файлы таблицы и собираем строки
                    Vector<Vector<string>> tableRows;
                    Vector<string> pages = getCSVFromDir(schema.name + "/" + tableName);
                    for (int i = 0; i < pages.size(); i++) {
                        string pagePath = pages.get(i);
                        Vector<Vector<string>> page = readCSV(pagePath);
                        Vector<string> header = page.get(0);

                        // ќпредел€ем индексы нужных колонок
                        Vector<int> columnIndexes;
                        for (int j = 0; j < columnsToSelect.size(); j++) {
                            string col = columnsToSelect.get(j);
                            int index = header.find(col);
                            if (index != -1) {
                                columnIndexes.pushBack(index);
                            }
                        }
                        columnIndexesList.pushBack(columnIndexes);

                        for (int j = 1; j < page.size(); j++) {
                            tableRows.pushBack(page.get(j));
                        }
                    }
                    tablesData.pushBack(tableRows);
                }
                // ¬ыполн€ем декартово произведение данных
                Vector<Vector<string>> crossProduct;
                Vector<string> currentCombination;
                // cout << columnIndexesList << endl;
                function<void(int)> generateCombinations = [&](int depth) {
                    if (depth == tables.size()) {
                        crossProduct.pushBack(currentCombination.copy());
                        return;
                    }

                    Vector<Vector<string>> tableData = tablesData.get(depth);
                    for (int i = 0; i < tableData.size(); i++) {
                        Vector<string> row = tableData.get(i);
                        Vector<int> columnIndexes = columnIndexesList.get(depth);
                        for (int j = 0; j < columnIndexes.size(); j++) {
                            currentCombination.pushBack(row.get(columnIndexes.get(j)));
                        }
                        generateCombinations(depth + 1);
                        int newLen = currentCombination.size() - columnIndexes.size();
                        currentCombination.resize(newLen);
                    }
                };

                generateCombinations(0);

                // ¬ыводим результаты
                cout << columns << endl;
                for (int i = 0; i < crossProduct.size(); i++) {
                    Vector<string> combination = crossProduct.get(i);
                    for (int j = 0; j < combination.size(); j++) {
                        cout << combination.get(j);
                        if (j < combination.size() - 1) {
                            cout << ", ";
                        }
                    }
                    cout << endl;
                }
            }
        }}

#endif // FUNC_H_INCLUDED
