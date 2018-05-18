
#define _REETRANT
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"


#define MAX_CUSTOMERS_THREADS 50                    // Número máximo de threads de clientes
#define MAX_CUSTOMERS 20                            // Número máximo de clientes que a barbearia suporta
#define COUCH_SEATS 3                               // Número de espaços no sofá
#define WAITING_ROOM_SPACE 13                       // Espaço na sala de espera em pé


// Protótipo das funções principais
void *customer(void *num);
void *barber(void *);

void randwait(int secs){
    int len;
    // Gera um número aleatório
    len = (int) ((drand48() * secs) + 1);
    sleep(len);
}

// DEFINIÇÃO DE SEMÁFOROS

// waitingRoom define o número de pessoas esperando em pé
sem_t waitingRoom;

// couch limita o tanto de clientes que podem sentar no sofá ao mesmo tempo
sem_t couch;   

// barberPillow é usado para sinalizar que o barbeiro está dormindo
// até que um novo cliente apareça
sem_t barberPillow;

// barberChair garante o acesso mutualmente exclusivo às cadeiras
sem_t barberChair;

// seatBelt é usado para fazer o cliente esperar até que 
// o barbeiro termine de cortar seu cabelo 
sem_t seatBelt;

// workBarber é usado para sinalizar se o barbeiro deve ou não começar a trabalhar
sem_t workBarber;


// Flag que para os barbeiros caso todos os clientes
// tenham sido atendidos
int allDone = 0;


// semNextCust é usado para controlar o próximo cliente e não haver deadlocks/concorrência
// queueNextCust é a fila que garante a ordem de chegada dos clientes 
sem_t semNextCust;
struct Queue *queueNextCust;


// Todos os semáforos e structs à seguir tem a mesma função do anterior: manter a fila e 
// evitar threads concorrentes ou deadlocks
sem_t semNextSofa;
struct Queue *queueNextSofa;

sem_t semNextWr;
struct Queue *queueNextWr;

sem_t semCutHair;
struct Queue *queueCutHair;

// semPaid controla aqueles clientes que já efetuaram o pagamento
sem_t semPaid;

// ableToPay indica aqueles clientes que podem efetuar o pagamento, ou seja
// quando o barbeiro está presente para receber o dinheiro
sem_t ableToPay;

// semNumCustomersOnShop é o que indica, como o nome diz, quantos clientes há
// na barbearia, para podermos controlar a lotação
int numCustomersOnShop = 0;
sem_t semNumCustomersOnShop;


// MAIN DO PROGRAMA

int main(int argc, char *argv[]) {

    // Declaração das threads e variáveis
    pthread_t btid[2];
    pthread_t tid[MAX_CUSTOMERS_THREADS];
    long RandSeed;
    int i, numCustomers;
    int Number[MAX_CUSTOMERS_THREADS], Barbers[3];


    // Checagem dos argumentos passados por linha de comando
    if (argc != 3) {
    printf("Use: SleepBarber <Numero Clientes> <Seed Random>\n");
    exit(-1);
    }

    // Pegando os argumentos e os convertendo para as variáveis necessárias
    numCustomers = atoi(argv[1]);
    RandSeed = atol(argv[2]);

    // Checa de o número de clientes passados por argumento é menor do que
    // o número máximo de threads estipuladas
    if (numCustomers > MAX_CUSTOMERS_THREADS) {
    printf("O numero maximo de clientes eh: %d.\n", MAX_CUSTOMERS_THREADS);
    exit(-1);
    }

    printf("\nBarberMain.c\n\n");
    printf("Solucao implementada para o Halziers Sleeping Barber Problem, para a disciplina de SO1 2018/1\n");

    // Inicializa o gerador de números aleatórios com a seed 
    srand48(RandSeed);


    // Indexando os barbeiros
    for (i=0; i<3;i++){
        Barbers[i] = i;
    }

    // Criação das filas de controle
    queueNextWr = createQueue(15);
    queueNextSofa = createQueue(4);
    queueNextCust = createQueue(3);
    queueCutHair = createQueue(3);



    // Inicialização dos semáforos com seus valores iniciais
    sem_init(&waitingRoom, 0, WAITING_ROOM_SPACE);
    sem_init(&couch, 0, COUCH_SEATS);
    sem_init(&barberChair, 0, 3);
    sem_init(&barberPillow, 0, 0);
    sem_init(&seatBelt, 0, 0);
    sem_init(&workBarber, 0, 0);
    sem_init(&semNextSofa,0,1);
    sem_init(&semNextCust,0,1);
    sem_init(&semNextWr,0,1);
    sem_init(&semCutHair,0,1);
    sem_init(&semPaid,0,1);
    sem_init(&ableToPay,0,1);
    sem_init(&semNumCustomersOnShop,0,1);


    // Criação das threads dos barbeiros
    for(i=0;i<3;i++){
        pthread_create(&btid[i], NULL, barber, (void *)&Barbers[i]);
    }

    // Inicialização do array de números
    for (i=0; i<MAX_CUSTOMERS_THREADS; i++){
        Number[i] = i;
    }

    // Criação das threads dos clientes
    for (i=0; i<numCustomers; i++) {
        pthread_create(&tid[i], NULL, customer, (void *)&Number[i]);
    }

    // Join das threads que faz com que todas esperem a finalização
    for (i=0; i<numCustomers; i++) {
    pthread_join(tid[i],NULL);
    }


    // Quando todos os clientes foram atendidos, a flag é alterada
    allDone = 1;


    sem_post(&barberPillow);  // Acordando os barbeiros
    sem_post(&barberPillow);
    sem_post(&barberPillow);
    sleep(1);


    printf("A barbearia esta fechada.\n");

    return 0;
}


