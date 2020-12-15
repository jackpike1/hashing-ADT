#include "specific.h"
#include "../../ADTs/General/general.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#define INITIALSIZE 17
#define MULTIPLIER 37
#define PRIME 13
#define SCALEFACTOR 4
#define HASH 16

bool _string_hash(assoc* a, void* key, unsigned int* hash);
bool _int_hash(assoc* a, void* key, unsigned int* hash);
bool _add_hash(assoc* a, void* key, void* data);
bool _probe(assoc* p, unsigned int*hash);
bool _add_data(assoc* a, void* key, void* data, unsigned int hash);
assoc* _realloc(assoc* a);
unsigned int _primetable(assoc* a);
bool _isprime(unsigned int c); 
bool _rehash(assoc* a, assoc* b);
bool _duplicate(assoc* a, void* key);

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
*/

void assoc_insert(assoc** a, void* key, void* data) {

    assoc *p, *b;
    p = *a;
    
    /* If 2/3 full capacity: realloc, rehash, add hash\
     re-direct pointer and free old structure*/
    if (p->size == (unsigned int)(p->capacity/1.5)) {
        b = _realloc(p);
        assert(_rehash(p, b));
        *a = b;
        _add_hash(b, key, data);
        free(p->hash_table);
        free(p);
    }
    else {
        _add_hash(p, key, data);
    }
}

/*   Returns the number of key/data pairs 
   currently stored in the table
   */

unsigned int assoc_count(assoc* a) {
   
    unsigned int i, capacity = a->capacity;
    unsigned int count = 0;

    for (i = 0; i < capacity; i++){
        if (a->hash_table[i].flag) {
            count += 1;
        }
    }
    return count;
}

/*   Returns a pointer to the data, given a key
   NULL => not found
*/

void* assoc_lookup(assoc* a, void* key) {

   unsigned int i, capacity = a->capacity;

   for (i = 0; i < capacity; i++) {
       if (a->hash_table[i].key == key) {
           return a->hash_table[i].data;
       }
   }
   return NULL;
}

/*
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

bool _string_hash(assoc* a, void* key, unsigned int* hash) {

    unsigned int h;
    unsigned const char *us;
   
    if (key == NULL || hash == NULL) {
       return false;
    }

    us = (unsigned const char *) key;
   
    h = 0;
    while(*us != '\0') {

        h = h * MULTIPLIER + *us;
        us++;
    } 
   
    *hash = h % a->capacity;
    return true;
}

/* 
https://stackoverflow.com/questions/664014/what-integer-hash-function-are-\
good-that-accepts-an-integer-hash-key */

bool _int_hash(assoc* a, void* key, unsigned int* hash) {

    unsigned int h;

    if (key == NULL || hash == NULL) {
       return false;
    }

    h = *((unsigned int*)key);

    h = ((h >> HASH) ^ h) * 0x45d9f3b;
    h = ((h >> HASH) ^ h) * 0x45d9f3b;
    h = (h >> HASH) ^ h;
    
    *hash = h % a->capacity;

    return true;

}

bool _add_hash(assoc* a, void* key, void *data) {

    unsigned int hash = 0;

    if (a == NULL || key == NULL) {
        return false;
    }
    
    if (a->keysize) {
        /*Get hash code*/
        if (!_int_hash(a, key, &hash)) {
            on_error("Eror from void pointer\n");
    }    
    }
    else {
        /*Get hash code*/
        if (!_string_hash(a, key, &hash)) {
            on_error("Error from void pointer\n");
        }
    }
    /*If collision, get new hash code*/
    if (a->hash_table[hash].flag == true) {
        if (!_probe(a, &hash)){
            on_error("Error finding a hash code\n");
        }
    }
    /* Add data*/
    if (!_add_data(a, key, data, hash)){
        on_error("Error inputting data to hash table\n");
    }
    return true;
}

