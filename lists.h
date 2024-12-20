#ifndef LISTS_H_INCLUDED
#define LISTS_H_INCLUDED

#pragma once
#include <iostream>
using namespace std;

// ����������� ������
template <typename T>
struct NodeSingly {
    T info;
    NodeSingly* next;
};

template <typename T>
void AddHeadS(NodeSingly<T>*& listic, T inf) {
    NodeSingly<T>* NewHead = new NodeSingly<T>;
    NewHead->info = inf;
    NewHead->next = listic;
    listic = NewHead;
}

template <typename T>
void AddTailS(NodeSingly<T>*& listic, T inf) {
    NodeSingly<T>* NewTail = new NodeSingly<T>;
    NewTail->info = inf;
    NewTail->next = nullptr;

    if (listic != nullptr) {
        NodeSingly<T>* lastNode = listic;
        while (lastNode->next != nullptr) {
            lastNode = lastNode->next;
        }
        lastNode->next = NewTail;
    }
    else {
        listic = NewTail;
    }
}

template <typename T>
void RemoveHeadS(NodeSingly<T>*& listic) {
    if (listic != nullptr) {
        NodeSingly<T>* temp = listic;
        listic = listic->next;
        delete temp;
    }
}

template <typename T>
void RemoveTailS(NodeSingly<T>*& listic) {
    if (listic == nullptr) return;
    if (listic->next == nullptr) {
        delete listic;
        listic = nullptr;
        return;
    }

    NodeSingly<T>* temp = listic;
    while (temp->next->next != nullptr) {
        temp = temp->next;
    }
    delete temp->next;
    temp->next = nullptr;
}

template <typename T>
void RemoveByValueS(NodeSingly<T>*& listic, T value) {
    if (listic == nullptr) return;
    if (listic->info == value) {
        RemoveHeadS(listic);
        return;
    }

    NodeSingly<T>* temp = listic;
    while (temp->next != nullptr && temp->next->info != value) {
        temp = temp->next;
    }

    if (temp->next != nullptr) {
        NodeSingly<T>* toDelete = temp->next;
        temp->next = temp->next->next;
        delete toDelete;
    }
}

template <typename T>
NodeSingly<T>* FindS(NodeSingly<T>* listic, T value) {
    NodeSingly<T>* temp = listic;
    while (temp != nullptr) {
        if (temp->info == value) {
            return temp;
        }
        temp = temp->next;
    }
    return nullptr;
}

template <typename T>
void PrintListS(NodeSingly<T>* List) {
    while (List != nullptr) {
        cout << List->info << " ";
        List = List->next;
    }
    cout << endl;
}
// ���������� ������
template <typename T>
struct NodeDoubLin {
    T info;
    NodeDoubLin* previous;
    NodeDoubLin* next;

};
template <typename T>
struct DoubleList {
    NodeDoubLin<T>* head;
    NodeDoubLin<T>* tail;
    DoubleList() : head(nullptr), tail(nullptr){}
};
// ���������� ��� ����������� ������
template <typename T>
void AddHeadD(DoubleList<T>& listic, T inf) {
    NodeDoubLin<T>* NewHead = new NodeDoubLin<T>;
    NewHead->previous = nullptr;
    NewHead->next = listic.head;
    NewHead->info = inf;

    if (listic.head != nullptr) {
        listic.head->previous = NewHead;
    }
    else {
        listic.tail = NewHead; // ���� ������ ����, ����� ���� ��������� �� ����� �������
    }

    listic.head = NewHead; // ��������� ��������� �� ������
}

template <typename T>
void AddTailD(DoubleList<T>& listic, T inf) {
    NodeDoubLin<T>* NewTail = new NodeDoubLin<T>;
    NewTail->next = nullptr;  // ����� ����� ��������� �� nullptr
    NewTail->info = inf;      // ������������� ��������

    if (listic.tail != nullptr) {  // ���� ������ �� ������
        NewTail->previous = listic.tail; // ��������� ���������� �������
        listic.tail->next = NewTail;     // ��������� ������ ����� � �����
    }
    else {
        NewTail->previous = nullptr; // ���� ������ ������, ����� ������� �� ����� �����������
        listic.head = NewTail;        // ����� ������� ���������� � �������, � �������
    }

    listic.tail = NewTail; // ��������� ��������� �� �����
}

//�������� ������ ������
template <typename T>
void RemoveHeadD(DoubleList<T>& listic) {
    if (listic.head != nullptr) {
        NodeDoubLin<T>* temp = listic.head;
        listic.head = listic.head->next;
        if (listic.head != nullptr) {//���� ������ ���� ����������
            listic.head->previous = nullptr;
        }
        else {
            listic.tail = nullptr;//��� ������ ���� ������
        }
        delete temp; //������� ������ �������
    }
}

//�������� ������ ������
template <typename T>
void RemoveTailD(DoubleList<T>& listic) {
    if (listic.tail == nullptr) return; //���� ������ ����

    NodeDoubLin<T>* temp = listic.tail;
    listic.tail = listic.tail->previous;

    if (listic.tail != nullptr) {//���� � ������ ����� ������ ��������
        listic.tail->next = nullptr;
    }
    else {
        listic.head = nullptr;//���� ������ ���� ������
    }

    delete temp;//������� ��������� �������
}

//�������� �� ��������
template <typename T>
void RemoveByValueD(DoubleList<T>& listic, T value) {
    NodeDoubLin<T>* temp = listic.head;

    while (temp != nullptr) {
        if (temp->info == value) {
            //���� ������� ������
            if (temp == listic.head) {
                RemoveHeadD(listic);
            }
            //���� ������� �����
            else if (temp == listic.tail) {
                RemoveTailD(listic);
            }
            //e������ ���� �� ��������
            else {
                temp->previous->next = temp->next;//��������� ���������� � ���������
                if (temp->next != nullptr) {//���� ���� ��������� ����
                    temp->next->previous = temp->previous;//��������� ��������� � ����������
                }
                delete temp;//������ ����
            }
            return;
        }
        temp = temp->next;
    }
}

//����� �� ��������
template <typename T>
NodeDoubLin<T>* FindD(DoubleList<T>& listic, T value) {
    NodeDoubLin<T>* temp = listic.head;
    while (temp != nullptr) {
        if (temp->info == value) {
            return temp;//����� ����
        }
        temp = temp->next;
    }
    return nullptr;//�� ����� ����
}

//g����� ������
template <typename T>
void PrintListD(const DoubleList<T>& listic) {
    NodeDoubLin<T>* temp = listic.head;
    while (temp != nullptr) {
        cout << temp->info << " ";
        temp = temp->next;
    }
    cout << endl;
}

#endif // LISTS_H_INCLUDED
