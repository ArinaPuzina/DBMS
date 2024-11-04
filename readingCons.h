#ifndef READINGCONS_H_INCLUDED
#define READINGCONS_H_INCLUDED
#include <regex>
#include <vector>
#include <sstream>
#include "Schema.h"
#include "func.h"

using namespace std;

/// Регулярные выражения для SQL команд
regex insertRegex("^INSERT\\s+INTO\\s+(\\w+)\\s+VALUES\\s+\\((.+)\\)\\s*;?$", regex_constants::icase);
regex selectRegex("^SELECT\\s+([\\w\\d\\.,\\s]+)\\s+FROM\\s+([\\w\\d,\\s]+)$", regex_constants::icase); // без where
regex deleteRegex("^DELETE\\s+FROM\\s+([\\w\\d,\\s]+)\\s*;?$", regex_constants::icase); // без where
regex deleteWhereRegex("^DELETE\\s+FROM\\s+([\\w\\d,\\s]+)\\s*WHERE\\s+(.+?)?\\s*;?$", regex_constants::icase);
regex selectWhereRegex("^SELECT\\s+([\\w\\d\\.,\\s]+)\\s+FROM\\s+([\\w\\d,\\s]+)\\s+WHERE\\s+(.+?)?\\s*;?$", regex_constants::icase);

enum CommandType {
    INSERT,
    SELECT,
    DELETE,
    UNKNOWN
};

CommandType identifyCommand(const string& command, smatch& match) {
    if (regex_match(command, match, insertRegex)) {
        return INSERT;
    } else if (regex_match(command, match, selectRegex)) {
        return SELECT;
    } else if (regex_match(command, match, selectWhereRegex)) {
        return SELECT;
    } else if (regex_match(command, match, deleteRegex)) {
        return DELETE;
    } else if (regex_match(command, match, deleteWhereRegex)) {
        return DELETE;
    }
    return UNKNOWN;
}

void menu(const string& command) {
    smatch match;
    CommandType commandType = identifyCommand(command, match);

    switch (commandType) {
        case INSERT: {
            string tableName = match[1];
            string valuesStr = match[2];

            Vector<string> values;
            regex valuePattern(R"('([^']+)')");
            auto values_begin = sregex_iterator(valuesStr.begin(), valuesStr.end(), valuePattern);
            auto values_end = sregex_iterator();

            for (auto it = values_begin; it != values_end; ++it) {
                values.pushBack(it->str(1));
            }
            Insert(tableName, values);
            cout << "the insertion occurred in: " << tableName << endl;
            cout << "it was filled with values: ";
            cout<<values<<endl;
            break;
        }
        case SELECT: {
            string columns = match[1];
            string tables = match[2];
            cout << "SELECT columns: " << columns << endl;
            cout << "FROM tables: " << tables << endl;

            // Если нужна дополнительная проверка для SELECT с WHERE, добавьте здесь.
            break;
        }
        case DELETE: {
            string tableName = match[1];
//            if (match.size() > 2) {
//                string condition = match[2];
//                cout << "DELETE FROM table: " << tableName << endl;
//                cout << "WHERE condition: " << condition << endl;}
             if(match.size()<=2) {
                DeleteFrom(tableName);
                cout << "DELETE FROM table: " << tableName << endl;
            }
            break;
        }
        case UNKNOWN:
        default:
            cout << "Unknown command." << endl;
            break;
    }
}


#endif // READINGCONS_H_INCLUDED
