/**
 * @file hashmap.c
 * @brief Implements a dynamic hashmap with a cstr as key
 *
 * Based on https://github.com/sheredom/hashmap.h
 */

#include "../header/hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Create a hashmap
 *
 * @param initialSize The initial size of the hashmap. Must be a power of two
 * @param outHashmap The storage for the created hashmap
 * @return int 0 if sucess 1 if fail
 */
int hashmapCreate(const unsigned initialSize, Hashmap* const outHashmap) {
    outHashmap->tableSize = initialSize;
    outHashmap->size = 0;

    // check if non zero power of two
    if (initialSize == 0 || ((initialSize & (initialSize - 1)) != 0)) {
        return 1;
    }

    outHashmap->data = (HashmapElement*)calloc(initialSize, sizeof(HashmapElement));
    if (!outHashmap->data) {
        return 1;
    }

    return 0;
}

/**
 * @brief Put an element into the hashmap
 *
 * @param hashmap The hashmap to insert into
 * @param key The string key to use
 * @param len The length of the string key
 * @param value The value to insert
 * @return int 0 if sucess 1 if fail
 */
int hashmapPut(Hashmap* const hashmap, const char* const key, const unsigned len, void* const value) {
    // find a bucket to put the value
    // expand the hashmap until it can find a suitable bucket
    unsigned int outIndex;
    while (!hashmapGetBucket(hashmap, key, len, &outIndex)) {
        if (hashmapExpand(hashmap)) {
            return 1;
        }
    }

    // put the value
    hashmap->data[outIndex].data = value;
    hashmap->data[outIndex].key = key;
    hashmap->data[outIndex].keyLen = len;

    // if key was not used yet, set to used and increase the size
    if (!hashmap->data[outIndex].used) {
        hashmap->data[outIndex].used = true;
        hashmap->size++;
    }

    return 0;
}

/**
 * @brief Get an element from the hashmap
 *
 * @param hashmap The hashmap to get from
 * @param key The string key to use
 * @param len The length of the string key
 * @return void* The previously set element, or NULL if none exists
 */
void* hashmapGet(const Hashmap* const hashmap, const char* const key, const unsigned len) {
    // find a bucket
    unsigned int curr = hashmapStringHasher(hashmap, key, len);

    // linear probing, if necessary
    for (unsigned int i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
        if (hashmap->data[curr].used) {
            if (hashmapCheckIfMatch(&hashmap->data[curr], key, len)) {
                return hashmap->data[curr].data;
            }
        }
        curr = (curr + 1) % hashmap->tableSize;
    }

    // not found
    return NULL;
}

/**
 * @brief Removes a key from the hashmap
 *
 * @param hashmap The hashmap to remove from
 * @param key The string key to use
 * @param len The length of the string key
 * @return int 0, if it found and removed it 1 otherwise
 */
int hashmapRemove(Hashmap* const hashmap, const char* const key, const unsigned len) {
    // find a bucket
    unsigned int curr = hashmapStringHasher(hashmap, key, len);

    // Linear probing, if necessary
    for (unsigned int i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
        if (hashmap->data[curr].used) {
            if (hashmapCheckIfMatch(&hashmap->data[curr], key, len)) {
                // Blank out everything
                memset(&hashmap->data[curr], 0, sizeof(HashmapElement));
                hashmap->size--;
                return 0;
            }
        }
        curr = (curr + 1) % hashmap->tableSize;
    }

    return 1;
}

/**
 * @brief Destroy the hashmap
 *
 * @param hashmap The hashmap to destroy
 */
void hashmapDestroy(Hashmap* const hashmap) {
    free(hashmap->data);
    memset(hashmap, 0, sizeof(Hashmap));
}

/**
 * @brief Check if the keys for two elements are the same. Used to detect collisions
 *
 * @param element The element to be checked against
 * @param key The key to check for
 * @param len The length of the key to check for
 * @return int If the keys are the same
 */
bool hashmapCheckIfMatch(const HashmapElement* const element, const char* const key, const unsigned len) {
    return (element->keyLen == len) && (memcmp(element->key, key, len) == 0);
}

/**
 * @brief Calculates the CRC32 for a string
 *
 * @param s The string
 * @param len The length of the string
 * @return unsigned The CRC32 value for the string
 */
