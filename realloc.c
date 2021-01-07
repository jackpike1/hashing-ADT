#include "specific.h"
#include "../../ADTs/General/general.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#define INITIALSIZE 17
#define PRIME 13
#define SCALEFACTOR 4
#define HASH1 5
#define HASH2 3
#define TWOTHIRDS /1.5

void _hash(assoc* a, unsigned int* hash);
void _hash_two(assoc* a, unsigned int* hash);
bool _add_hash(assoc* a);
bool _probe(assoc* a, unsigned int* hash);
void _add_data(assoc* a, unsigned int hash);
assoc* _realloc(assoc* a);
unsigned int _primetable(assoc* a);
bool _isprime(unsigned int c); 
bool _rehash(assoc* a, assoc* b);
bool _isduplicate(assoc* a);
hash _search(assoc* a);

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

    p->cdata = data;
    p->ckey = key;
    
    /*Check for duplicates*/
    if (_isduplicate(p)) {
    }
    else {
        /* If 2/3 capacity: realloc, rehash, add hash,
         re-direct pointer and free old structure*/
        if (p->size == (unsigned int)\
        (p->capacity TWOTHIRDS)) {
            b = _realloc(p);
            assert(_rehash(p, b));
            *a = b;
            if (!_add_hash(b)) {
                on_error("Error: Null pointer\n");
            }
            free(p->hash_table);
            free(p);
        }
        else {
            if (!_add_hash(p)) {
                on_error("Error: Null pointer\n");
            }
        }
    }
}

/*   Returns the number of key/data pairs 
   currently stored in the table
*/

unsigned int assoc_count(assoc* a) {
   
    return a->size;
}

/*   Returns a pointer to the data, given a key
   NULL => not found
*/

void* assoc_lookup(assoc* a, void* key) {
    
    hash hash1;

    a->ckey = key;
    hash1 = _search(a);
    
    return hash1.data;
}

/*
void assoc_todot(assoc* a) {

}
*/

/*Free up all allocated space from 'a'
*/ 
void assoc_free(assoc* a) {

    free(a->hash_table);
    free(a);
}

/* Two variations of djb2 hash function edited from: \
https://gist.github.com/MohamedTaha98/ccdf734f13299efb73ff0b12f7ce429f 
*/

void _hash(assoc* a, unsigned int* hash) {

    unsigned long h = 5381;
    char *str;
    unsigned int count = 0; 
    str = (char*)a->ckey;

    if (a->keysize) {
        while ((count < a->keysize)) {
            h = ((h << HASH1) + h) + *str;
            count++;
        }
    }
    else {
        while ((*str++)) {
            h = ((h << HASH1) + h) + *str;
        }
    }
    *hash = h % a->capacity;
}

void _hash_two(assoc* a, unsigned int* hash) {

    unsigned long h = 599;
    unsigned int count = 0; 
    char *str;
    str = (char*)a->ckey;
   
    if (a->keysize) {
        while ((count < a->keysize)) {
            h = ((h << HASH2) + h) + *str;
            count++;
        }
    }
    else {
        while ((*str++)) {
            h = ((h << HASH2) + h) + *str;
       }
    }
    *hash = h % a->capacity;
}


bool _add_hash(assoc* a) {

    unsigned int hash = 0;

    if (a == NULL || a->ckey == NULL) {
        return false;
    }

    _hash(a, &hash);
    
    /*If collision, get new hash code*/
    if (a->hash_table[hash].flag) {
        if (!_probe(a, &hash)){
            on_error("Error finding a hash code\n");
        }
    }

    _add_data(a, hash);
    
    return true;
}

bool _probe(assoc* a, unsigned int* hash) {
                                   
    int i, step, size, new_hash;
    unsigned int hashtwo = 0;

    _hash_two(a, &hashtwo);

    /*https://www.geeksforgeeks.org/double-hashing/ 
    Use hashtwo to create step to probe*/
    step = PRIME - (hashtwo % PRIME);
    size = a->capacity, new_hash = *(hash) + step;

    for (i = 0; i < size; i++) {
       /*Wrap around hash_table*/
       if (new_hash >= size) {
          new_hash = new_hash -size;
       }
       /*Check for empty cell */
       if (!a->hash_table[new_hash].flag) { 
          *hash = new_hash;
          return true;
       }
        new_hash = new_hash + step;
    }
    return false;
}

void _add_data(assoc *a, unsigned int hash) {

    a->hash_table[hash].data = a->cdata;
    a->hash_table[hash].flag = true;
    a->hash_table[hash].key = a->ckey;
    a->size += 1;
}