bool _probe(assoc* p, unsigned int*hash) {
                                   
    int i, step, size, new_hash;

    if (p == NULL || hash == NULL) {
        return false;
    }

    /*https://www.geeksforgeeks.org/double-hashing/ */
    step = PRIME - (*(int*)hash % PRIME);
    
    size = p->capacity, new_hash = *(hash) + step;

    for (i = 0; i < size; i++) {
       /*Wrap around hash_table*/
       if (new_hash >= size) {
          new_hash = new_hash -size;
       }
       /*Check for empty cell */
       if (p->hash_table[new_hash].flag == false) {
          *hash = new_hash;
          return true;
       }
        new_hash = new_hash + step;
    }
    return false;
}

bool _add_data(assoc *a, void* key, void* data, unsigned int hash) {

    if (a == NULL || key == NULL) {
        return false;
    }

    a->hash_table[hash].data = data;
    a->hash_table[hash].flag = true;
    a->hash_table[hash].key = key;
    a->size += 1;

    return true;
}

assoc* _realloc(assoc* a) {

    assoc* b = ncalloc(1, sizeof(assoc));

    b->hash_table = (hash*) ncalloc(sizeof(hash), _primetable(a));
    b->capacity = _primetable(a);
    b->keysize = a->keysize;
    b->size = a->size;
    
    return b;
}

unsigned int _primetable(assoc* a) {

    unsigned int prime;

    prime = a->capacity*SCALEFACTOR;

    while (!_isprime(prime)) {
        prime += 1;
    }
    return prime;
}

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

bool _rehash(assoc* a, assoc* b) {

    unsigned int i = 0, size = a->capacity;

    if (a == NULL || b == NULL) {
        return false;
    }

    for (i = 0; i < size; i++) {
        if (a->hash_table[i].flag) {
            _add_hash(b, a->hash_table[i].key, a->hash_table[i].data);
        }
    }
    return true;
}

bool _duplicate(assoc* a, void* key) {

    unsigned int hash = 0, step, size;

    size = a->capacity, step = PRIME - (hash % PRIME);

    if (a->keysize) {
        /*Get hash code*/
        if (!_int_hash(a, key, &hash)) {
            on_error("Eror from void pointer\n");
    }    
    }
    else {
        /*Get hash code*/
        if (!_string_hash(a, key, &hash)) {
            on_error("Error from void pointer\n");
        }
    }
    
    /*Check single and double hashes*/
    while (a->hash_table[hash].flag) {
        if (a->hash_table[hash].key == key) {
            return true;
        }
        
        hash = hash + step;
        if (hash >= size) {
          hash = hash -size;
       }
    }
    return false;  
}



