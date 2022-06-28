#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "structs.h"
#include "stdbool.h"
#include "string.h"

#define INPUT_FILE_NAME "in.txt"
#define OUTPUT_FILE_NAME "out.txt"

typedef enum {
    REALLOC_OUT_OF_MEMORY,
    MALLOC_OUT_OF_MEMORY,
    INVALID_ARGUMENT,
    INVALID_TEMPLATE,
    QUEUE_IS_FULL,
    SUCCESS
} opCodes;

char *opMsg[] = {
        "Could not reallocate memory. Trying to use malloc...",
        "Unable to reallocate memory",
        "bad argument exception",
        "bad template",
        "Not enough queue size",
        "Success"
};

void resize(Vector *vector) {
    Vertex *tmp = realloc(vector->array, 2 * sizeof(Vertex) * vector->capacity);
    if (!tmp) {
        fprintf(stderr, "%s\n", opMsg[REALLOC_OUT_OF_MEMORY]);
        tmp = malloc(2 * sizeof(Vertex) * vector->capacity);
        if (!tmp) {
            fprintf(stderr, "%s for vector resize: capacity = %d, bytes = %d\n", opMsg[MALLOC_OUT_OF_MEMORY],
                    vector->capacity * 2, 2 * sizeof(Vertex) * vector->capacity);
            memcpy(tmp, vector->array, sizeof(Pattern) * vector->capacity);
            free(vector->array);
            vector->array = tmp;
            return;
        }
        return;
    }
    vector->array = tmp;
    vector->capacity = vector->capacity * 2;
}


void push(Vector *vector, Vertex *item) {
    if (vector->size >= vector->capacity) {
        resize(vector);
    }
    vector->array[vector->size] = *item;
    vector->size++;
}

Vertex *newVertex(bool isLeaf, int patternId, uint32_t parent, uint8_t symbolTo) {
    if (isLeaf != 0 & isLeaf != 1) {
        fprintf(stderr, "%s in newVertex()\n", opMsg[INVALID_ARGUMENT]);
        return NULL;
    }
    Vertex *v = malloc(sizeof(Vertex));
    if (!v) {
        fprintf(stderr, opMsg[MALLOC_OUT_OF_MEMORY]);
        return NULL;
    }
    memset(v->next, -1, 256 * sizeof(int));
    v->leaf = (char) isLeaf;
    v->suffixLink = -1;
    v->parent = parent;
    v->symbolTo = symbolTo;
    v->patternId = patternId;
    return v;
}

Vector *newVector(int capacity) {
    Vector *v = malloc(sizeof(Vector));
    if (!v) {
        fprintf(stderr, "%s for vector\n", opMsg[MALLOC_OUT_OF_MEMORY]);
        return NULL;
    }
    if (capacity < 1) {
        capacity = 4;
    }
    Vertex *arr = malloc(sizeof(Vertex) * capacity);
    if (!arr) {
        free(v);
        fprintf(stderr, "%s for vector array\n", opMsg[MALLOC_OUT_OF_MEMORY]);
        return NULL;
    }
    v->array = arr;
    v->size = 0;
    v->capacity = capacity;
    return v;
}

Vector *bohrInit() {
    Vector *vectorTmp = newVector(0);
    Vertex *vertexTmp = newVertex(false, -1, 0, 0);
    if (vertexTmp != NULL && vectorTmp != NULL) {
        // Add root
        push(vectorTmp, vertexTmp);
        return vectorTmp;
    } else {
        // Free memory
        if (vectorTmp) {
            free(vectorTmp->array);
            free(vectorTmp);
        }
        free(vertexTmp);
        return NULL;
    }
}

void addString(Vector *bohr, uint8_t *string, int stringOrdNum) {
    uint32_t strLen = strlen((const char *) string);
    uint32_t curNum = 0;
    for (int i = 0; i < strLen; ++i) {
        if (bohr->array[curNum].next[string[i]] == -1) {
            push(bohr, newVertex(false, -1, curNum, string[i]));
            bohr->array[curNum].next[string[i]] = bohr->size - 1;
        }
        curNum = bohr->array[curNum].next[string[i]];
    }
    bohr->array[curNum].leaf = true;
    bohr->array[curNum].patternId = stringOrdNum;
}

uint32_t getSuffixLink(Vector *bohr, uint32_t curStep);