assoc* _realloc(assoc* a) {

    assoc* b = ncalloc(1, sizeof(assoc));

    b->hash_table = (hash*) ncalloc(sizeof(hash), \
    _primetable(a));
    b->capacity = _primetable(a);
    b->keysize = a->keysize;
    
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
            b->ckey = a->hash_table[i].key;
            b->cdata = a->hash_table[i].data;
            _add_hash(b);
        }
    }
    return true;
}

bool _isduplicate(assoc* a) {

    hash hash1 = _search(a);

    if (hash1.flag) {
        return true;
    }
    return false;
}

hash _search(assoc* a) {
    
    hash empty_hash;
    unsigned int hashone = 0, hashtwo = 0, step, \
    size = a->capacity;
    _hash(a, &hashone); 
    
    /*Check single and double hashes for duplicates*/
    while (a->hash_table[hashone].flag) {
        if (!a->keysize) {
            if (!strcmp((char*)a->hash_table[hashone].key, \
            (char*)a->ckey)) {
                return a->hash_table[hashone];
            }
        }
        else {
            if (!memcmp(a->hash_table[hashone].key, \
            a->ckey, a->keysize)) {
                return a->hash_table[hashone];
            }
        }
        _hash_two(a, &hashtwo);
        step = PRIME - (hashtwo % PRIME); 
        hashone += step;
        /*Wrap around hash table*/
        if (hashone >= size) {
            hashone = hashone -size;
        }
    }
   empty_hash.flag = false; 
   empty_hash.data = NULL;
   empty_hash.key = NULL;
   return empty_hash;
}

