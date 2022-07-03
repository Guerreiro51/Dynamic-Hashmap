#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header/hashmap.h"

enum dataType { INTEGER,
                REAL };
union v {
    int integer;
    float real;
};

struct _var {
    bool isVar;
    int type;  // 0: int, 1: n_real
    union v value;
    unsigned scope;
    unsigned addr;
    char name[32];
};

struct _proc {
    bool isVar;
    int returnType;  // 0: int, 1: n_real
    unsigned addr;
    char name[32];
    int* argsTypes;
    char** argsNames;
};

int compilerLogFreeIterator(void* const context, HashmapElement* const elem) {
    printf("%s has been freed!\n", elem->key);
    if (*(bool*)elem->data) {
        struct _var* e = (struct _var* const)elem->data;
        free(e);
    } else {
        struct _proc* e = (struct _proc* const)elem->data;
        free(e);
    }
    return -1;
}
int insertVar(Hashmap* symbolTable, char name[], int type, union v value, unsigned scope) {
    static unsigned addr = UINT_MAX;
    struct _var* var = (struct _var*)malloc(sizeof(struct _var));
    if (!var)
        return 1;
    var->type = type;
    var->value = value;
    var->scope = scope;
    var->isVar = true;
    var->addr = ++addr;
    strcpy(var->name, name);

    return hashmapPut(symbolTable, var->name, strlen(var->name), var);
}

int insertProc(Hashmap* symbolTable, char name[], int returnType, unsigned addr) {
    struct _proc* proc = (struct _proc*)malloc(sizeof(struct _proc));
    proc->returnType = returnType;
    proc->isVar = false;
    proc->addr = addr;
    strcpy(proc->name, name);

    hashmapPut(symbolTable, proc->name, strlen(proc->name), proc);
}

void showSymbolTableElement(void* const elem) {
    if (*(bool*)elem) {  // var
        struct _var* e = (struct _var* const)elem;
        if (e->type == 0)  // integer
            printf("Var %s (scope %d, addr %d) value %d\n", e->name, e->scope, e->addr, e->value.integer);
        else if (e->type == 1)  // real
            printf("Var %s (scope %d, addr %d) value %f\n", e->name, e->scope, e->addr, e->value.real);
    } else {  // proc
        struct _proc* e = (struct _proc* const)elem;
        printf("Proc %s (return type %s)\n", e->name, ((e->returnType) ? "INT" : "REAL"));
    }
}

int main() {
    Hashmap hashmap;
    if (hashmapCreate(2, &hashmap)) {
        printf("Couldn't create the hashmap!\n");
        return 0;
    }

    /********************************************************************************/
    // INSERT {life: 42}, {test: 69}, {test2: 420}
    int meaningOfLife = 42;
    if (hashmapPut(&hashmap, "life", strlen("life"), &meaningOfLife)) {
        printf("Couldn't put element!\n");
        return 0;
    }
    int test = 69;
    if (hashmapPut(&hashmap, "test", strlen("test"), &test)) {
        printf("Couldn't put element!\n");
        return 0;
    }
    int test2 = 420;
    if (hashmapPut(&hashmap, "test2", strlen("test2"), &test2)) {
        printf("Couldn't put element!\n");
        return 0;
    }

    // look for life, replace its value with 69
    void* const element = hashmapGet(&hashmap, "life", strlen("life"));
    if (!element) {
        printf("Couldn't find element!\n");
        return 0;
    }
    printf("Found element %d\n", *(int* const)element);
    if (hashmapPut(&hashmap, "life", strlen("life"), &test)) {
        printf("Couldn't put element!\n");
        return 0;
    }
    void* const element2 = hashmapGet(&hashmap, "life", strlen("life"));
    if (!element) {
        printf("Couldn't find element!\n");
        return 0;
    }
    printf("Found element %d\n", *(int* const)element2);

    // look for test, remove it, look for it again
    void* const element3 = hashmapGet(&hashmap, "test", strlen("test"));
    if (!element) {
        printf("Couldn't find element!\n");
        return 0;
    }
    printf("Found element %d\n", *(int* const)element3);
    hashmapRemove(&hashmap, "test", strlen("test"));
    void* const element4 = hashmapGet(&hashmap, "test", strlen("test"));
    if (element4) {
        printf("Could find element %d after remove!\n", *(int* const)element4);
        return 0;
    } else {
        printf("Removed element test!\n");
    }

    hashmapDestroy(&hashmap);
    /********************************************************************************/
    // if data needed to be freed
    Hashmap hashmapWithOwnership;
    if (hashmapCreate(2, &hashmapWithOwnership)) {
        printf("Couldn't create the hashmap!\n");
        return 0;
    }
    char* meaningOfLifeStr = (char*)malloc(16 * sizeof(char));
    strcpy(meaningOfLifeStr, "42 toalha");
    if (hashmapPut(&hashmapWithOwnership, "life", strlen("life"), meaningOfLifeStr)) {
        printf("Couldn't put element!\n");
        return 0;
    }
    void* const element5 = hashmapGet(&hashmapWithOwnership, "life", strlen("life"));
    if (!element) {
        printf("Couldn't find element!\n");
        return 0;
    }
    printf("Found element %s\n", (char* const)element5);
    hashmapDestroyWithOwnership(&hashmapWithOwnership, logFreeIterator);

    /********************************************************************************/
    /* SIMULATION OF COMPILER SYMBOL TABLE BEHAVIOUR */
    Hashmap symbolTable;
    hashmapCreate(2, &symbolTable);

    insertVar(&symbolTable, "intVar", INTEGER, (union v)4, 0);
    insertVar(&symbolTable, "floatVar", REAL, (union v)3.14f, 3);
    insertProc(&symbolTable, "proc", 0, 0);

    showSymbolTableElement(hashmapGet(&symbolTable, "intVar", strlen("intVar")));
    showSymbolTableElement(hashmapGet(&symbolTable, "floatVar", strlen("floatVar")));
    showSymbolTableElement(hashmapGet(&symbolTable, "proc", strlen("proc")));

    hashmapDestroyWithOwnership(&symbolTable, compilerLogFreeIterator);
}