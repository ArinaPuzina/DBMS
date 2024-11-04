#ifndef SCHEMA_H_INCLUDED
#define SCHEMA_H_INCLUDED
#include "hashTab.h"
#include "vector.h"
#include <string>
#include <iostream>
using namespace std;
// Структура для схемы
struct Schema {
    string name; // Название схемы
    Map<Vector<string>> structure; // Структура таблиц (имя таблицы и столбцы)
    int tuplesLimit; // Лимит на количество кортежей в одном файле
};
extern Schema schema;

#endif // SCHEMA_H_INCLUDED