unsigned hashmapCRC32(const char* const s, const unsigned len) {
    static const unsigned crc32tab[] = {
        0x00000000U, 0xF26B8303U, 0xE13B70F7U, 0x1350F3F4U, 0xC79A971FU,
        0x35F1141CU, 0x26A1E7E8U, 0xD4CA64EBU, 0x8AD958CFU, 0x78B2DBCCU,
        0x6BE22838U, 0x9989AB3BU, 0x4D43CFD0U, 0xBF284CD3U, 0xAC78BF27U,
        0x5E133C24U, 0x105EC76FU, 0xE235446CU, 0xF165B798U, 0x030E349BU,
        0xD7C45070U, 0x25AFD373U, 0x36FF2087U, 0xC494A384U, 0x9A879FA0U,
        0x68EC1CA3U, 0x7BBCEF57U, 0x89D76C54U, 0x5D1D08BFU, 0xAF768BBCU,
        0xBC267848U, 0x4E4DFB4BU, 0x20BD8EDEU, 0xD2D60DDDU, 0xC186FE29U,
        0x33ED7D2AU, 0xE72719C1U, 0x154C9AC2U, 0x061C6936U, 0xF477EA35U,
        0xAA64D611U, 0x580F5512U, 0x4B5FA6E6U, 0xB93425E5U, 0x6DFE410EU,
        0x9F95C20DU, 0x8CC531F9U, 0x7EAEB2FAU, 0x30E349B1U, 0xC288CAB2U,
        0xD1D83946U, 0x23B3BA45U, 0xF779DEAEU, 0x05125DADU, 0x1642AE59U,
        0xE4292D5AU, 0xBA3A117EU, 0x4851927DU, 0x5B016189U, 0xA96AE28AU,
        0x7DA08661U, 0x8FCB0562U, 0x9C9BF696U, 0x6EF07595U, 0x417B1DBCU,
        0xB3109EBFU, 0xA0406D4BU, 0x522BEE48U, 0x86E18AA3U, 0x748A09A0U,
        0x67DAFA54U, 0x95B17957U, 0xCBA24573U, 0x39C9C670U, 0x2A993584U,
        0xD8F2B687U, 0x0C38D26CU, 0xFE53516FU, 0xED03A29BU, 0x1F682198U,
        0x5125DAD3U, 0xA34E59D0U, 0xB01EAA24U, 0x42752927U, 0x96BF4DCCU,
        0x64D4CECFU, 0x77843D3BU, 0x85EFBE38U, 0xDBFC821CU, 0x2997011FU,
        0x3AC7F2EBU, 0xC8AC71E8U, 0x1C661503U, 0xEE0D9600U, 0xFD5D65F4U,
        0x0F36E6F7U, 0x61C69362U, 0x93AD1061U, 0x80FDE395U, 0x72966096U,
        0xA65C047DU, 0x5437877EU, 0x4767748AU, 0xB50CF789U, 0xEB1FCBADU,
        0x197448AEU, 0x0A24BB5AU, 0xF84F3859U, 0x2C855CB2U, 0xDEEEDFB1U,
        0xCDBE2C45U, 0x3FD5AF46U, 0x7198540DU, 0x83F3D70EU, 0x90A324FAU,
        0x62C8A7F9U, 0xB602C312U, 0x44694011U, 0x5739B3E5U, 0xA55230E6U,
        0xFB410CC2U, 0x092A8FC1U, 0x1A7A7C35U, 0xE811FF36U, 0x3CDB9BDDU,
        0xCEB018DEU, 0xDDE0EB2AU, 0x2F8B6829U, 0x82F63B78U, 0x709DB87BU,
        0x63CD4B8FU, 0x91A6C88CU, 0x456CAC67U, 0xB7072F64U, 0xA457DC90U,
        0x563C5F93U, 0x082F63B7U, 0xFA44E0B4U, 0xE9141340U, 0x1B7F9043U,
        0xCFB5F4A8U, 0x3DDE77ABU, 0x2E8E845FU, 0xDCE5075CU, 0x92A8FC17U,
        0x60C37F14U, 0x73938CE0U, 0x81F80FE3U, 0x55326B08U, 0xA759E80BU,
        0xB4091BFFU, 0x466298FCU, 0x1871A4D8U, 0xEA1A27DBU, 0xF94AD42FU,
        0x0B21572CU, 0xDFEB33C7U, 0x2D80B0C4U, 0x3ED04330U, 0xCCBBC033U,
        0xA24BB5A6U, 0x502036A5U, 0x4370C551U, 0xB11B4652U, 0x65D122B9U,
        0x97BAA1BAU, 0x84EA524EU, 0x7681D14DU, 0x2892ED69U, 0xDAF96E6AU,
        0xC9A99D9EU, 0x3BC21E9DU, 0xEF087A76U, 0x1D63F975U, 0x0E330A81U,
        0xFC588982U, 0xB21572C9U, 0x407EF1CAU, 0x532E023EU, 0xA145813DU,
        0x758FE5D6U, 0x87E466D5U, 0x94B49521U, 0x66DF1622U, 0x38CC2A06U,
        0xCAA7A905U, 0xD9F75AF1U, 0x2B9CD9F2U, 0xFF56BD19U, 0x0D3D3E1AU,
        0x1E6DCDEEU, 0xEC064EEDU, 0xC38D26C4U, 0x31E6A5C7U, 0x22B65633U,
        0xD0DDD530U, 0x0417B1DBU, 0xF67C32D8U, 0xE52CC12CU, 0x1747422FU,
        0x49547E0BU, 0xBB3FFD08U, 0xA86F0EFCU, 0x5A048DFFU, 0x8ECEE914U,
        0x7CA56A17U, 0x6FF599E3U, 0x9D9E1AE0U, 0xD3D3E1ABU, 0x21B862A8U,
        0x32E8915CU, 0xC083125FU, 0x144976B4U, 0xE622F5B7U, 0xF5720643U,
        0x07198540U, 0x590AB964U, 0xAB613A67U, 0xB831C993U, 0x4A5A4A90U,
        0x9E902E7BU, 0x6CFBAD78U, 0x7FAB5E8CU, 0x8DC0DD8FU, 0xE330A81AU,
        0x115B2B19U, 0x020BD8EDU, 0xF0605BEEU, 0x24AA3F05U, 0xD6C1BC06U,
        0xC5914FF2U, 0x37FACCF1U, 0x69E9F0D5U, 0x9B8273D6U, 0x88D28022U,
        0x7AB90321U, 0xAE7367CAU, 0x5C18E4C9U, 0x4F48173DU, 0xBD23943EU,
        0xF36E6F75U, 0x0105EC76U, 0x12551F82U, 0xE03E9C81U, 0x34F4F86AU,
        0xC69F7B69U, 0xD5CF889DU, 0x27A40B9EU, 0x79B737BAU, 0x8BDCB4B9U,
        0x988C474DU, 0x6AE7C44EU, 0xBE2DA0A5U, 0x4C4623A6U, 0x5F16D052U,
        0xAD7D5351U};

    unsigned crc32val = 0;
    for (unsigned i = 0; i < len; i++)
        crc32val = crc32tab[(unsigned char)crc32val ^ (unsigned char)s[i]] ^ (crc32val >> 8);

    return crc32val;
}

