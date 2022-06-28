#pragma once
#include <limits.h>

typedef struct _Vertex {
    int next[256];
    short int patternId;
    int suffixLink;
    uint32_t parent;
    uint8_t symbolTo;
    char leaf;
} Vertex;

typedef struct _Vector {
    Vertex *array;
    int size;
    int capacity;
} Vector;

typedef struct _Pattern {
    uint8_t *string;
    uint8_t *stringToReplace;
    int size;
} Pattern;

typedef struct _Queue {
    uint8_t *data;  // указатель на данные
    int low;        // указатель на нижнюю границу
    int high;       // указатель на верхнюю границу
    int count;      // количество элементов в очереди
    int max;        // максимальное количество элементов
} Queue;

typedef struct _PatternArray {
    Pattern *array;
    int size;
    int capacity;
} PatternArray;