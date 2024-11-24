#ifndef SCHEMA_H_INCLUDED
#define SCHEMA_H_INCLUDED
#include "hashTab.h"
#include "vector.h"
#include <string>
#include <iostream>
using namespace std;

struct Schema {
    string name;
    Map<Vector<string>> structure;
    int tuplesLimit;
};
extern Schema schema;

#endif // SCHEMA_H_INCLUDED