void _assoc_test(void) {

    hash hash1;
    int key, data, cc, ee, ff, gg, hh, ii; 
    unsigned int num, hash;
    unsigned long nn;
    double jj, kk;
    float ll, mm; 
    void *p, *d, *c, *e, *f, *g, *h, *i;
    char *str = (char *)ncalloc(sizeof(char), 100);
    char str2[1000], str3[1000], str4[1000], str5[1000], \
    str6[1000], str7[1000];
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

    /*Test _hash function*/
    b->ckey = str;
    _hash(b, &hash);
    assert(hash < INITIALSIZE);
    strcpy(str, "Test 1");
    b->ckey = str;
    _hash(b, &hash);
    assert(hash < INITIALSIZE);
    strcpy(str, "A second test");
    b->ckey = str;
    _hash(b, &hash);
    assert(hash < INITIALSIZE);
    strcpy(str, "Third test");
     b->ckey = str;   
    _hash(b, &hash);
    assert(hash < INITIALSIZE);

    assoc_free(b);

    /*Test ints*/
    cc = 84794;
    ee = 467363;
    ff = 33839;
    gg = 393933;
    hash = 0;
     a->ckey = &cc;    
    _hash(a, &hash);
    assert(hash < INITIALSIZE);
     a->ckey = &ee;   
    _hash(a, &hash);
    assert(hash < INITIALSIZE);
     a->ckey = &ff;   
    _hash(a, &hash);
    assert(hash < INITIALSIZE);
     a->ckey = &gg;   
    _hash(a, &hash);
    assert(hash < INITIALSIZE);

    /*Test for doubles, floats, longs*/
    b = assoc_init(sizeof(double));
    jj = 4829.6176;
    kk = 16262.0843;
    hash = 0; 
     b->ckey = &jj;   
    _hash(b, &hash);
    assert(hash < INITIALSIZE);
     b->ckey = &kk;  
    _hash(b, &hash);
    assert(hash < INITIALSIZE);
    assoc_free(b);

    b = assoc_init(sizeof(float));
    ll = 15162.1626;
    mm = 37386.122;
     b->ckey = &ll;  
    _hash(b, &hash);
    assert(hash < INITIALSIZE);
     b->ckey = &mm;  
    _hash(b, &hash);
    assert(hash < INITIALSIZE);
    assoc_free(b);

    b = assoc_init(sizeof(unsigned long));
    nn = 8127282839916836;
     b->ckey = &nn;  
    _hash(b, &hash);
    assert(hash < INITIALSIZE);
    assoc_free(b);

    /* Test assoc_free function*/
    assoc_free(a);


    /* Test _hash_two function*/
    b = assoc_init(0);
    strcpy(str, "Test 1");
     b->ckey = str;  
    _hash_two(b, &hash);
    assert(hash < INITIALSIZE);
    strcpy(str, "A second test");
     b->ckey = str;  
    _hash_two(b, &hash);
    assert(hash < INITIALSIZE);
    strcpy(str, "Third test");
     b->ckey = str;  
    _hash_two(b, &hash);
    assert(hash < INITIALSIZE);

    assoc_free(b);

    /*Test ints*/
    a = assoc_init(sizeof(int));
    cc = 84794;
    ee = 467363;
    ff = 33839;
    gg = 393933;
    hash = 0; 
     a->ckey = &cc;  
    _hash_two(a, &hash);
    assert(hash < INITIALSIZE);
     a->ckey = &ee;  
    _hash_two(a, &hash);
    assert(hash < INITIALSIZE);
     a->ckey = &ff;  
    _hash_two(a, &hash);
    assert(hash < INITIALSIZE);
     a->ckey = &gg;  
    _hash_two(a, &hash);
    assert(hash < INITIALSIZE);

    /*Test for doubles, floats, longs*/
    b = assoc_init(sizeof(double));
    jj = 4829.6176;
    kk = 16262.0843;
    hash = 0; 
    b->ckey = &jj;
    _hash_two(b, &hash);
    assert(hash < INITIALSIZE);
    b->ckey = &kk;
    _hash_two(b, &hash);
    assert(hash < INITIALSIZE);
    assoc_free(b);

    b = assoc_init(sizeof(float));
    ll = 15162.1626;
    mm = 37386.122;
    b->ckey = &ll;
    _hash_two(b, &hash);
    assert(hash < INITIALSIZE);
    b->ckey = &mm;
    _hash_two(b, &hash);
    assert(hash < INITIALSIZE);
    assoc_free(b);

    b = assoc_init(sizeof(unsigned long));
    nn = 8127282839916836;
    b->ckey = &nn;
    _hash_two(b, &hash);
    assert(hash < INITIALSIZE);
    assoc_free(b);

    /* Test assoc_free function*/
    assoc_free(a);

    /*Test _probe function*/
    a = assoc_init(0);
    a->hash_table[2].flag = true;
    hash = 2;
    cc = 7353;
    p = &hash;
    a->ckey = &cc;
    _probe(a, p);
    assert(*(int*)p == 13);
    
    a->hash_table[13].flag = true;
    hash = 2;
    ee = 37363;
    p = &hash;
    a->ckey = &ee;
    _probe(a, p);
    assert(*(int*)p == 10);
    
    a->hash_table[5].flag = true;
    hash = 5;
    ff = 2282;
    p = &hash;
    a->ckey = &ff;
    _probe(a, p);
    assert(*(int*)p == 4);

    a->hash_table[16].flag = true;
    hash = 16;
    hh = 272728;
    p = &hash;
    a->ckey = &hh;
    _probe(a, p);
    assert(*(int*)p == 12);

    assoc_free(a);

    /* Test _add_data function*/
    a = assoc_init(sizeof(int));
    key = 7261537;
    p = &key;
    hash = 13;
    a->ckey = p;
    a->cdata = NULL;
    _add_data(a, hash);
    assert(*(int*)(a->hash_table[hash].key) == key);
    assert(a->hash_table[hash].flag == true);

    data = 753892;
    d = &data;
    hh = 12;
    hash = 11;
    a->ckey = d;
    a->cdata = &hh;  
    _add_data(a, hash);
    assert(*(int*)(a->hash_table[hash].key) == data);
    assert(*(int*)(a->hash_table[hash].data) == 12);
    assert(a->hash_table[hash].flag == true);
    
    b = assoc_init(0);

    strcpy(str, "Hello, World!");
    hash = 16;
    b->ckey = str;
    b->cdata = NULL;
    _add_data(b, hash);
    assert(strcmp(str, (char *)b->hash_table[hash].key) == 0);

    assoc_free(a);
    assoc_free(b);

    /* Test _add_hash function*/
    a = assoc_init(sizeof(int));
    num = 2333289;
    p = &num;
    a->ckey = p;
    a->cdata = NULL;
    assert(_add_hash(a));
    assert(a->hash_table[9].flag == true);
    assert(*(unsigned int*)(a->hash_table[9].key) == num);
    key = 4701931;
    d = &key;
    a->ckey = d;
    a->cdata = NULL;
    assert(_add_hash(a));
    assert(a->hash_table[4].flag == true);
    assert(*(int*)(a->hash_table[4].key) == key);

    b = assoc_init(0);
    strcpy(str2, "I hate C");
    p = &str2;
    b->ckey = p;
    b->cdata = NULL;
    assert(_add_hash(b));
    assert(b->hash_table[15].flag == true);
    assert(strcmp(str2, (char*)b->hash_table[15].key)== 0);

    strcpy(str2, "I actually love C");
    p = &str2;
    b->ckey = p;
    b->cdata = NULL;
    assert(_add_hash(b));
    assert(b->hash_table[0].flag == true);
    assert(strcmp(str2, (char*)b->hash_table[0].key)== 0);

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
    assert(b->size == 0);
    assoc_free(a);
    a = _realloc(b);
    assert(a->capacity == 293);

    assoc_free(a);
    assoc_free(b);

    /*Test _rehash*/
    a = assoc_init(sizeof(int));
    cc = 875386;
    ee = 278464;
    ff = 52283;
    gg = 9364;
    hh = 75322;
    c = &cc;
    e = &ee;
    f = &ff;
    g = &gg;
    h = &hh;
    a->ckey = c;
    a->cdata = NULL;
    _add_hash(a);
    a->ckey = e;
    _add_hash(a);
    a->ckey = f;
    _add_hash(a);
    a->ckey = g;
    _add_hash(a);
    a->ckey = h;
    _add_hash(a);
    b = _realloc(a);
    assert(_rehash(a, b));

    /*Show hashed into different position*/
    assert(*(int*)a->hash_table[9].key == cc);
    assert(*(int*)b->hash_table[62].key == cc);
    assert(*(int*)a->hash_table[10].key == ee);
    assert(*(int*)b->hash_table[8].key == ee);
    assert(*(int*)a->hash_table[11].key == ff);
    assert(*(int*)b->hash_table[46].key == ff);
    assert(*(int*)a->hash_table[16].key == gg);
    assert(*(int*)b->hash_table[25].key == gg);
    assert(*(int*)a->hash_table[15].key == hh);
    assert(*(int*)b->hash_table[48].key == hh);
    
    ii = 52;
    i = &ii;
    b->ckey = i;
    b->cdata = NULL;
    _add_hash(b);

    assoc_free(a);
    assoc_free(b);

    /*Test assoc_insert, assoc_search and assoc_count*/
    a = assoc_init(sizeof(int));
    a->ckey = e;
    a->cdata = NULL;
    _add_hash(a);
    a->ckey = f;
    a->cdata = NULL;
    _add_hash(a);
    a->ckey = g;
    a->cdata = NULL;
    _add_hash(a);
    a->ckey = h;
    a->cdata = NULL;
    _add_hash(a);
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

    b->ckey = c;
    hash1 = _search(b);
    assert(!strcmp(hash1.key, c));
    b->ckey = d;
    hash1 = _search(b);
    assert(!strcmp(hash1.key, d));
    b->ckey = e;
    hash1 = _search(b);
    assert(!strcmp(hash1.key, e));
    b->ckey = f;
    hash1 = _search(b);
    assert(!strcmp(hash1.key, f));
    b->ckey = g;
    hash1 = _search(b);
    assert(!strcmp(hash1.key, g));
    b->ckey = h;
    hash1 = _search(b);
    assert(!hash1.flag);

    assert(assoc_count(b) == 5);
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

    /*Test _isduplicate*/
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

    b->ckey = &str2;
    assert(_isduplicate(b));
    b->ckey = e;
    assert(_isduplicate(b));
    b->ckey = f;
    assert(_isduplicate(b));
    b->ckey = g;
    assert(_isduplicate(b));

    a = _realloc(b);
    a->ckey = c;
    assert(!_isduplicate(a));
    a->ckey = d;
    assert(!_isduplicate(a));
    a->ckey = e;
    assert(!_isduplicate(a));

    _rehash(b, a);
    a->ckey = c;
    assert(_isduplicate(a));
    a->ckey = e;
    assert(_isduplicate(a));
    a->ckey = f;
    assert(_isduplicate(a));
    a->ckey = g;
    assert(_isduplicate(a));

    assoc_free(a);
    assoc_free(b);

    a = assoc_init(sizeof(int));
    cc = 873873;
    ee = 270264;
    ff = 526283;
    gg = 937464;
    hh = 763322;
    c = &cc;
    e = &ee;
    f = &ff;
    g = &gg;
    h = &hh;
    a->cdata = NULL;
    a->ckey = c;
    _add_hash(a);
    a->ckey = e;
    _add_hash(a);
    a->ckey = f;
    _add_hash(a);
    a->ckey = g;
    _add_hash(a);
    a->ckey = h;
    _add_hash(a);
    b = _realloc(a);
    assert(_rehash(a, b));

    a->ckey = c;
    assert(_isduplicate(a));
    a->ckey = e;
    assert(_isduplicate(a));
    a->ckey = f;
    assert(_isduplicate(a));
    a->ckey = g;
    assert(_isduplicate(a));

    b->ckey = c;
    assert(_isduplicate(b));
    b->ckey = e;
    assert(_isduplicate(b));
    a->ckey = f;
    assert(_isduplicate(b));
    g = &ff;
    assoc_insert(&b, g, &gg);


    assoc_free(a);
    assoc_free(b);

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
    h = &str6;
    cc = 4;
    ee = 78;

    ff = 523;
    gg = 69;
    hh = 123;
    assoc_insert(&b, c, &cc);
    assoc_insert(&b, e, &ee);
    assoc_insert(&b, f, &ff);
    assoc_insert(&b, g, &gg);
    assoc_insert(&b, &str6, &hh);
    assoc_insert(&b, &str5, &hh);

    assoc_free(b);

    free(str);

}


