#include "vector.h"
#include <fstream>
#include <filesystem>
#include "hashTab.h"
#include "json.hpp"
#include "readingCons.h"
#include "Schema.h"
#include "func.h"
#include <locale>
#include <unistd.h>

#include <stdlib.h>
#include <string>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <thread>
#include <mutex>
#define ERROR_S "SERVER ERROR"
#define PORT 7437



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

        //cоздание файла 1.csv, если его нет
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


        string pkFilePath = tablePath + "/" + tableName + "_pk_sequence";
        if (!fs::exists(pkFilePath)) {
            ofstream pkFile(pkFilePath);
            if (pkFile.is_open()) {
                pkFile << "0"; // Начальная последовательность
                pkFile.close();
            }
        }


        string lockFilePath = tablePath + "/" + tableName + "_lock";
        if (!fs::exists(lockFilePath)) {
            ofstream lockFile(lockFilePath);
            lockFile.close();
        }
    }
}
// Обработка клиента
void handleClient(int clientSocket) {
    char buffer[1024] = {0};

    while (true) {
        int valread = read(clientSocket, buffer, sizeof(buffer));
        if (valread <= 0) break;

        string command(buffer);
        command = trim(command, '\n');

        if (command == "exit") break;

        try {
            menu(command);
            send(clientSocket, "OK\n", 3, 0);
        } catch (const exception& e) {
            send(clientSocket, ERROR_S, strlen(ERROR_S), 0);
        }

        memset(buffer, 0, sizeof(buffer));
    }
    shutdown(clientSocket, SHUT_RDWR);
    close(clientSocket);
}

// Запуск сервера
void startServer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) throw runtime_error("Socket creation failed");

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
        throw runtime_error("Socket bind failed");

    if (listen(server_fd, 3) < 0)
        throw runtime_error("Socket listen failed");

    cout << "Server listening on port " << PORT << "..." << endl;

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(server_fd, (sockaddr*)&clientAddress, &clientAddressLen);
        if (clientSocket < 0) {
            cerr << "Accept failed" << endl;
            continue;
        }

        thread(handleClient, clientSocket).detach();
    }
}

// Основная функция
int main() {
    try {
        cout << "Reading schema from JSON file..." << endl;
        schema = readSchemaFromFile("scheme.json");

        cout << "Creating directories and files..." << endl;
        createSchemaDirectories();

        cout << "Starting DBMS..." << endl;
        startServer();
    } catch (const exception& e) {
        cerr << "Critical error: " << e.what() << endl;
    }

    return 0;
}