uint32_t makeStep(Vector *bohr, uint32_t curStep, uint8_t c) {
    Vertex *tmp = &(bohr->array[curStep]);
    if (tmp->next[c] != -1) {
        return tmp->next[c];
    } else {
        if (curStep == 0)
            return 0;
        return makeStep(bohr, getSuffixLink(bohr, curStep), c);
    }
}

uint32_t getSuffixLink(Vector *bohr, uint32_t curStep) {
    Vertex *tmp = &(bohr->array[curStep]);
    if (tmp->suffixLink == -1) {
        if (tmp->parent == 0 || tmp == bohr->array) {
            tmp->suffixLink = 0;
        } else {
            tmp->suffixLink = (int) makeStep(bohr, getSuffixLink(bohr, tmp->parent), tmp->symbolTo);
        }
    }
    return tmp->suffixLink;
}

void resizePatternArray(PatternArray *pArray) {
    Pattern *tmp = realloc(pArray->array, 2 * sizeof(Pattern) * pArray->capacity);
    if (!tmp) {
        fprintf(stderr, "%s\n", opMsg[REALLOC_OUT_OF_MEMORY]);
        tmp = malloc(2 * sizeof(Pattern) * pArray->capacity);
        if (!tmp) {
            fprintf(stderr, "%s for for pattern array resize: capacity = %d, bytes = %d\n", opMsg[MALLOC_OUT_OF_MEMORY],
                    pArray->capacity * 2, 2 * sizeof(Pattern) * pArray->capacity);
            return;
        }
        memcpy(tmp, pArray->array, sizeof(Pattern) * pArray->capacity);
        free(pArray->array);
        pArray->array = tmp;
    }
    pArray->array = tmp;
    pArray->capacity = pArray->capacity * 2;
}

void pushPatternArray(PatternArray *pArray, Pattern *item) {
    if (pArray->size >= pArray->capacity) {
        resizePatternArray(pArray);
    }
    pArray->array[pArray->size] = *item;
    pArray->size++;
}

Pattern *newPattern(uint8_t *string, uint8_t *stringToReplace) {
    Pattern *p = malloc(sizeof(Pattern));
    if (!p) {
        fprintf(stderr, "%s for pattern\n", opMsg[MALLOC_OUT_OF_MEMORY]);
        return NULL;
    }
    p->size = (int) strlen((const char *) string);
    uint8_t *string1 = malloc(sizeof(uint8_t) * (p->size + 1));
    if (!string1) {
        free(p);
        fprintf(stderr, "%s for pattern key string\n", opMsg[MALLOC_OUT_OF_MEMORY]);
        return NULL;
    }
    strcpy((char *) string1, (const char *) string);
    p->string = string1;

    uint8_t *string2 = malloc(sizeof(uint8_t) * (strlen((const char *) stringToReplace) + 1));
    if (!string2) {
        free(p);
        free(string1);
        fprintf(stderr, "%s for pattern value string\n", opMsg[MALLOC_OUT_OF_MEMORY]);
        return NULL;
    }
    strcpy((char *) string2, (const char *) stringToReplace);
    p->stringToReplace = string2;
    return p;
}

PatternArray *newPatternArray(int capacity) {
    PatternArray *p = malloc(sizeof(PatternArray));
    if (!p) {
        fprintf(stderr, "%s for PatternArray\n", opMsg[MALLOC_OUT_OF_MEMORY]);
        return NULL;
    }
    if (capacity < 1) {
        capacity = 4;
    }
    Pattern *arr = malloc(sizeof(Pattern) * capacity);
    if (!arr) {
        free(arr);
        fprintf(stderr, "%s for PatternArray array\n", opMsg[MALLOC_OUT_OF_MEMORY]);
        return NULL;
    }
    p->array = arr;
    p->capacity = capacity;
    p->size = 0;
    return p;
}

uint32_t parseJson(char *jsonString, uint32_t **array, uint32_t *arrSize, uint32_t capacity) {
    char *statePattern = "\"\":\"\"";
    uint32_t statePatternLen = strlen(statePattern);
    uint32_t statePatternPos = 0;
    uint32_t jsonSize = strlen(jsonString);
    uint32_t offset = 0;

    if (capacity == 0)
        capacity = jsonSize;
    *array = malloc(sizeof(uint32_t) * capacity);
    for (int i = 0; i < jsonSize; ++i) {
        while (i < jsonSize && jsonString[i] != statePattern[statePatternPos % statePatternLen]) i++;
        if (statePattern[statePatternPos % statePatternLen] == '"' && jsonString[i - 1] != '\\' && i != jsonSize) {
            (*array)[*arrSize] = i;
            if (*arrSize > 0)
                if (i - (*array)[*arrSize - 1] > offset)
                    offset = i - (*array)[*arrSize - 1];
            *arrSize += 1;
        }
        if (jsonString[i - 1] != '\\')
            statePatternPos += 1;
    }
    return offset;
}

