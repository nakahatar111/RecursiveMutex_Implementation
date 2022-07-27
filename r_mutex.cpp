#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include <math.h> 
#include <ctype.h>
#include <cassert>
using namespace std;
using namespace std::chrono;
int count = 0;

class r_mutex{
  private:
    pthread_cond_t condRec;
    pthread_mutex_t mutexRec;
    pthread_t currentThreadId;
    int lock_count = 0;
  public:
    r_mutex(){
      pthread_cond_init(&condRec, NULL);
      pthread_mutex_init(&mutexRec, NULL);
    }
    ~r_mutex(){
      assert(pthread_cond_destroy(&condRec)==0);
      assert(pthread_mutex_destroy(&mutexRec)==0);
    }

    void mutex_recursive_lock(){
      pthread_mutex_lock(&mutexRec);
      if(lock_count == 0)
        currentThreadId = pthread_self();

      if(pthread_equal(currentThreadId, pthread_self()))
        lock_count++;
      else{
        while(lock_count > 0)
          pthread_cond_wait(&condRec, &mutexRec);
        currentThreadId = pthread_self();
        lock_count = 1;
      }
      pthread_mutex_unlock(&mutexRec);
    }
    
    void mutex_recursive_unlock(){
      pthread_mutex_lock(&mutexRec);
      if(lock_count == 1){
        currentThreadId = -1;
        lock_count = 0;
        pthread_mutex_unlock(&mutexRec);
        pthread_cond_signal(&condRec); //let waiting thread know
      }
      else if(pthread_equal(currentThreadId, pthread_self())){
        lock_count--;
        pthread_mutex_unlock(&mutexRec);
      }
      else{
        cout << "This thread did not lock the mutex" << endl;
        pthread_mutex_unlock(&mutexRec);
      }
    }
};
r_mutex recursive;

void *increase(void*){
  recursive.mutex_recursive_lock();
  recursive.mutex_recursive_lock();
  recursive.mutex_recursive_lock();
  count ++;
  cout << "count: "<< count << endl;
  recursive.mutex_recursive_unlock();
  recursive.mutex_recursive_unlock();
  recursive.mutex_recursive_unlock();
  pthread_exit(NULL);
}

int main (int argc, char *argv[]){
  if(argc == 2 && isdigit(*argv[1])){
    int NUMINCREASE = stoi(argv[1]);
    pthread_t inc_thread[NUMINCREASE];

    auto start = high_resolution_clock::now();

    for(int i = 0; i < NUMINCREASE; i++){
      int err = pthread_create(&inc_thread[i], NULL, increase, NULL);
      if(err){
        cout << "Error code "<< err << endl; 
        exit(-1);
      }
    }

    for (int t = 0; t < NUMINCREASE; t++){
      int retval = pthread_join(inc_thread[t], NULL);
      if(retval != 0){
        printf("Error occurred when joining the thread\n");
        exit(-1);
      }
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Time: " << duration.count() << " microseconds" << endl;
    
    pthread_exit(NULL);
  }
  else{
    cout << "Argument is not an int" << endl;
  }
}