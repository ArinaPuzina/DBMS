#ifndef FUNC_H_INCLUDED
#define FUNC_H_INCLUDED

void Insert(const string& tableName, const Vector<string>& values) {
string pathToTable = schema.name + "/" + tableName;

    // Определяем путь к файлу первичных ключей
    string pkFilePath = pathToTable + "/" + tableName + "_pk_sequence";

    // Открываем или создаем файл с первичными ключами
    ifstream pkInput(pkFilePath);
    int currentKey = 0;
    if (pkInput) {
        pkInput >> currentKey; // Читаем текущее значение первичного ключа
    }
    pkInput.close();

    // Увеличиваем первичный ключ на 1
    currentKey++;

    // Определяем текущий файл для записи (1.csv, 2.csv и т.д.)
    int index = 1;
    string currentFile;
    while (true) {
        currentFile = pathToTable + "/" + to_string(index) + ".csv";
        ifstream csvCheck(currentFile);
        int lineCount = 0;
        string csvLine;

        // Считаем количество строк в текущем файле
        while (getline(csvCheck, csvLine)) {
            lineCount++;
        }

        // Если файл не заполнен (меньше, чем tuples_limit), используем его
        if (lineCount < schema.tuplesLimit) {
            break;
        }
        index++; // Переход к следующему файлу, если текущий заполнен
    }

    // Открываем CSV файл для добавления новой строки
    ofstream csvOutput(currentFile, ios::app);
    if (!csvOutput.is_open()) {
        throw runtime_error("Unable to open file: " + currentFile);
    }

    // Формируем строку с новыми данными, добавляем первичный ключ
    csvOutput << currentKey; // Записываем первичный ключ
    for (size_t i = 0; i < values.size(); ++i) {
        csvOutput << "," << values.get(i); // Используем метод get для получения значения
    }
    csvOutput << endl; // Завершаем запись новой строки
    csvOutput.close();

    // Сохраняем обновленный первичный ключ
    ofstream pkOutput(pkFilePath);
    pkOutput << currentKey; // Записываем новое значение первичного ключа
    pkOutput.close();
}
//void Where(){
//}
void DeleteFrom(const string& tableName){
    string pathToTable = schema.name + "/" + tableName;

    // Определяем текущий файл для записи (1.csv, 2.csv и т.д.)
    int index = 1;
    string currentFile;
    while (true) {
        currentFile = pathToTable + "/" + to_string(index) + ".csv";
        ifstream csvCheck(currentFile);

        // Проверяем, существует ли файл
        if (!csvCheck) {
            break; // Если файл не существует, выходим из цикла
        }

        // Очищаем содержимое текущего CSV файла
        ofstream csvOutput(currentFile, ios::trunc);
        if (!csvOutput.is_open()) {
            throw runtime_error("Unable to open file for deletion: " + currentFile);
        }

        // Закрываем файл после очистки
        csvOutput.close();
        index++; // Переход к следующему файлу
    }

    // Удаляем файл первичных ключей
    string pkFilePath = pathToTable + "/" + tableName + "_pk_sequence";
    remove(pkFilePath.c_str()); // Удаляем файл с первичными ключами

}

#endif // FUNC_H_INCLUDED