// FUNÇÃO CUSTOMER (CLIENTE)

void *customer(void *number) {
    int num = *(int *)number;

    // Os clientes saem para ir até a barbearia e chegam após um
    // intervalo de tempo aleatório
    printf("Cliente %d esta indo para a barbearia.\n", num);
    randwait(5);
    printf("Cliente %d chegou na barbearia.\n", num);

    numCustomersOnShop++;

    // Caso a barbearia não esteja lotada, executa normalmente as threads
    if(numCustomersOnShop <= MAX_CUSTOMERS){

        sem_wait(&waitingRoom);                                                
        sem_wait(&semNextWr);
        enqueue(queueNextWr, num);
        printf("Cliente %d entrou na barbearia.\n", num);

        // Aguarda até que haja lugar na sala de espera
        sem_post(&semNextWr);
        sem_wait(&couch);
        sem_wait(&semNextSofa);
        num = dequeue(queueNextWr);
        sem_post(&waitingRoom);
        enqueue(queueNextSofa, num);
        printf("Cliente %d sentando no sofa.\n", num);

        // Aguarda até que uma cadeira de barbeiro esteja livre
        sem_post(&semNextSofa);
        sem_wait(&barberChair);

        // A cadeira é liberada, então fica livre um espaço na área de espera
        sem_wait(&semNextCust);
        num = dequeue(queueNextSofa);
        sem_post(&couch);
        enqueue(queueNextCust, num);
        sem_post(&semNextCust);

        // Acorda o barbeiro
        printf("Cliente %d acordando o barbeiro.\n", num);
        sem_post(&barberPillow);
        sem_post(&workBarber);

        // Aguarda até que o barbeiro corte o cabelo
        sem_wait(&seatBelt);

        // Sai da cadeira
        sem_post(&barberChair);
        sem_wait(&semCutHair);
        num = dequeue(queueCutHair);
        sem_post(&semCutHair);

        // Vai efetuar o pagamento
        sem_post(&ableToPay);
        sem_wait(&semPaid);

        // Cliente saindo da barbearia
        sem_wait(&semNumCustomersOnShop);
        numCustomersOnShop--;
        sem_post(&semNumCustomersOnShop);
        printf("Cliente %d esta saindo da barbearia.\n", num);
    }else{
        printf("A barbearia esta lotada. Cliente %d esta indo pra casa.\n", num);
    }
}


//FUNÇÃO BARBER (BARBEIRO)

void *barber(void *numberbarber) {
    int numnextcustomer = 0;
    int numbarber = *(int*)numberbarber;

        // Enquanto ainda existirem clientes chegando à barbearia
        while (!allDone) {
                
            // O barbeiro dorme até que algum cliente o acorde
            printf("O barbeiro %d esta dormindo. \n", numbarber);
            sem_wait(&barberPillow);
            
            if (!allDone) {

                // O barbeiro gasta uma quantidade aleatoria de tempo
                // para cortar o cabelo do cliente
                sem_wait(&semNextCust);
                numnextcustomer = dequeue(queueNextCust);
                sem_post(&semNextCust);
                printf("O barbeiro %d esta cortando o cabelo do cliente %d.\n", numbarber, numnextcustomer);

                sem_wait(&workBarber);
                randwait(3);
                printf("O barbeiro %d terminou o corte.\n", numbarber);

                sem_wait(&semCutHair);
                enqueue(queueCutHair, numnextcustomer);
                sem_post(&semCutHair);

                // Solta o cliente da cadeira quando o corte acaba
                sem_post(&seatBelt);
                sem_wait(&ableToPay);

                printf("O barbeiro %d esta recebendo o pagamento do cliente %d.\n", numbarber, numnextcustomer);
                randwait(3);

                sem_post(&semPaid);
                sem_post(&ableToPay);
                numnextcustomer = 0;

        }else {
            printf("O barbeiro %d esta indo para casa.\n", numbarber);
        }
    }
}