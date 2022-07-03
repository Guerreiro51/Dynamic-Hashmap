/**
 * @file hashmap.h
 * @brief Implements a dynamic hashmap with a cstr as key
 *
 * Based on https://github.com/sheredom/hashmap.h
 */
#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>
#define HASHMAP_MAX_CHAIN_LENGTH 8

typedef struct {
    const char* key;
    unsigned keyLen;
    bool used;
    void* data;
} HashmapElement;

typedef struct {
    unsigned tableSize;
    unsigned size;
    HashmapElement* data;
} Hashmap;

int hashmapCreate(const unsigned initialSize, Hashmap* const outHashmap);
int hashmapPut(Hashmap* const hashmap, const char* const key, const unsigned len, void* const value);
void* hashmapGet(const Hashmap* const hashmap, const char* const key, const unsigned len);
int hashmapRemove(Hashmap* const hashmap, const char* const key, const unsigned len);
void hashmapDestroy(Hashmap* const hashmap);

bool hashmapCheckIfMatch(const HashmapElement* const element, const char* const key, const unsigned len);

unsigned hashmapCRC32(const char* const s, const unsigned len);
unsigned hashmapStringHasher(const Hashmap* const m, const char* const keystring, const unsigned len);
bool hashmapGetBucket(const Hashmap* const m, const char* const key, const unsigned len, unsigned* const out_index);

int hashmapApplyIterator(Hashmap* const hashmap, int (*f)(void* const, HashmapElement* const), void* const context);
int hashmapRehashIterator(void* const newHashmap, HashmapElement* const element);

int hashmapExpand(Hashmap* const m);

int logFreeIterator(void* const context, HashmapElement* const elem);
void hashmapDestroyWithOwnership(Hashmap* const hashmap, int (*iterator)(void* const, HashmapElement* const));

#endif  // HASHMAP_H