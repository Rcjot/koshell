#include <stdio.h>
#include <stdlib.h>
#include "koshell.h"

void init_dynarray( DynArgv *arr) {
  arr -> data = malloc(5 * sizeof(char *));
  arr -> size = 0;
  arr -> capacity = 5;
}

void push(DynArgv *arr, char *token) {
  if (arr -> size == arr -> capacity) {
    printf("reallocating, current size is %d\n", arr -> size);
    size_t new_capacity = (arr -> capacity * 2);
    char **temp_ptr = realloc(arr -> data, new_capacity *  sizeof(char *));
    arr -> capacity = new_capacity; 

    if (temp_ptr == NULL) {
      printf("Can't allocate more memory");
      return;
    } else {
      arr -> data = temp_ptr;
    }
  }

  char **index_to_push = arr -> data + arr -> size;
  *index_to_push = token;
   // dereferencing
  // can also do : arr->data[arr->size] = num;
  arr -> size++;
}


void free_data(DynArgv *arr) {
  for (int i = 0; i < arr -> size; i++) {
    free(arr -> data[i]);
  }
  free(arr -> data);
}
