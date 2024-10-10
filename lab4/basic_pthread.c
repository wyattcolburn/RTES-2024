#include <stdio.h>
#include <pthread.h>


void* add() {
  printf("Hello\n");
  return NULL;
}

int main() {
    pthread_t thread1;
    pthread_create(&thread1, NULL, add, NULL);
    pthread_join(thread1, NULL);
    return 0; 


}
