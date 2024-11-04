#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED
#include <string>
#include <iostream>
using namespace std;
template <typename T>
struct Vector {
    T* data;
    int len;
    int cap;
    float loadFactor;

    Vector(int initCap = 16, int initLen = 0) {
        cap = initCap;
        len = 0;
        data = new T[cap];
        loadFactor = 0.5;
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
        if (static_cast<float>(len) / cap >= loadFactor) {
            extend();
        }
        data[len] = value;
        len++;
    }

    void insert(int index, T value) {
        if (static_cast<float>(len) / cap >= loadFactor) {
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
                return i; // ¬озвращаем индекс, если значение найдено
            }
        }
        return -1; // ¬озвращаем -1, если значение не найдено
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

#endif // VECTOR_H_INCLUDED
