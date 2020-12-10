#include "specific.h"
#include "../../ADTs/General/general.h"
#include <assert.h>
#include <string.h>

#define INITIALSIZE 17
#define MULTIPLIER 37

bool _string_hash(assoc** a, void* key, long unsigned* hash);
bool _int_hash(assoc** a, void* key, long unsigned* hash);
bool _add_hash(assoc** a, void* key, void* data);

/*
   Initialise the Associative array
   keysize : number of bytes (or 0 => string)
   This is important when comparing keys since
   we'll need to use either memcmp() or strcmp()
*/
assoc* assoc_init(int keysize) {

    assoc *a =  ncalloc(1, sizeof(assoc));
    
    a->hash_table = (hash*) ncalloc(sizeof(hash), INITIALSIZE);
    a->capacity = INITIALSIZE;
    a->keysize = keysize;

    return a; 
}

/*
   Insert key/data pair
   - may cause resize, therefore 'a' might
   be changed due to a realloc() etc.


void assoc_insert(assoc** a, void* key, void* data) {

    

}

   Returns the number of key/data pairs 
   currently stored in the table

unsigned int assoc_count(assoc* a) {
   
    return 0;
}


   Returns a pointer to the data, given a key
   NULL => not found

void* assoc_lookup(assoc* a, void* key) {

   return NULL;

}

void assoc_todot(assoc* a) {

}
*/

/*Free up all allocated space from 'a'*/ 
void assoc_free(assoc* a) {

    free(a->hash_table);
    free(a);

}

/* Multiplication method taken from \
http://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)HashTables.html */

bool _string_hash(assoc** a, void* key, long unsigned* hash) {

    long unsigned h;
    assoc *p = *a;
    unsigned const char *us;
   
    if (key == NULL || hash == NULL) {
       return false;
    }

    /* cast s to unsigned const char * */
    /* this ensures that elements of s will be treated as having values >= 0 */
    us = (unsigned const char *) key;
   
    h = 0;
    while(*us != '\0') {

        h = h * MULTIPLIER + *us;
        us++;
    } 
   
    *hash = h % p->capacity;
    return true;
}

/* Hash function taken from
https://stackoverflow.com/questions/664014/what-integer-hash-function-are-\
good-that-accepts-an-integer-hash-key */

bool _int_hash(assoc** a, void* key, long unsigned* hash) {

    long unsigned h;
    assoc *p = *a;

    if (key == NULL || hash == NULL) {
       return false;
    }

    h = *((long unsigned*)key);

    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = (h >> 16) ^ h;
    
    *hash = h % p->capacity;
    return true;

}

bool _add_hash(assoc** a, void* key, void *data) {

    long unsigned hash;
    assoc *p = *a;
    

    if (p->keysize == 0) {
       _string_hash(a, key, &hash);
       strcpy(p->hash_table[hash].key, key);
       p->hash_table[hash].flag = true;
       p->hash_table[hash].data = data;
    }
    else {
       _int_hash(a, key, &hash);
       memcpy(p->hash_table[hash].key, key, sizeof(int));
       p->hash_table[hash].flag = true;
    }

    return true;

}


void _assoc_test(void) {

    int key, data; 
    unsigned long num, hash;
    void *p;
    char *str = (char *)ncalloc(sizeof(char), 100);
    assoc *a, *b;
    
    /* Test assoc_init function*/
    a = assoc_init(sizeof(int));
    
    assert(a->size == 0);
    assert(a->keysize == sizeof(int));
    assert(a->capacity = INITIALSIZE);
    assert(a->hash_table[0].key == 0);
    assert(a->hash_table[0].data == 0);

    key = 5;
    data = 7;
    a->hash_table[0].key = &key;
    a->hash_table[0].data = &data;
    a->hash_table[5].key = &key;
    a->hash_table[5].data = &data;
    a->hash_table[0].flag = true;

    /*Dereference typecasted void pointer*/
    assert(*((int*)a->hash_table[0].key) == 5);
    assert(*((int*)a->hash_table[0].data) == 7);
    assert(*((int*)a->hash_table[5].key) == 5);
    assert(*((int*)a->hash_table[5].data) == 7);
    assert(a->hash_table[0].flag == true);

    b = assoc_init(0);

    assert(b->size == 0);
    assert(b->keysize == 0);
    assert(b->capacity = INITIALSIZE);
    assert(b->hash_table[0].key == 0);
    assert(b->hash_table[0].data == 0);

    /*Pointer to pointer typecasting *(char **) is used \
    to retrieve the string stored in a character pointer using void pointer*/ 
    strcpy(str, "Hello, World!");
    b->hash_table[4].key = &str;
    assert(strcmp(str,*(char **)b->hash_table[4].key) == 0);
    strcpy(str, "Hallelujah I love C");
    b->hash_table[6].key = &str;
    assert(strcmp(str,*(char **)b->hash_table[6].key) == 0);
    

    /* Test _string_hash function*/
    assert(_string_hash(&a, str, &hash) == true);
    assert(hash < INITIALSIZE);
    strcpy(str, "Test 1");
    assert(_string_hash(&a, str, &hash) == true);
    assert(hash < INITIALSIZE);
    strcpy(str, "A second test");
    assert(_string_hash(&a, str, &hash) == true);
    assert(hash < INITIALSIZE);
    strcpy(str, "Third test");
    assert(_string_hash(&a, str, &hash) == true);
    assert(hash < INITIALSIZE);
    printf("%lu\n", hash);
    /* Test for NULL*/
    assert(_string_hash(&a, NULL, &hash) == false);
    assert(_string_hash(&a, str, NULL) == false);
    assert(_string_hash(&a, NULL, NULL) == false);

    /*Test_int_hash function*/
    num = 334137732;
    p = &num;
    assert(_int_hash(&a, p, &hash) == true);
    assert(hash < INITIALSIZE);
    num = 412;
    p = &num;
    assert(_int_hash(&a, p, &hash) == true);
    assert(hash < INITIALSIZE);
    num = 92057229146;
    p = &num;
    assert(_int_hash(&a, p, &hash) == true);
    assert(hash < INITIALSIZE);
    num = 63729479;
    p = &num;
    assert(_int_hash(&a, p, &hash) == true);
    assert(hash < INITIALSIZE);
    /* Test for NULL*/
    assert(_int_hash(&a, p, NULL) == false);
    assert(_int_hash(&a, NULL, NULL) == false);
    assert(_int_hash(&a, NULL, &hash) == false);

    /* Test assoc_free function*/
    assoc_free(a);
    assoc_free(b);


    free(str);



}
