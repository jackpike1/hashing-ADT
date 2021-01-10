/*Cuckoo Hash works perfectly for ints in testassoc
and it works for strings, but fails to calloc after
120,000 new strings as hash table becomes too big.
I can't quite find a good enough hash function in 
spite of research*/

#include "specific.h"
#include "../../ADTs/General/general.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define INITIALSIZE 17
#define SCALEFACTOR 4
#define HASH1 0
#define HASH2 19
#define TWOTHIRDS /1.5
#define EMPTYHASH empty_hash.flag = false; \
empty_hash.data = NULL; empty_hash.key = NULL;

void _hash(assoc* a, void* key, unsigned int* hash);
void _hash_two(assoc* a, void* key, unsigned int* hash);
bool _add_hash(assoc* a, void* key, void* data);
bool _add_hash_two(assoc* a, void* key, void* data);
void _add_data(hash* a, void* key, void* data, \
unsigned int hash);
assoc* _realloc(assoc* a);
unsigned int _primetable(assoc* a);
bool _isprime(unsigned int c); 
bool _rehash(assoc* a, assoc* b);
bool _isduplicate(assoc* a, void* key);
hash _search_one(assoc* a, void* key);
hash _search_two(assoc* a, void* key);
int log2n(unsigned int n);

/*
   Initialise the Associative array
   keysize : number of bytes (or 0 => string)
   This is important when comparing keys since
   we'll need to use either memcmp() or strcmp()
*/

assoc* assoc_init(int keysize) {

    assoc *a =  ncalloc(1, sizeof(assoc));
    
    a->hash_table = (hash*) ncalloc(sizeof(hash), \
    INITIALSIZE);
    a->hash_table2 = (hash*) ncalloc(sizeof(hash), \
    INITIALSIZE);
    a->capacity = INITIALSIZE;
    a->keysize = keysize;

    return a;
}

/*
   Insert key/data pair
   - may cause resize, therefore 'a' might
   be changed due to a realloc() etc.
*/

void assoc_insert(assoc** a, void* key, void* data) {

    assoc *p, *b;
    p = *a;

    /*Check for void pointers */
    if (p == NULL || key == NULL) {
        on_error("Error: Null pointer\n");
    }

    /*Check for duplicates*/
    if (_isduplicate(p, key)) {
    }
    else {
        /* If log2(hashtablesize) bounces, 
        realloc table*/
        if (!_add_hash(p, key, data)) {
            b = _realloc(p);
            assert(_rehash(p, b));
            *a = b;
            _add_hash(b, key, data);
            
            free(p->hash_table);
            free(p->hash_table2);
            free(p);
        }
    }
}

/*
   Returns the number of key/data pairs 
   currently stored in the table
*/
unsigned int assoc_count(assoc* a) {

    return a->size;
}

/*
   Returns a pointer to the data, given a key
   NULL => not found
*/
void* assoc_lookup(assoc* a, void* key) {

    hash hash1 = _search_one(a, key);
    hash hash2 = _search_two(a, key);

    if (hash1.key != NULL) {
        return hash1.data;
    }
    if (hash2.key != NULL) {
        return hash2.data;
    }

    return NULL;
}

void assoc_todot(assoc* a);

/* Free up all allocated space from 'a' */
void assoc_free(assoc* a) {
    
    free(a->hash_table);
    free(a->hash_table2);
    free(a);
}

void _assoc_test(void);

/* Two variations of djb2 hash function edited from: \
https://gist.github.com/MohamedTaha98/ccdf734f13299efb73ff0b12f7ce429f */

void _hash(assoc* a, void* key, unsigned int* hash) {

    unsigned long h = 5381 + a->capacity;
    char *str;
    unsigned int c, count = 0; 
    str = (char*)key;

    if (a->keysize) {
        while ((count < a->keysize)) {
            h = ((h << HASH1) + h) + *(int*)key;
            count++;
        }
    }

    /*http://www.cse.yorku.ca/~oz/hash.html*/
    else {
        while ((c = *str++)){
            h = c + (h << 6) + (h << 16) - h;
        }
    }
    *hash = h % a->capacity;
}

void _hash_two(assoc* a, void* key, unsigned int* hash) {

    unsigned long h = 7;
    unsigned int count = 0; 
    char *str;
    str = (char*)key;
   
    if (a->keysize) {
        while ((count < a->keysize)) {
            h = ((h << HASH2) + *str + count);
            count++;
        }
    }
    else {
        while ((*str++)) {
            h = ((h << HASH2) + h + *str);
    }
    }
    *hash = h % a->capacity;    
}