Queue *queueInit(size_t size) {
    Queue *q = calloc(1, sizeof(Queue));
    q->data = calloc(size, sizeof(uint8_t));
    q->low = q->high = size - 1;
    q->max = size;
    return q;
}

void queue_add(Queue *q, uint8_t a) {
    if (q->count == q->max) {
        fprintf(stderr, "%s\n", opMsg[QUEUE_IS_FULL]);
        return;
    }

    q->data[q->low--] = a;
    q->count++;

    if (q->low < 0) {
        q->low = q->max - 1;
    }

}

uint16_t queue_get(Queue *q) {
    if (q->count == 0)
        return 256;
    uint8_t a = q->data[q->high--];
    q->count--;

    if (q->high < 0) {
        q->high = q->max - 1;
    }

    return a;
}

void freeQueue(Queue *q) {
    free(q->data);
    free(q);
}

void freeVector(Vector *v) {
    free(v->array);
    free(v);
}

void freePatternArray(PatternArray *pArray) {
    for (int i = 0; i < pArray->size; ++i) {
        free(pArray->array[i].string);
        free(pArray->array[i].stringToReplace);
    }
    free(pArray->array);
    free(pArray);
}

opCodes fillPatternArrayFromJSON(FILE *inFile, PatternArray **pArray, int64_t jsonStartPos, uint32_t *jsonLen) {
    uint32_t quotesInJSONCount = 0;
    uint8_t tmpSmb;
    while ((tmpSmb = getc(inFile)) != '\n' && feof(inFile) == 0) {
        if (tmpSmb == '"') quotesInJSONCount++;
    }

    *jsonLen = ftell(inFile) - jsonStartPos - 1;
    char *json = malloc(sizeof(char) * *jsonLen);

    if (!json) {
        fprintf(stderr, "%s for JSON string", opMsg[MALLOC_OUT_OF_MEMORY]);
        return MALLOC_OUT_OF_MEMORY;
    }

    fseek(inFile, jsonStartPos, SEEK_SET);
    fread(json, sizeof(char), *jsonLen, inFile);
    json[*jsonLen - 1] = '\0';

    uint32_t indexedQuotesArraySize = 0;
    uint32_t *indexedQuotesArray = NULL;
    uint32_t maximum_offset = parseJson(json, &indexedQuotesArray, &indexedQuotesArraySize, quotesInJSONCount);

    *pArray = newPatternArray(quotesInJSONCount / 4);
    if (!*pArray) {
        fprintf(stderr, "%s for JSON string", opMsg[MALLOC_OUT_OF_MEMORY]);
        free(json);
        free(indexedQuotesArray);
        return MALLOC_OUT_OF_MEMORY;
    }
    // Words init

    uint8_t tmp1[2 * maximum_offset + 2];
    int startW, endW, startD, endD;
    if (indexedQuotesArraySize > 1) {
        for (int i = 0; i < indexedQuotesArraySize - 3; i += 4) {
            startW = indexedQuotesArray[i], endW = indexedQuotesArray[i + 1];
            startD = indexedQuotesArray[i + 2], endD = indexedQuotesArray[i + 3];
            memcpy(tmp1 + 1, json + startW + 1, endW - startW - 1);
            tmp1[0] = '{';
            tmp1[endW - startW] = '}';
            tmp1[endW - startW + 1] = '\0';
            memcpy(tmp1 + endW - startW + 2, json + startD + 1, endD - startD - 1);
            tmp1[endW - startW + 2 + endD - startD - 1] = '\0';
//            printf("%s \n\n%s\n\n", tmp1, tmp1 + endW - startW + 2);
            pushPatternArray(*pArray, newPattern(tmp1, tmp1 + endW - startW + 2));
        }
    }
    free(json);
    return SUCCESS;
}

uint32_t getInStringLen(FILE *inFile, uint32_t jsonStartPos, uint32_t jsonLen) {
    uint32_t strLen = 0;
    while (getc(inFile) != EOF) strLen++;
    fseek(inFile, jsonStartPos + jsonLen, SEEK_SET);
    while (getc(inFile) != '\n' && feof(inFile) == 0);
    return strLen;
}

