#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "cda.h"

struct cda {
    void **store;
    int head;
    int size;
    int capacity;
    void (*display)(void *, FILE *);
    void (*free)(void *);
    int debug;
};

CDA *newCDA(void) {
    CDA *items = malloc(sizeof(CDA));
    items->debug = 0;
    items->capacity= 1;
    items->size = 0;
    items->store = malloc(sizeof(void *));
    items->display = NULL;
    items->head = 0;
    items->free = NULL;
    assert(items != 0);
    return items;
}
void setCDAdisplay(CDA *items, void (*display)(void *,FILE *)){
    items->display = display;
}

extern void setCDAfree(CDA *items, void (*free)(void *)) {
    items->free = free;
}

void growCDA(CDA *items){
    void **newStore = malloc(sizeof(void *) * 2 * items->capacity);
    for (int i = 0; i < items->size; i++){
        newStore[i] = items->store[(items->head + i) % items->capacity];
    }
    items->capacity *= 2;
    free(items->store);
    items->store = newStore;
    items->head = 0;
}

void insertCDA(CDA *items,int index,void *value){
    assert(index >= 0 && index <= items->size);

    if (items->size >= items->capacity){
        growCDA(items);
    }

    if (index == items->size && items->size < items->capacity){
        items->size += 1;
        setCDA(items,index,value);
    }

    else if (index == 0 && items->size < items->capacity){
        items->head = ((items->head - 1 + items->capacity) % items->capacity);
        items->store[items->head] = value;
        items->size += 1;
    }

    else {
        if (index >= items->size / 2) {
            for (int i = (items->size - 1); i >= index; i--) {
                setCDA(items, i + 1, getCDA(items, i));
            }
            setCDA(items, index, value);
        }

        else if (index < items->size / 2) {
            for (int i = 0; i <= index; i++) {
                setCDA(items, i-1, getCDA(items,i));
            }
            setCDA(items,index,value);
        }
    }
    assert(items->store != 0);
}

void *removeCDA(CDA *items,int index) {
    assert(index >= 0 && index < items->size && items->size > 0);

    void *removedItem = getCDA(items,index);

    if (index == 0){
        items->head = (items->head + 1 + items->capacity) % items->capacity;
        items->size -= 1;
    }

    else if (index == (items->size - 1)){
        items->size -= 1;
    }

    else if (index < items->size && index > 0){
        if (index >= items->size / 2) {
            for (int i = index; i <= (items->size - 2); i++) {
                setCDA(items, i, getCDA(items, i + 1));
            }
        }

        else if (index < items->size / 2) {
            for (int i = index; i >= 1; i--) {
                setCDA(items,i, getCDA(items, i-1));
            }
            items->head = (items->head + items->capacity + 1) % items->capacity;
        }

        items->size -= 1;
    }

    if (items->size < (items->capacity / 4) && (items->capacity / 2) >= 1){
        void **newStore = malloc(sizeof(void *) * (items->capacity / 2));
        for (int i = 0; i < items->size; i++){
            newStore[i] = items->store[(items->head + i) % items->capacity];
        }
        items->capacity = items->capacity / 2;
        free(items->store);
        items->store = newStore;
        items->head = 0;
    }

    if (items->size == 0) {
        items->head = 0;
        items->capacity = 1;
    }

    return removedItem;
}

void unionCDA(CDA *recipient,CDA *donor){
    for (int i = 0; i < donor->size; i++){
        insertCDA(recipient, recipient->size, getCDA(donor,i));
    }
    donor->size = 0;
    donor->capacity = 1;
    donor->head = 0;
    donor->store = realloc(donor->store, sizeof(*donor->store) * donor->capacity);
}

void *getCDA(CDA *items,int index){
    assert(index >= 0 && index < items->size);
    int actualIndex = ((items->head + index) % items->capacity);
    return items->store[actualIndex];
}

void *setCDA(CDA *items,int index,void *value){
    assert(index >= -1 && index <= items->size);

    int actualIndex;

    if (index == -1){
        insertCDA(items,0,value);
        return 0;
    }

    else if (index == items->size){
        insertCDA(items,index,value);
        actualIndex = ((items->head + items->size - 1) %  items->capacity);
        return 0;
    }

    else {
        actualIndex = ((items->head + index) % items->capacity);
        items->store[actualIndex] = value;
        return items->store[actualIndex];
    }
}

int  sizeCDA(CDA *items){
    return items->size;
}

void displayCDA(CDA *items,FILE *fp){
    if (items->display == NULL) {
        if (items->size == 0 && items->debug == 0){
            fprintf(fp,"()");
        }
        else if (items->size == 0 && items->debug > 0){
            int space = items->capacity - items->size;
            fprintf(fp,"((%d))",space);
        }
        else if (items->size > 0){
            fprintf(fp,"(");
            if (items->debug == 0){
                for (int i = 0; i < (items->size -1); i++){
                    fprintf(fp,"@%p,",getCDA(items,i));
                }
                fprintf(fp,"@%p",getCDA(items,items->size - 1));
            }
            else if (items->debug > 0){
                int space = items->capacity - items->size;
                for (int i = 0; i < items->size; i++){
                    fprintf(fp,"@%p,",getCDA(items,i));
                }
                fprintf(fp,"(%d)",space);
            }
            fprintf(fp,")");
        }
    }

    else {
        if (items->size == 0 && items->debug == 0){
            fprintf(fp,"()");
        }
        if (items->size == 0 && items->debug > 0){
            int space = items->capacity - items->size;
            fprintf(fp,"((%d))",space);
        }
        if (items->size > 0){
            fprintf(fp,"(");
            if (items->debug == 0) {
                for (int i = 0; i < (items->size -1); i++) {
                    items->display(getCDA(items, i), fp);
                    fprintf(fp, ",");
                }
                items->display(getCDA(items,items->size - 1),fp);
            }
            if (items->debug > 0){
                int space = items->capacity - items->size;
                for (int i = 0; i < items->size; i++){
                    items->display(getCDA(items,i),fp);
                    fprintf(fp,",");
                }
                fprintf(fp,"(%d)",space);
            }
            fprintf(fp,")");
        }
    }
}

int debugCDA(CDA *items,int level){
    int oldDebug = items->debug;
    items->debug = level;
    return oldDebug;
}

void freeCDA(CDA *items){
    if (items->free != NULL) {
        for (int i = 0; i < items->size; i++){
            items->free(getCDA(items,i));
        }
    }
    free(items->store);
    free(items);
}