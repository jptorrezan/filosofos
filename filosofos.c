#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
//Book example

#define     N       5               //numero de filosofos
#define     LEFT    (i+N-1)%N       //filosofo da esquerda
#define     RIGHT   (i+1)%N         //filosofo da direita

#define PENSANDO    0
#define FOME        1
#define COMENDO     2

int state[N]; //estado de cada filosofo

//funcao dos filosofos
void eat();
void think();
void take_forks(int i);
void put_forks(int i);
void test(int i);


//===============================================
// FRACAO DO CODIGO REFERENTE AO MONITOR
//===============================================
//monitor fornecido em 
// Define the data we need to create the monitor
typedef struct monitor_DataType {
  sem_t OKtoRead;
  sem_t OKtoWrite;
  int readerCount;
  int isBusyWriting;
  
  // The read-queue
  int readRequested;
} monitor;
struct monitor_DataType monitor_data; 

// Function that will block until write can start
void monitor_StartWrite(monitor *monitor_data) {
  if(monitor_data->isBusyWriting || monitor_data->readerCount != 0){
    sem_wait(&(monitor_data->OKtoWrite));
  }
  monitor_data->isBusyWriting++;    // Using 1 as true
}

// Function to signal reading is complete
void monitor_EndWrite(monitor *monitor_data) {
  monitor_data->isBusyWriting--;
  if(monitor_data->readRequested){
    sem_post(&(monitor_data->OKtoRead));
  } else {
    sem_post(&(monitor_data->OKtoWrite));
  }
}

// Function that will block until read can start
void monitor_StartRead(monitor *monitor_data) {
  if(monitor_data->isBusyWriting){
    monitor_data->readRequested++;
    sem_wait(&(monitor_data->OKtoRead));
    monitor_data->readRequested--;
  }
  monitor_data->readerCount++;
  sem_post(&(monitor_data->OKtoRead));
}

// Function to signal reading is complete
void monitor_EndRead(monitor *monitor_data) {
  monitor_data->readerCount--;
  if(monitor_data->readerCount == 0){
    sem_post(&(monitor_data->OKtoWrite));
  }
}


// intialize the monitor
// return's 0 on success, just like sem_init()
int monitor_Initialized(monitor *monitor_data){  
    int returnValue = 1;

    // Initialize the structure
    monitor_data->readerCount = 0;
    monitor_data->isBusyWriting = 0;
    monitor_data->readRequested = 0;
    
    // initialize the semaphores
    if(sem_init(&(monitor_data->OKtoWrite), 0, 1) == 0 && 
       sem_init(&(monitor_data->OKtoRead), 0, 1) == 0){
        returnValue = 0;
    } else {
      printf("Unable to initialize semaphores\n");
    }
    return returnValue;
}

// Destroys the semphores->
void monitor_Destroy(monitor *monitor_data){
  sem_destroy(&(monitor_data->OKtoWrite));
  sem_destroy(&(monitor_data->OKtoRead));
}

//===============================================
//FRAÇAO DO CODIGO REFERENTE AO FILOSOFO
//===============================================

/*
Funcao que administra o filosofo, eh um loop infinito
que alterna entre pensando e comendo, de acordo com a
diponibilidade de garfos.
*/
void* filosofo(void* filo_number){
    int *i = (int*) filo_number;   
    printf("[START]:Filosofo numero %d inciado\n", *i);
    while(1){
        think(*i);        //acao de pensar (apenas aguarda um tempo rand)
        take_forks(*i);   //pegar garfos
        eat(*i);          //comer (apenas aguarda um tempo rand)
        put_forks(*i);    //devolver garfos
    }
}

//pegar garfos
void take_forks(int i){
    monitor_StartRead(&monitor_data);   //inicio da leitura, bloqueando monitor
    state[i] = FOME;                    //filosofo parou de pensar, logo FOME
    test(i);                            //testa se há garfos disponiveis, se houver desbloqueia monitor de leitura
    while(state[i] != COMENDO); 
}

//guardar garfos
void put_forks(int i){
    monitor_StartWrite(&monitor_data);  //bloqueia monitor escrita
    state[i] = PENSANDO;                //parou de comer, logo PENSAR
    test(LEFT);                         //testa se estao com fome e se há garfos disponiveis  para os laterais, se houver desbloqueia monitor de leitura
    test(RIGHT);                        //   ||
    monitor_EndWrite(&monitor_data);    //desbloqueia monitor de escrita
}

//testar disponibilidades
void test(int i){
  if (state[i] == FOME && state[LEFT] != COMENDO && state[RIGHT] != COMENDO){    //checa se há fome e se ha garfos
    state[i] = COMENDO;   
    monitor_EndRead(&monitor_data); //desbloqueia um monitor de leitura
  }
    
}

//funcao eat() apenas espera um tempo randomico
void eat(int i){
    int a = rand()%10+1;
    printf("[PID:%li]:Filosofo [%d] comendo \tpor [%d] segundos\n", pthread_self(),i, a);
    sleep(a);
    
}

//funcao think() apenas espera um tempo randomico
void think(int i){
    int a = rand()%10+1;
    printf("[PID:%li]:Filosofo [%d] pensando \tpor [%d] segundos\n",pthread_self(), i, a);
    sleep(a);
}

//incializa os states dos filosofos, todos começam pensando
void initialization_code() { 
    for (int i = 0; i < N; i++)
        state[i] = PENSANDO;
}

int main(){

    initialization_code();
    pthread_t f[N];
    int a[N] = {0, 1, 2, 3, 4};

    printf("[MAIN]:threads sendo criadas.\n");
    pthread_create(&f[a[0]], NULL, filosofo, (void *)&a[0]); 
    pthread_create(&f[a[1]], NULL, filosofo, (void *)&a[1]);
    pthread_create(&f[a[2]], NULL, filosofo, (void *)&a[2]);
    pthread_create(&f[a[3]], NULL, filosofo, (void *)&a[3]);
    pthread_create(&f[a[4]], NULL, filosofo, (void *)&a[4]);

    

    while(1); 
}

