#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED
#include <string>
#include <iostream>
#define LOADFACTOR 0.5
using namespace std;
template <typename T>
struct Vector {
    T* data;
    int len;
    int cap;

    Vector(int initCap = 16, int initLen = 0) {
        cap = initCap;
        len = 0;
        data = new T[cap];
    }

    int size() const {
        return len;
    }

    void extend() {
        int newCap = cap * 2;
        T* newData = new T[newCap];
        for (int i = 0; i < cap; i++) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        cap = newCap;
    }

    void pushBack(T value) {
        if (static_cast<float>(len) / cap >= LOADFACTOR) {
            extend();
        }
        data[len] = value;
        len++;
    }

    void insert(int index, T value) {
        if (static_cast<float>(len) / cap >= LOADFACTOR) {
            extend();
        }
        for (int i = len; i > index; i--) {
            data[i] = data[i - 1];
        }
        data[index] = value;
        len++;
    }

    void remove(int index) {
        if (index < 0 || index >= len) {
            throw invalid_argument("invalid index");
        }

        for (int i = index; i < len - 1; i++) {
            data[i] = data[i + 1];
        }
        len--;
    }

    void set(int index, T value) {
        if (index < 0 || index >= len) {
            throw invalid_argument("invalid index");
        }
        data[index] = value;
    }

    void resize(int newLen) {
        len = newLen;
    }

    T get(int index) const {
        if (index < 0 || index >= len) {
            throw invalid_argument("invalid index");
        }

        return data[index];
    }

    int find(const T& value) const {
        for (int i = 0; i < len; i++) {
            if (data[i] == value) {
                return i; //dозвращаем индекс, если значение найдено
            }
        }
        return -1;
    }

    string join(char delimiter = ',') {
        string result = "";

        for (int i = 0; i < size(); i++) {
            if (i != size() - 1) {
                result += data[i] + delimiter;
            }
            else {
                result += data[i];
            }
        }

        return result;
    }
    //для for_each
    T* begin() {
        return data;
    }


    T* end() {
        return data + len;
    }
    Vector<T> copy() const {
        Vector<T> newVector(cap);
        newVector.len = len;

        for (int i = 0; i < len; i++) {
            newVector.data[i] = data[i];
        }

        return newVector;
    }

};

template <typename T>
ostream& operator<<(ostream& os, const Vector<T>& vec) {
    for (int i = 0; i < vec.size(); i++) {
        if (i != vec.size() - 1) {
            os << vec.get(i) << ",";
        }
        else {
            os << vec.get(i);
        }
    }
    return os;
}
string trim(const string& str, char ch = ' ') {
    size_t first = str.find_first_not_of(ch);
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(ch);
    return str.substr(first, last - first + 1);
}

Vector<string> split(string str, string delimiter) {
    Vector<string> values;
    size_t pos = 0;
    while ((pos = str.find(delimiter)) != string::npos) {
        string value = str.substr(0, pos);
        str = trim(trim(str), '\t');
        values.pushBack(value);
        str.erase(0, pos + delimiter.length());
    }
    str = trim(trim(str), '\t');
    values.pushBack(str);  // Последнее значение

    return values;
}
#endif // VECTOR_H_INCLUDED