/* Find hash and insert data into hashtable
*/
bool _add_hash(assoc* a, void* key, void* data) {

    unsigned int hash = 0;
    void *old_key = 0, *old_data = 0;
    static int count = 0;

    _hash(a, key, &hash);
    old_key = a->hash_table[hash].key;
    old_data = a->hash_table[hash].data;

    /*If cell is full bounce data into second hash table*/
    if (a->hash_table[hash].flag) {
        count += 1;
        /*Resize hash table after log base2 bounces*/
        if (count == log2n(a->capacity)) {
            printf("count is %d capacity is %d assoc_count(a) is %d\n", count, a->capacity, assoc_count(a));
            count = 0;
            return false;
        }
        _add_data(a->hash_table, key, data, hash);
        _add_hash_two(a, old_key, old_data);
    }
    else {
        a->size += 1;
        _add_data(a->hash_table, key, data, hash);
    }
    return true;
}

/* Find hash and insert into hashtable 
*/
bool _add_hash_two(assoc* a, void* key, void* data) {

    unsigned int hash = 0;
    void *old_key = 0, *old_data = 0;
    
    _hash_two(a, key, &hash);
    old_key = a->hash_table2[hash].key;
    old_data = a->hash_table2[hash].data;

    /*If cell is full bounce data into first hash table */
    if (a->hash_table2[hash].flag) {
        _add_data(a->hash_table2, key, data, hash);
        _add_hash(a, old_key, old_data);
    }
    else {
        a->size += 1;
        _add_data(a->hash_table2, key, data, hash);
    }
    return true;
}

/* Add data to correct cell in hash table 
*/
void _add_data(hash* a, void* key, void* data, \
unsigned int hash) {

    a[hash].data = data;
    a[hash].flag = true;
    a[hash].key = key;
}

/* Allocate space for new hash table 
*/
assoc* _realloc(assoc* a) {

    assoc* b = ncalloc(1, sizeof(assoc));

    b->hash_table = (hash*) ncalloc(sizeof(hash), \
    _primetable(a));
    b->hash_table2 = (hash*) ncalloc(sizeof(hash), \
    _primetable(a));
    b->capacity = _primetable(a);
    b->keysize = a->keysize;
    
    return b;
}

/*Find the next prime number using scalefactor to\
 size the new hash table
 */
unsigned int _primetable(assoc* a) {

    unsigned int prime;

    prime = a->capacity*SCALEFACTOR;

    while (!_isprime(prime)) {
        prime += 1;
    }
    return prime;
}

/*Check whether a number is prime
*/
bool _isprime(unsigned int c) {
   
   unsigned int i; 
   bool flag = true;
   
   for (i = 2; i <= c/2; i++) {
      
      if (c % i == 0) {
         flag = false;
      }
   }   
   return flag; 
}   

/* Take data from one table and hash into second table 
*/
bool _rehash(assoc* a, assoc* b) {

    unsigned int i = 0, size = a->capacity;

    if (a == NULL || b == NULL) {
        return false;
    }

    for (i = 0; i < size; i++) {
        if (a->hash_table[i].flag) {
            _add_hash(b, a->hash_table[i].key, \
            a->hash_table[i].data);
        }
        if (a->hash_table2[i].flag) {
            _add_hash(b, a->hash_table2[i].key, \
            a->hash_table2[i].data);
        }
    }
    return true;
}

/* Check if key already exists in hashtable 
*/
bool _isduplicate(assoc* a, void* key) {

    hash hash1 = _search_one(a, key);
    hash hash2 = _search_two(a, key);

    if (hash1.key != NULL || hash2.key != NULL) {
        return true;
    }
    return false;
}

/* Search for key within first hashtable 
 using strcmp and memcmp
 */
hash _search_one(assoc* a, void* key) {

    hash empty_hash;
    unsigned int hashone;
    
    _hash(a, key, &hashone);

    if (!a->keysize) {
        if (a->hash_table[hashone].flag) {
            if (!strcmp((char*)a->hash_table[hashone].key,\
             (char*)key))  {
                return a->hash_table[hashone];
            }
        }
    }
    else {
        if (a->hash_table[hashone].flag) {
            if (!memcmp(a->hash_table[hashone].key, key,\
             a->keysize))  {
                return a->hash_table[hashone];
            }
        }
    }   
    EMPTYHASH
    return empty_hash;
}

/* Search for key within second hashtable 
using strcmp and memcmp
*/
hash _search_two (assoc* a, void* key) {

    hash empty_hash;
    unsigned int hashtwo;
    
    _hash_two(a, key, &hashtwo);
    
    if (!a->keysize) {
        if (a->hash_table2[hashtwo].flag) {
            if (!strcmp((char*)a->hash_table2[hashtwo].key,\
             (char*)key)) {
                return a->hash_table2[hashtwo];
            }
        }
    }
    else {
        if (a->hash_table2[hashtwo].flag) {
            if (!memcmp(a->hash_table2[hashtwo].key,\
             key, a->keysize)) {
                return a->hash_table2[hashtwo];
            }
        }
    }
    EMPTYHASH
    return empty_hash;
}

/* Calculates log base 2 of a number 
required when resizing array
*/
int log2n(unsigned int n) {

    return (n > 1) ? 1 + log2n(n / 2) : 0;
}