/**
 * @brief Returns a hash value for a string
 *
 * @param hashmap The hashmap for which the hash is being generated
 * @param keystring The key string
 * @param len The length of the key string
 * @return unsigned the generated hash value
 */
unsigned hashmapStringHasher(const Hashmap* const hashmap, const char* const keystring, const unsigned len) {
    unsigned key = hashmapCRC32(keystring, len);

    // Robert Jenkins' 32 bit Mix Function
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);

    // Knuth's Multiplicative Method
    key = (key >> 3) * 2654435761;
    return key % hashmap->tableSize;
}

/**
 * @brief Gets a bucket index for an element
 *
 * @param hashmap The hashmap to look for the bucket
 * @param key The key to insert
 * @param len The length of the key to insert
 * @param outIndex The output index
 * @return bool If a bucket was found
 */
bool hashmapGetBucket(const Hashmap* const hashmap, const char* const key, const unsigned len, unsigned* const outIndex) {
    /* If full, return immediately */
    if (hashmap->size >= hashmap->tableSize) {
        return false;
    }

    // find original index
    unsigned int start = hashmapStringHasher(hashmap, key, len);

    // linear probe to check if we've already insert the element
    int totalUsed = 0;
    unsigned int curr = start;
    for (unsigned int i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
        bool used = hashmap->data[curr].used;
        totalUsed += used;
        if (used && hashmapCheckIfMatch(&hashmap->data[curr], key, len)) {
            *outIndex = curr;
            return true;
        }
        curr = (curr + 1) % hashmap->tableSize;
    }

    // linear probe again (if there was at least one empty entry),
    // this time knowing the element isn't there.
    // return the first not used bucket
    if (HASHMAP_MAX_CHAIN_LENGTH > totalUsed) {
        curr = start;
        for (unsigned int i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
            if (!hashmap->data[curr].used) {
                *outIndex = curr;
                return true;
            }

            curr = (curr + 1) % hashmap->tableSize;
        }
    }

    // could not find empty bucket within HASHMAP_MAX_CHAIN_LENGTH
    return false;
}

