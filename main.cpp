#include "vector.h"
#include <fstream>
#include <filesystem>
#include "hashTab.h"
#include "json.hpp"
#include <locale>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <thread>
#include "Schema.h"
#include "readingCons.h"


#define BUFLEN 1024

using namespace std;

const int PORT = 7432;

using json = nlohmann::json;
namespace fs = std::filesystem;


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

// Создание директорий и файлов для таблиц
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
                pkFile << "0"; // Начальное значение первичного ключа
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

// Очистка буфера
void nullBuffer(char* buf, int len) {
    for (int i = 0; i < len; i++) {
        buf[i] = 0;
    }
}

// Обработка клиента
void handleClient(int clientSocket) {
    char buffer[BUFLEN];
    nullBuffer(buffer, BUFLEN);

    while (true) {
        int valread = read(clientSocket, buffer, BUFLEN);
        if (valread < 0) {
            cerr << "Error while reading query" << endl;
            return;
        }
        string command(buffer);
        command = trim(command, '\n');

        if (command == "exit") {
            break;
        }

        
        menu(command,clientSocket); 
        nullBuffer(buffer, BUFLEN);
    }

    close(clientSocket);
}

// Запуск сервера
void startServer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // создаём сервер SOCK_STREAM=TCP
    if (server_fd == 0) {
        cerr << "Server creation error" << endl;
        return;
    }

    struct sockaddr_in address;

    address.sin_family = AF_INET; // используем ipv4
    address.sin_addr.s_addr = INADDR_ANY; // любой адрес
    address.sin_port = htons(PORT); 

    // Привязываем сокет к порту
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Socket error" << endl;
        return;
    }
    // Слушаем подключения (до 3 штук)(queue)
    if (listen(server_fd, 3) < 0) {
        cerr << "Listen error" << endl;
        return;
    }

    cout << "Server listening on port " << PORT << "..." << endl;

    // Принимаем подключения клиентов
    while (true) {
        int newSocket;
        sockaddr_in clientAddress; // Структура для хранения адреса клиента
        socklen_t clientAddressLen = sizeof(clientAddress); 

        newSocket = accept(server_fd, (sockaddr*)&clientAddress, &clientAddressLen);
        if (newSocket < 0) {
            cerr << "Accept client error" << endl;
            return;
        }
        else{
            cout<<"Client accepted"<<endl;
        
            const char* welcomeMessage = "You are connected to the server!\n";
            send(newSocket, welcomeMessage, strlen(welcomeMessage), 0);
        }

        
        thread clientThread(handleClient, newSocket);
        clientThread.detach(); 
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