void _assoc_test(void) {

    int key, data, cc, ee, ff, gg, hh, ii; 
    unsigned int num, hash;
    void *p, *d, *c, *e, *f, *g, *h, *i;
    char *str = (char *)ncalloc(sizeof(char), 100);
    char str2[1000], str3[1000], str4[1000], str5[1000], str6[1000], str7[1000];
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
    assert(_string_hash(a, str, &hash) == true);
    assert(hash < INITIALSIZE);
    strcpy(str, "Test 1");
    assert(_string_hash(a, str, &hash) == true);
    assert(hash < INITIALSIZE);
    strcpy(str, "A second test");
    assert(_string_hash(a, str, &hash) == true);
    assert(hash < INITIALSIZE);
    strcpy(str, "Third test");
    assert(_string_hash(a, str, &hash) == true);
    assert(hash < INITIALSIZE);
    /* Test for NULL*/
    assert(_string_hash(a, NULL, &hash) == false);
    assert(_string_hash(a, str, NULL) == false);
    assert(_string_hash(a, NULL, NULL) == false);

    /*Test_int_hash function*/
    num = 334137732;
    p = &num;
    assert(_int_hash(a, p, &hash) == true);
    assert(hash < INITIALSIZE);
    num = 412;
    p = &num;
    assert(_int_hash(a, p, &hash) == true);
    assert(hash < INITIALSIZE);
    num = 920572291;
    p = &num;
    assert(_int_hash(a, p, &hash) == true);
    assert(hash < INITIALSIZE);
    num = 63729479;
    p = &num;
    assert(_int_hash(a, p, &hash) == true);
    assert(hash < INITIALSIZE);
    /* Test for NULL*/
    assert(_int_hash(a, p, NULL) == false);
    assert(_int_hash(a, NULL, NULL) == false);
    assert(_int_hash(a, NULL, &hash) == false);

    /* Test assoc_free function*/
    assoc_free(a);
    assoc_free(b);

    /*Test _probe function*/
    a = assoc_init(0);
    a->hash_table[2].flag = true;
    hash = 2;
    p = &hash;
    _probe(a, p);
    assert(*(int*)p == 13);
    
    a->hash_table[13].flag = true;
    hash = 2;
    p = &hash;
    _probe(a, p);
    assert(*(int*)p == 7);
    
    a->hash_table[5].flag = true;
    hash = 5;
    p = &hash;
    _probe(a, p);
    assert(*(int*)p == 4);

    a->hash_table[16].flag = true;
    hash = 16;
    p = &hash;
    _probe(a, p);
    assert(*(int*)p == 9);

    /*Test for NULL*/
    assert(_probe(NULL, p) == false);
    assert(_probe(a, NULL) == false);

    assoc_free(a);

    /* Test _add_data function*/
    a = assoc_init(sizeof(int));
    key = 7261537;
    p = &key;
    hash = 13;
    assert(_add_data(a, p, NULL, hash));
    assert(*(int*)(a->hash_table[hash].key) == key);
    assert(a->hash_table[hash].flag == true);

    data = 753892;
    d = &data;
    hh = 12;
    hash = 11;
    assert(_add_data(a, d, &hh, hash));
    assert(*(int*)(a->hash_table[hash].key) == data);
    assert(*(int*)(a->hash_table[hash].data) == 12);
    assert(a->hash_table[hash].flag == true);
    
    b = assoc_init(0);

    strcpy(str, "Hello, World!");
    hash = 16;
    assert(_add_data(b, str, NULL, hash));
    assert(strcmp(str, (char *)b->hash_table[hash].key) == 0);


    /*Test for NULL*/
    assert(!_add_data(NULL, d, NULL, hash));
    assert(!_add_data(b, NULL, NULL, hash));

    assoc_free(a);
    assoc_free(b);

    /* Test _add_hash function*/
    a = assoc_init(sizeof(int));
    num = 2333289326;
    p = &num;
    assert(_add_hash(a, p, NULL));
    assert(a->hash_table[1].flag == true);
    assert(*(unsigned int*)(a->hash_table[1].key) == num);

    key = 470192831;
    d = &key;
    assert(_add_hash(a, d, NULL));
    assert(a->hash_table[9].flag == true);
    assert(*(int*)(a->hash_table[9].key) == key);

    b = assoc_init(0);
    strcpy(str2, "I hate C");
    p = &str2;
    assert(_add_hash(b, p, NULL));
    assert(b->hash_table[7].flag == true);
    assert(strcmp(str2, (char*)b->hash_table[7].key)== 0);

    strcpy(str2, "I actually love C");
    p = &str2;
    assert(_add_hash(b, p, NULL));
    assert(b->hash_table[4].flag == true);
    assert(strcmp(str2, (char*)b->hash_table[4].key)== 0);

    assoc_free(a);
    assoc_free(b);    

    /*Test is_prime function*/
    assert(_isprime(5));
    assert(_isprime(1721));
    assert(_isprime(677));
    assert(_isprime(479));
    assert(!_isprime(80));
    assert(!_isprime(100));
    assert(!_isprime(81));

    /*Test _primetable */
    a = assoc_init(0);
    assert(_primetable(a) == 71);
    a->capacity = 71;
    assert(_primetable(a) == 293);
    a->capacity = 293;
    assert(_primetable(a) == 1181);
    assert(_primetable(a) != 167);


    assoc_free(a);

    /*Test _realloc */
    a = assoc_init(0);
    a->size = 16;
    b = _realloc(a);
    assert(b->capacity == 71);
    assert(b->size == 16);
    assoc_free(a);
    a = _realloc(b);
    assert(a->capacity == 293);

    assoc_free(a);
    assoc_free(b);

    /*Test _rehash*/
    a = assoc_init(sizeof(int));
    cc = 87387386;
    ee = 27028464;
    ff = 526283;
    gg = 937464;
    hh = 7635322;
    c = &cc;
    e = &ee;
    f = &ff;
    g = &gg;
    h = &hh;
    _add_hash(a, c, NULL);
    _add_hash(a, e, NULL);
    _add_hash(a, f, NULL);
    _add_hash(a, g, NULL);
    _add_hash(a, h, NULL);
    b = _realloc(a);
    assert(_rehash(a, b));

    /*Show hashed into different position*/
    assert(*(int*)a->hash_table[1].key == cc);
    assert(*(int*)b->hash_table[34].key == cc);
    assert(*(int*)a->hash_table[9].key == ee);
    assert(*(int*)b->hash_table[23].key == ee);
    assert(*(int*)a->hash_table[8].key == ff);
    assert(*(int*)b->hash_table[40].key == ff);
    assert(*(int*)a->hash_table[10].key == gg);
    assert(*(int*)b->hash_table[37].key == gg);
    assert(*(int*)a->hash_table[14].key == hh);
    assert(*(int*)b->hash_table[59].key == hh);

    ii = 52;
    i = &ii;
    _add_hash(b, i, NULL);

    assoc_free(a);
    assoc_free(b);

    /*Test assoc_insert and assoc_count*/
    a = assoc_init(sizeof(int));
    _add_hash(a, e, NULL);
    _add_hash(a, f, NULL);
    _add_hash(a, g, NULL);
    _add_hash(a, h, NULL);
    a->size = 11;
    assoc_insert(&a, c, NULL);

    assoc_free(a);

    b = assoc_init(0);
    strcpy(str2, "Hello");
    strcpy(str3,"goodbye");
    strcpy(str4, "ink");
    strcpy(str5, "minx");
    strcpy(str6, "trifle");
    strcpy(str7, "dog");
    c = &str2;
    d = &str3;
    e = &str4;
    f = &str5;
    g = &str6;
    h = &str7;
    cc = 4;
    ee = 78;
    ff = 523;
    gg = 69;
    hh = 123;
    assoc_insert(&b, c, &cc);
    assoc_insert(&b, d, NULL);
    assoc_insert(&b, e, &ee);
    assoc_insert(&b, f, &ff);
    assoc_insert(&b, g, &gg);
    assert(assoc_count(b) == 5);
    b->size = 11;
    assoc_insert(&b, h, NULL);
    assert(assoc_count(b) == 6);
    assoc_free(b);

    a = assoc_init(sizeof(int));
    c = &cc;
    e = &ee;
    f = &ff;
    g = &gg;
    h = &hh;
    assoc_insert(&a, c, NULL);
    assoc_insert(&a, e, NULL);
    assoc_insert(&a, f, NULL);
    assoc_insert(&a, g, NULL);
    assoc_insert(&a, h, NULL);
    assert(assoc_count(a) == 5);

    assoc_free(a);

    /*Test assoc_lookup*/
    b = assoc_init(0);
    strcpy(str2, "Hello");
    strcpy(str3,"goodbye");
    strcpy(str4, "ink");
    strcpy(str5, "minx");
    strcpy(str6, "trifle");
    strcpy(str7, "dog");
    c = &str2;
    d = &str3;
    e = &str4;
    f = &str5;
    g = &str6;
    h = &str7;
    cc = 4;
    ee = 78;
    ff = 523;
    gg = 69;
    hh = 123;
    assoc_insert(&b, c, &cc);
    assoc_insert(&b, e, &ee);
    assoc_insert(&b, f, &ff);
    assoc_insert(&b, g, &gg);
    assoc_insert(&b, h, &hh);

    assert(*(int*)assoc_lookup(b, f) == 523);
    assert(*(int*)assoc_lookup(b, c) == 4);
    assert(*(int*)assoc_lookup(b, e) == 78);
    assert(*(int*)assoc_lookup(b, g) == 69);
    assert(*(int*)assoc_lookup(b, h) == 123);

    assoc_free(b);




    free(str);
}


