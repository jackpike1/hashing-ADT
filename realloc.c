#include "specific.h"
#include "../../ADTs/General/general.h"
#include <assert.h>
#include <string.h>

#define INITIALSIZE 17

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


void _assoc_test(void) {

    int key, data;
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
    
    /* Test assoc_free function*/
    assoc_free(a);
    assoc_free(b);

    free(str);



}