/**
 * @brief Iterate over all the elements in a hashmap applying the function f.
 * If f returns -1, remove the item.
 * If f returns 0, do nothing.
 * otherwise stops iterating
 *
 * @param hashmap The hashmap to iterate over
 * @param f The function pointer to call on each element
 * @param context The context to pass as the first argument to f
 * @return int 0 if the entire hashmap has been iterated over. 1 if not
 */
int hashmapApplyIterator(Hashmap* const hashmap, int (*f)(void* const, HashmapElement* const), void* const context) {
    for (unsigned int i = 0; i < hashmap->tableSize; i++) {
        HashmapElement* elem = &hashmap->data[i];
        if (elem->used) {
            int retFlag = f(context, elem);
            switch (retFlag) {
                case -1: {  // remove item
                    memset(elem, 0, sizeof(HashmapElement));
                    hashmap->size--;
                } break;
                case 0:  // continue iterating
                    break;
                default:  // early exit
                    return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Iterator to copy elements to a new hashmap while clearing the previous one
 *
 * @param newHashmap The new hashmap
 * @param element The current element
 * @return int 1 if it could not copy, -1 otherwise
 */
int hashmapRehashIterator(void* const newHashmap, HashmapElement* const element) {
    int flag = hashmapPut((Hashmap*)newHashmap, element->key, element->keyLen, element->data);
    if (flag) {
        return 1;
    }
    // clear old value to avoid stale pointers
    return -1;
}

/**
 * @brief Doubles the size of the hashmap
 *
 * @param hashmap the old hashmap
 * @return int 0 if success 1 otherwise
 */
int hashmapExpand(Hashmap* const hashmap) {
    // If this multiplication overflows hashmap_create will fail (not a power of 2)
    unsigned newSize = 2 * hashmap->tableSize;

    Hashmap newHash;
    int flag = hashmapCreate(newSize, &newHash);
    if (flag)
        return flag;

    // copy the old elements to the new hashmap
    flag = hashmapApplyIterator(hashmap, hashmapRehashIterator, (void*)&newHash);
    if (flag)
        return flag;

    hashmapDestroy(hashmap);

    // replace new hashmap
    memcpy(hashmap, &newHash, sizeof(Hashmap));

    return 0;
}

/**
 * @brief Iterator that logs strings and frees them
 *
 * @param context Not used, compatibility with hashmapApplyIterator
 * @param elem The elem to be logged and destroyed
 * @return int
 */
int logFreeIterator(void* const context, HashmapElement* const elem) {
    printf("%s = %s has been freed!\n", elem->key, (char* const)elem->data);
    free(elem->data);
    return -1;
}

/**
 * @brief Destroy the hashmap elements and the hashmap itself
 *
 * @param hashmap The hashmap to destroy
 * @param iterator Iterator function that destroy the element
 */
void hashmapDestroyWithOwnership(Hashmap* const hashmap, int (*iterator)(void* const, HashmapElement* const)) {
    if (hashmapApplyIterator(hashmap, iterator, NULL)) {
        printf("Failed to deallocate hashmap entries\n");
    }
    hashmapDestroy(hashmap);
}
