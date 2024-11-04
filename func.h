#ifndef FUNC_H_INCLUDED
#define FUNC_H_INCLUDED

void Insert(const string& tableName, const Vector<string>& values) {
string pathToTable = schema.name + "/" + tableName;

    // ���������� ���� � ����� ��������� ������
    string pkFilePath = pathToTable + "/" + tableName + "_pk_sequence";

    // ��������� ��� ������� ���� � ���������� �������
    ifstream pkInput(pkFilePath);
    int currentKey = 0;
    if (pkInput) {
        pkInput >> currentKey; // ������ ������� �������� ���������� �����
    }
    pkInput.close();

    // ����������� ��������� ���� �� 1
    currentKey++;

    // ���������� ������� ���� ��� ������ (1.csv, 2.csv � �.�.)
    int index = 1;
    string currentFile;
    while (true) {
        currentFile = pathToTable + "/" + to_string(index) + ".csv";
        ifstream csvCheck(currentFile);
        int lineCount = 0;
        string csvLine;

        // ������� ���������� ����� � ������� �����
        while (getline(csvCheck, csvLine)) {
            lineCount++;
        }

        // ���� ���� �� �������� (������, ��� tuples_limit), ���������� ���
        if (lineCount < schema.tuplesLimit) {
            break;
        }
        index++; // ������� � ���������� �����, ���� ������� ��������
    }

    // ��������� CSV ���� ��� ���������� ����� ������
    ofstream csvOutput(currentFile, ios::app);
    if (!csvOutput.is_open()) {
        throw runtime_error("Unable to open file: " + currentFile);
    }

    // ��������� ������ � ������ �������, ��������� ��������� ����
    csvOutput << currentKey; // ���������� ��������� ����
    for (size_t i = 0; i < values.size(); ++i) {
        csvOutput << "," << values.get(i); // ���������� ����� get ��� ��������� ��������
    }
    csvOutput << endl; // ��������� ������ ����� ������
    csvOutput.close();

    // ��������� ����������� ��������� ����
    ofstream pkOutput(pkFilePath);
    pkOutput << currentKey; // ���������� ����� �������� ���������� �����
    pkOutput.close();
}
//void Where(){
//}
void DeleteFrom(const string& tableName){
    string pathToTable = schema.name + "/" + tableName;

    // ���������� ������� ���� ��� ������ (1.csv, 2.csv � �.�.)
    int index = 1;
    string currentFile;
    while (true) {
        currentFile = pathToTable + "/" + to_string(index) + ".csv";
        ifstream csvCheck(currentFile);

        // ���������, ���������� �� ����
        if (!csvCheck) {
            break; // ���� ���� �� ����������, ������� �� �����
        }

        // ������� ���������� �������� CSV �����
        ofstream csvOutput(currentFile, ios::trunc);
        if (!csvOutput.is_open()) {
            throw runtime_error("Unable to open file for deletion: " + currentFile);
        }

        // ��������� ���� ����� �������
        csvOutput.close();
        index++; // ������� � ���������� �����
    }

    // ������� ���� ��������� ������
    string pkFilePath = pathToTable + "/" + tableName + "_pk_sequence";
    remove(pkFilePath.c_str()); // ������� ���� � ���������� �������

}

#endif // FUNC_H_INCLUDED