void printCharWithConditions(FILE *outFile, Vector *bohr, PatternArray *pArray, int *index, uint32_t curStep) {
    if (pArray->array[bohr->array[curStep].patternId].stringToReplace[*index] != '\\' ||
        pArray->array[bohr->array[curStep].patternId].stringToReplace[*index + 1] != '"') {
        if (pArray->array[bohr->array[curStep].patternId].stringToReplace[*index] == '\\') {
            if (pArray->array[bohr->array[curStep].patternId].stringToReplace[*index + 1] == 'n') {
                *index += 1;
                putc('\n', outFile);
            } else if (pArray->array[bohr->array[curStep].patternId].stringToReplace[*index + 1] == 't') {
                *index += 1;
                putc('\t', outFile);
            } else putc('\\', outFile);
        } else {
            putc(pArray->array[bohr->array[curStep].patternId].stringToReplace[*index], outFile);
        }
    }
}

void findAndReplace(FILE *inFile, FILE *outFile, Vector *bohr, uint32_t strLen, PatternArray *pArray) {
    // Getting max length in pArray
    uint32_t maxLen = 0;
    uint8_t tmpSmb;
    for (int i = 0; i < pArray->size; ++i) {
        if (maxLen < pArray->array[i].size) {
            maxLen = pArray->array[i].size;
        }
    }

    uint32_t curStep = 0;
    Queue *queueArr = queueInit(maxLen);
    int *replacementCounterArray = calloc(pArray->size, sizeof(int));
    for (int i = 0; i < strLen; ++i) {
        tmpSmb = (uint8_t) getc(inFile);
        curStep = makeStep(bohr, curStep, tmpSmb);
        if (bohr->array[curStep].leaf) {
            replacementCounterArray[bohr->array[curStep].patternId]++;
            while (queueArr->count - (pArray->array[bohr->array[curStep].patternId].size - 1)) {
                fprintf(outFile, "%c", queue_get(queueArr));
            }
            while (queueArr->count) {
                queue_get(queueArr);
            }
            uint32_t tmpStrLen = strlen(
                    (const char *) pArray->array[bohr->array[curStep].patternId].stringToReplace);
            for (int j = 0; j < tmpStrLen; ++j) {
                printCharWithConditions(outFile, bohr, pArray, &j, curStep);
            }
        } else {
            if (queueArr->count == queueArr->max) {
                if (queueArr->count > 0)
                    putc(queue_get(queueArr), outFile);
            }
            queue_add(queueArr, tmpSmb);
        }
    }
    while (queueArr->count) {
        fprintf(outFile, "%c", queue_get(queueArr));
    }
    for (int i = 0; i < pArray->size; ++i) {
        if (replacementCounterArray[i] == 0) {
            fclose(outFile);
            outFile = fopen(OUTPUT_FILE_NAME, "wb");
            fprintf(outFile, opMsg[INVALID_TEMPLATE]);
            fclose(outFile);
            break;
        }
    }
    freeQueue(queueArr);
}

int main() {
    FILE *inFile = fopen(INPUT_FILE_NAME, "r");
    int N;
    fscanf(inFile, "%d", &N);

    Vector *bohr = bohrInit();
    if (!bohr) {
        fclose(inFile);
        return 0;
    }

    // Getting json start position
    while (getc(inFile) != '\n' && feof(inFile) == 0);
    int64_t jsonStartPos = ftell(inFile);
    uint32_t jsonLen = 0;

    PatternArray *pArray = NULL;
    if (SUCCESS != fillPatternArrayFromJSON(inFile, &pArray, jsonStartPos, &jsonLen)) {
        freeVector(bohr);
        fclose(inFile);
        return 0;
    }

    int strLen = getInStringLen(inFile, jsonStartPos, jsonLen);

    if (feof(inFile) != 0) {
        return 0;
    }

    FILE *outFile = fopen(OUTPUT_FILE_NAME, "wb");
    if (pArray->size == 0) {
        for (int i = 0; i < strLen; ++i) putc(getc(inFile), outFile);
    } else {
        // Fill bohr
        for (int i = 0; i < pArray->size; ++i) {
            addString(bohr, pArray->array[i].string, i);
        }

        // Start finding
        findAndReplace(inFile, outFile, bohr, strLen, pArray);
    }
    fclose(inFile);
    fclose(outFile);
    freeVector(bohr);
    freePatternArray(pArray);
}
