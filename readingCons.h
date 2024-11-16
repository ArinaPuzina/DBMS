#ifndef READINGCONS_H_INCLUDED
#define READINGCONS_H_INCLUDED
#include <regex>
#include "vector.h"
#include <sstream>
#include "Schema.h"
#include "func.h"
#include "locking.h"
#include <string>
using namespace std;


/// –егул€рные выражени€ дл€ SQL команд
regex insertRegex("^INSERT\\s+INTO\\s+(\\w+)\\s+VALUES\\s+\\((.+)\\)\\s*;?$", regex_constants::icase);
regex selectRegex("^SELECT\\s+([\\w\\d\\.,\\s]+)\\s+FROM\\s+([\\w\\d,\\s]+)$", regex_constants::icase); // без where
regex deleteRegex("^DELETE\\s+FROM\\s+([\\w\\d,\\s]+)\\s*;?$", regex_constants::icase); // без where
regex deleteWhereRegex("^DELETE\\s+FROM\\s+([\\w\\d,\\s]+)\\s*WHERE\\s+(.+?)?\\s*;?$", regex_constants::icase);
regex selectWhereRegex("^SELECT\\s+([\\w\\d\\.,\\s]+)\\s+FROM\\s+([\\w\\d,\\s]+)\\s+WHERE\\s+(.+?)?\\s*;?$", regex_constants::icase);

enum CommandType {
    INSERT,
    SELECT,
    SELECTWHERE,
    DELETE,
    DELETEWHERE,
    UNKNOWN
};

CommandType identifyCommand(const string& command, smatch& match) {
    if (regex_match(command, match, insertRegex)) {
        return INSERT;
    } else if (regex_match(command, match, selectRegex)) {
        return SELECT;
    } else if (regex_match(command, match, selectWhereRegex)) {
        return SELECTWHERE;
    } else if (regex_match(command, match, deleteRegex)) {
        return DELETE;
    } else if (regex_match(command, match, deleteWhereRegex)) {
        return DELETEWHERE;
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

        case DELETE: {
        string tablesStr = match[1].str();
        Vector<string> tables = split(tablesStr, ",");
        fDelete(tables, "");
        //unlockTables(storage.schema.name, tables);
        }
        case DELETEWHERE: {
            //делим по OR
            string tableName=match[1].str();
            string conditions=match[2].str();
            Vector<string> ConditionsOR= split(conditions,"OR");

            DeleteWhere(tableName, ConditionsOR);
            break;
        }
        case SELECT:{
        string columnsStr = match[1].str();
        string tablesStr = match[2].str();
        Vector<string> tables = split(tablesStr, ",");
        Vector<string> columns = split(columnsStr, ",");

        select(columns, tables, "");
            break;
        }


        case UNKNOWN:
        default:
            cout << "Unknown command." << endl;
            break;
    }
}


#endif // READINGCONS_H_INCLUDED
