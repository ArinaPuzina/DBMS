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

    csvOutput << currentKey<<",";
    for (size_t i = 0; i < values.size(); ++i) {
        csvOutput << values.get(i);
        if (i !=values.size()-1)
            csvOutput <<",";
    }
    csvOutput << endl;
    csvOutput.close();

    //cохраняем обновленный первичный ключ
    ofstream pkOutput(pkFilePath);
    pkOutput << currentKey;
    pkOutput.close();
}

void fDelete(Vector<string> tables, string condition) {
        if (condition == "") {
            for (int i = 0; i < tables.size(); i++) {
                string tableName = tables.get(i);

                Vector<string> pages = listCSVFiles(schema.name + "/" + tableName);
                for (int j = 0; j < pages.size(); j++) {
                    fs::remove(pages.get(j));
                }

                string pagePath = schema.name + "/" + tableName + "/1.csv";
                writeList(pagePath, tableName, schema.structure.get(tableName));
            // Очистка файла с первичным ключом
            string pkSequencePath = schema.name + "/" + tableName + "/" + tableName + "_pk_sequence";
            ofstream pkSequenceFile(pkSequencePath, ios::trunc); // Открытие в режиме очистки
            if (pkSequenceFile.is_open()) {
                pkSequenceFile << "0"; // Устанавливаем первичный ключ в начальное значение
                pkSequenceFile.close();
            } else {
                cerr << "Failed to clear primary key sequence file: " << pkSequencePath << endl;
            }
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
        elementsOfRows = split(elementsInVec.get(i), ",");//разделяем строку поэлементно
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

                if (expectedValue.front() == '\'' && expectedValue.back() == '\'') {//если колона сравнивается со значением
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

        //если строка соответствует условиям OR
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

//для cross join
void generateCombinations(int index,const Vector<Vector<Vector<string>>>& tablesData,const Vector<Vector<int>>& columnIndexesList,Vector<Vector<string>>& crossProduct,Vector<string>& currentCombination) {
    if (index == tablesData.size()) {
        crossProduct.pushBack(currentCombination.copy());
        return;
    }

    Vector<Vector<string>> tableData = tablesData.get(index);//данные текущей таблицы
    for (int i = 0; i < tableData.size(); i++) {
        Vector<string> row = tableData.get(i);
        Vector<int> columnIndexes = columnIndexesList.get(index);

        // Добавляем элементы строки по соответствующим индексам
        for (int j = 0; j < columnIndexes.size(); j++) {
            currentCombination.pushBack(row.get(columnIndexes.get(j)));
        }

        //вызов для следующей таблицы
        generateCombinations(index + 1, tablesData, columnIndexesList, crossProduct, currentCombination);

        //eдаляем добавленные только что элкменты
        int newLen = currentCombination.size() - columnIndexes.size();
        currentCombination.resize(newLen);
    }
}

void select(Vector<string> columns, Vector<string> tables) {
        // Проверяем, что таблица существует и это единственная таблица
        for (int i = 0; i < tables.size(); i++) {
            string tableName = tables.get(i);
            if (!schema.structure.contains(tableName)) {
                cerr << "Table '" << tableName << "' does not exist." << endl;
                return;
            }
        }

        // Проверяем, что все таблицы есть в колонках
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

            if (tables.size() == 1) { // для одной таблицы
                string tableName = tables.get(0);

                // Проверяем, что все колонки относятся к таблице
                Vector<string> availableColumns = schema.structure.get(tableName);
                Vector<string> columnsToSelect;

                for (int i = 0; i < columns.size(); i++) {
                    string fullColumn = columns.get(i);
                    // Разделяем строку на таблицу и колонку (формат таблица.колонка)
                    size_t dotPos = fullColumn.find(".");
                    if (dotPos == string::npos) {
                        cerr << "Error: Column '" << fullColumn << "' is not in 'table.column' format." << endl;
                        return;
                    }

                    string requestedTable = fullColumn.substr(0, dotPos);
                    string requestedColumn = fullColumn.substr(dotPos + 1);

                    // Проверяем, что таблица совпадает с той, что указана в запросе
                    if (requestedTable != tableName) {
                        cerr << "Error: Column '" << fullColumn << "' does not belong to table '" << tableName << "'." << endl;
                        return;
                    }

                    // Проверяем, что колонка существует в таблице
                    if (availableColumns.find(requestedColumn) == -1) {
                        cerr << "Error: Column '" << requestedColumn << "' does not exist in table '" << tableName << "'." << endl;
                        return;
                    }

                    // только названия колонок
                    columnsToSelect.pushBack(requestedColumn);
                }

                // Теперь читаем CSV-файлы и выводим только запрошенные колонки
                Vector<string> pages = listCSVFiles(schema.name + "/" + tableName);
                for (int i = 0; i < pages.size(); i++) {
                    string pagePath = pages.get(i);
                    Vector<Vector<string>> page = readCSV(pagePath);
                    Vector<string> header = page.get(0);

                    // Определяем индексы нужных колонок
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
                        // Выводим только те колонки, которые были запрошены
                        for (int k = 0; k < columnIndexes.size(); k++) {
                            cout << row.get(columnIndexes.get(k));
                            if (k < columnIndexes.size() - 1) {
                                cout << ", ";
                            }
                        }
                        cout << endl;
                    }
                }
            } else { // для нескольких таблиц CROSS JOIN.................................................
                Vector<Vector<Vector<string>>> tablesData;
                Vector<Vector<int>> columnIndexesList;
                for (int i = 0; i < tables.size(); i++) {
                    string tableName = tables.get(i);
                    Vector<string> availableColumns = schema.structure.get(tableName);//достаем все колонки
                    Vector<string> columnsToSelect;

                    //определяем, какие колонки нужно выбирать из каждой таблицы
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

                    //читаем CSV-файлы таблицы и собираем строки
                    Vector<Vector<string>> tableRows;
                    Vector<string> pages = getCSVFromDir(schema.name + "/" + tableName);
                    for (int i = 0; i < pages.size(); i++) {
                        string pagePath = pages.get(i);
                        Vector<Vector<string>> page = readCSV(pagePath);
                        Vector<string> header = page.get(0);

                        //определяем индексы нужных колонок
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
                            tableRows.pushBack(page.get(j));//строки таблицы
                        }
                    }
                    tablesData.pushBack(tableRows);//вектор таблиц из векторов строк
                }
                //выполняем декартово произведение данных
                Vector<Vector<string>> crossProduct;
                Vector<string> currentCombination;

                generateCombinations(0, tablesData, columnIndexesList, crossProduct, currentCombination);


                // Выводим результаты
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
        }
void selectWhere(Vector<string> columns, Vector<string> tables, const Vector<string>& ConditionsOR) {
    //проверяем, что таблица существует и это единственная таблица
    for (int i = 0; i < tables.size(); i++) {
        string tableName = tables.get(i);
        if (!schema.structure.contains(tableName)) {
            cerr << "Table '" << tableName << "' does not exist." << endl;
            return;
        }
    }

    //проверяем, что все таблицы есть в колонках
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

    if (tables.size() == 1) { // Для одной таблицы
        string tableName = tables.get(0);

        // Проверяем, что все колонки относятся к таблице
        Vector<string> availableColumns = schema.structure.get(tableName);
        Vector<string> columnsToSelect;

        for (int i = 0; i < columns.size(); i++) {
            string fullColumn = columns.get(i);
            size_t dotPos = fullColumn.find(".");
            if (dotPos == string::npos) {
                cerr << "Error: Column '" << fullColumn << "' is not in 'table.column' format." << endl;
                return;
            }

            string requestedTable = fullColumn.substr(0, dotPos);
            string requestedColumn = fullColumn.substr(dotPos + 1);

            if (requestedTable != tableName) {
                cerr << "Error: Column '" << fullColumn << "' does not belong to table '" << tableName << "'." << endl;
                return;
            }

            if (availableColumns.find(requestedColumn) == -1) {
                cerr << "Error: Column '" << requestedColumn << "' does not exist in table '" << tableName << "'." << endl;
                return;
            }

            columnsToSelect.pushBack(requestedColumn);
        }


        Vector<string> pages = listCSVFiles(schema.name + "/" + tableName);//чтение CSV-файлов и обработка условий WHERE
        for (int i = 0; i < pages.size(); i++) {
            string pagePath = pages.get(i);
            Vector<Vector<string>> page = readCSV(pagePath);
            Vector<string> header = page.get(0);

            // Определяем индексы нужных колонок
            Vector<int> columnIndexes;
            for (int j = 0; j < columnsToSelect.size(); j++) {
                string col = columnsToSelect.get(j);
                int index = header.find(col);
                if (index != -1) {
                    columnIndexes.pushBack(index);
                }
            }

            // Проход по строкам страницы
            for (int rowIndex = 1; rowIndex < page.size(); rowIndex++) { // Пропускаем заголовок
                Vector<string> elementsOfRows = page.get(rowIndex);
                bool rowMatchesOr = false;

                //gроверка условий OR
                for (int orIndex = 0; orIndex < ConditionsOR.size(); ++orIndex) {
                    Vector<string> andConditions = split(ConditionsOR.get(orIndex), "AND");
                    bool rowMatchesAnd = true;

                    //проверка условий AND
                    for (const auto& condition : andConditions) {
                        Vector<string> conditionParts = split(condition, "=");
                        if (conditionParts.size() != 2) continue;//смотрим следующее условие

                        string column = conditionParts.get(0);
                        string expectedValue = conditionParts.get(1);

                        int columnIndex = stoi(findColumnIndex(column));
                        if (columnIndex == -1) {
                            rowMatchesAnd = false;
                            break;
                        }

                        if (expectedValue.front() == '\'' && expectedValue.back() == '\'') {
                            string constantValue = expectedValue.substr(1, expectedValue.size() - 2);
                            if (elementsOfRows.get(columnIndex) != constantValue) {
                                rowMatchesAnd = false;
                                break;//переходим к OR так как AND очевидно 0
                            }
                        } else {
                            int compareIndex = stoi(findColumnIndex(expectedValue));
                            if (compareIndex == -1 || elementsOfRows.get(columnIndex) != elementsOfRows.get(compareIndex)) {
                                rowMatchesAnd = false;
                                break;//-//-
                            }
                        }
                    }

                    if (rowMatchesAnd) {
                        rowMatchesOr = true;
                        break;
                    }
                }

                // Если строка соответствует условиям OR,
                if (rowMatchesOr) {
                    for (int k = 0; k < columnIndexes.size(); k++) {
                        cout << elementsOfRows.get(columnIndexes.get(k));//выводим нужные колонки
                        if (k < columnIndexes.size() - 1) {
                            cout << ", ";
                        }
                    }
                    cout << endl;
                }
            }
        }
    }
    else if (tables.size() > 1) { // Для нескольких таблиц CROSS JOINNNNNNN-------------------------------------------------------------------------------
        Vector<Vector<Vector<string>>> tablesData;//вектор векторов-таблиц из векторов строк
        Vector<Vector<int>> columnIndexesList;

        //считываем данные для всех таблиц
        for (int i = 0; i < tables.size(); i++) {
            string tableName = tables.get(i);
            Vector<string> availableColumns = schema.structure.get(tableName);
            Vector<string> columnsToSelect;

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

            //cчитываем данные таблицы
            Vector<Vector<string>> tableRows;
            Vector<string> pages = listCSVFiles(schema.name + "/" + tableName);
            for (int k = 0; k < pages.size(); k++) {
                string pagePath = pages.get(k);
                Vector<Vector<string>> page = readCSV(pagePath);
                Vector<string> header = page.get(0);

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

        //выполняем декартово произведение
        Vector<Vector<string>> crossProduct;
        Vector<string> currentCombination;

        generateCombinations(0, tablesData, columnIndexesList, crossProduct, currentCombination);

        // Фильтруем результаты по условиям WHERE
        Vector<Vector<string>> filteredResult;

        for (int i = 0; i < crossProduct.size(); i++) {
            Vector<string> row = crossProduct.get(i);
            bool rowMatchesOr = false;

            for (int orIndex = 0; orIndex < ConditionsOR.size(); ++orIndex) {
                Vector<string> andConditions = split(ConditionsOR.get(orIndex), "AND");//поделили на энд
                bool rowMatchesAnd = true;

                for (const auto& condition : andConditions) {
                    Vector<string> conditionParts = split(condition, "=");//поделили на условия а    =    б
                    if (conditionParts.size() != 2) continue;//если нет преходим к следующему условию

                    string column = conditionParts.get(0);
                    string expectedValue = conditionParts.get(1);

                    size_t dotPos = column.find(".");
                    if (dotPos == string::npos) {
                        cerr << "Error: Column '" << column << "' is not in 'table.column' format." << endl;
                        rowMatchesAnd = false;
                        break;//перешли к следующему энд
                    }

                    string tableName = column.substr(0, dotPos);
                    string columnName = column.substr(dotPos + 1);

                    int columnIndex = -1;
                    int currentIndex = 0;
                    for (int tableIndex = 0; tableIndex < tables.size(); tableIndex++) {
                        if (tables.get(tableIndex) == tableName) {
                            Vector<int> columnIndexes = columnIndexesList.get(tableIndex);
                            Vector<string> tableColumns = schema.structure.get(tableName);

                            for (int colIdx : columnIndexes) {
                                if (tableColumns.get(colIdx) == columnName) {
                                    columnIndex = currentIndex;
                                    break;
                                }
                                currentIndex++;
                            }
                            if (columnIndex != -1) break;
                        } else {
                            currentIndex += columnIndexesList.get(tableIndex).size();
                        }
                    }

                    if (columnIndex == -1) {
                        rowMatchesAnd = false;
                        break;
                    }

                    if (expectedValue.front() == '\'' && expectedValue.back() == '\'') {
                        string constantValue = expectedValue.substr(1, expectedValue.size() - 2);
                        if (row.get(columnIndex) != constantValue) {
                            rowMatchesAnd = false;
                            break;
                        }
                    }
                    else if (expectedValue.find(".") != string::npos) {
                // Проверяем, что expectedValue также в формате "table.column"
                        size_t dotPosValue = expectedValue.find(".");
                        string expectedTable = expectedValue.substr(0, dotPosValue);
                        string expectedColumn = expectedValue.substr(dotPosValue + 1);

                        int expectedColumnIndex = -1;
                        int currentIndexValue = 0;

                        for (int tableIndex = 0; tableIndex < tables.size(); tableIndex++) {
                            if (tables.get(tableIndex) == expectedTable) {
                                Vector<int> columnIndexes = columnIndexesList.get(tableIndex);
                                Vector<string> tableColumns = schema.structure.get(expectedTable);

                                for (int colIdx : columnIndexes) {
                                    if (tableColumns.get(colIdx) == expectedColumn) {
                                        expectedColumnIndex = currentIndexValue;
                                        break;
                                        }
                                    currentIndexValue++;
                                    }
                                if (expectedColumnIndex != -1) break;
                            }   else {
                                    currentIndexValue += columnIndexesList.get(tableIndex).size();
                            }
                        }

                        if (expectedColumnIndex == -1) {
                            cerr << "Error: Column '" << expectedValue << "' does not exist in the provided tables." << endl;
                            rowMatchesAnd = false;
                            break;
                        }


                        if (row.get(columnIndex) != row.get(expectedColumnIndex)) {
                            rowMatchesAnd = false;// Сравниваем значения двух колонок
                            break;
                    }
                }

                }

                if (rowMatchesAnd) {
                    rowMatchesOr = true;
                    break;
                }
            }

            if (rowMatchesOr) {
                filteredResult.pushBack(row);
            }
        }
        if(filteredResult.size()==0){
            cout<<"Нет строк, удовлетворяющих данным условиям, значит и колон нет"<<endl;
        }
        // Выводим отфильтрованные строки
        for (int i = 0; i < filteredResult.size(); i++) {
            Vector<string> combination = filteredResult.get(i);
            for (int j = 0; j < combination.size(); j++) {
                cout << combination.get(j);
                if (j < combination.size() - 1) {
                    cout << ", ";
                }
            }
            cout << endl;
        }
    }
}


#endif // FUNC_H_INCLUDED
