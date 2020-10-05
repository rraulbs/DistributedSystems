/*
 * TP2 - Distributed systems
 * produtorconsumidor_Semaforos.cpp
 *
 *  Created on: 2020
 *      Team:
 *      	Raul
 *			Osmar
 */
#include <semaphore.h>	//	This header file is not part of C or C++.
#include <pthread.h>	//	This header file is not part of C or C++.
//	C Library ========================
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <clocale>
//====================================
//	Containers
#include <vector>
//====================================
//	Input/Output Stream Library	======
#include <sstream>
#include <iostream>
using namespace std;
//====================================
//=====================================================================
//the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
//=====================================================================
int M = 100000;			//	Quantidade de n�meros que dever�o ser processados
int iterations = 0;		//	N�mero de itera��es realizadas iterations <= M
bool finish = false;	//	Torna-se verdadeiro ap�s 10^5 n�meros serem processados
bool print = true;		//	Se true imprimi resultado no terminal (Maior o tempo de execu��o)
sem_t empty, full;		//	Sem�foros contadores
pthread_mutex_t mutex;	//	Mutex: usado para exclus�o m�tua
double mean_time = 0;	//  Usado para o estudo de caso, rodar 10 vezes para calcular o tempo m�dio de execu��o em cada combina��o


bool isPrimeNumber(long int n){ // long int: Pois a Thread produtor deve gerar n�meros inteiros aleat�rios entre 1 e 10^7
	n = abs(n);
	switch(n){
		case 0:
			return false;
		case 1:
			return false;
		default:
			for(int i = 2; i <= int(sqrt(n) + 1); i++){	//	O segundo maior divisor de um n�mero natural � no m�ximo a raiz desse n�mero
				if(n%i == 0){	// n n�o � primo
					return false;
				}
			}
			break;
	}
    return true; //	n � primo
}

void *produtor(void *i){	//	Fun��o executada assim que uma thread � criada
    vector<long int> *p_values = (vector<long int>*)i;
    while(finish == false){
        long int prod = rand() % 10000000;
        //	Espera por semaforos
        sem_wait(&empty);			//	Decrements (locks) the semaphore (wait for empty buffer)
        pthread_mutex_lock(&mutex);	//	Lock buffer list
        //	Regi�o cr�tica	======================
        for(int i = 0; i < p_values->size(); i++){
            if(!finish and p_values->at(i) == 0){
                p_values->at(i) = prod;	//	Insere prod em p_values[i]
                break;
            }
        }
        //	======================================
        //	Libera sem�foros:
        pthread_mutex_unlock(&mutex);	//	Unlock buffer list
        sem_post(&full);				//	Increments the value of a semaphore
    }
    //	Release other threads from their semaphore, so that they can end:
    sem_post(&full);
    sem_post(&empty);
    pthread_exit(NULL);
}

void *consumidor(void *i){	//	Fun��o executada assim que uma thread � criada
    vector<long int> *p_values = (vector<long int>*)i;
    while(iterations < M and finish == false){
        long int value;
        bool value_found = false;
        //	Espera por sem�foros
        sem_wait(&full);			//	Decrements (locks) the semaphore (Wait for a full buffer)
        pthread_mutex_lock(&mutex);	//	Lock buffer list
        //	Regi�o cr�tica	======================
        for(int i=0; i<p_values->size();i++)
        {
            if(!finish and p_values->at(i) != 0)
            {
                value = p_values->at(i);
                value_found = true;
                p_values->at(i) = 0;
                iterations += 1;
                break;
            }
        }
        //	======================================
        //	Libera sem�foros:
        pthread_mutex_unlock(&mutex);	//	Unlock buffer list
        sem_post(&empty);				//	Increments the value of a semaphore
        if(!finish and value_found and print){	//	Consome
            stringstream stream;
            stream << CYAN << "i = " << iterations << "\t" << MAGENTA << value;
            if(!isPrimeNumber(value))            {
                stream << RED << "\tis not prime " << RESET <<endl;
            }
            else{
                stream << GREEN << "\tis prime." << RESET <<endl;
            }
            cout << stream.str();
        }
    }
    finish = true;
    //	Release other threads from their semaphore, so that they can end
    sem_post(&full);
    sem_post(&empty);
    pthread_exit(NULL);
}

void produtorconsumidorComSemaforos(int n, int p, int c){
    pthread_t threads[p+c];	//	Array que cont�m as threads criadas
    vector<long int> values (n);
	clock_t inicio, fim;
    pthread_mutex_init(&mutex, NULL);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, n);
    srand(time(NULL));
	inicio = clock();
    for(int i = 0; i < p; i++){		//	Cria K de threads e envia a cada uma o trecho de n�meros que devem processar
        int success = pthread_create(&threads[i],NULL,&produtor, &values);
        if(success != 0){
            cout << "Failed to create thread." <<endl;
        }
    }

    for(int i = p; i < (p + c); i++){	//	Cria K de threads e envia a cada uma o trecho de n�meros q devem processar
        int success = pthread_create(&threads[i],NULL,&consumidor, &values);
        if(success != 0){
            cout << "Failed to create thread." <<endl;
        }
    }

    for(int i = 0; i< (p + c); i++){	//	Aguarda o fim da execu��o das outras threads
        pthread_join(threads[i], NULL);
        stringstream stream;
        stream << CYAN << "Thread " << i << " ended." << RESET <<endl;
        cout << stream.str();
    }
    fim = clock();
//	PARA ESTUDO CASO: TEMPO M�DIO (Rodando n vezes)
    mean_time = mean_time + double(fim-inicio)/CLOCKS_PER_SEC;
//	PARA TEMPO DE EXECU��O:
    cout << GREEN << "Tempo de execu��o: "<<  double(fim-inicio)/CLOCKS_PER_SEC << " segundos." << RESET <<endl;
    cout << CYAN << "Iterations = " << iterations << RESET <<endl;
    sem_destroy(&full);
    sem_destroy(&empty);
    pthread_mutex_destroy(&mutex);
}

int main(int argc, char** argv){
    setlocale(LC_ALL, "portuguese");
	if(argc != 4){
    	string message;
    	message	= 	"Argumento inv�lido!\n"
    				"Devem ser passados quatro argumentos:\n"
    				"\t1) O pr�prio programa\n"
    				"\t2) O tamanho do vetor de int que ser� compartilhado entre threads\n"
    				"\t3) A quantidade de threads produtoras\n"
    				"\t4) A quantidade de threads consumidoras\n";
        cout << BOLDRED << message << RESET <<endl;
        return 0;
	}
    int N = atoi(argv[1]);	//	N =	Tamanho do vetor de int
    int P = atoi(argv[2]);	//	P = Quantidade de threads produtoras
    int C = atoi(argv[3]);	//	C = Quantidade de threads consumidoras
    produtorconsumidorComSemaforos(N, P, C);
    //	PARA ESTUDO CASO: TEMPO M�DIO (Rodando n vezes)
//    int n = 10;
//    for(int i=0; i<n; i++){
//        produtorconsumidorComSemaforos(N, P, C);
//        iterations = 0;
//        finish = false;
//    }
//    cout << GREEN << "Tempo m�dio de execu��o: "<< mean_time/n << " segundos." << RESET <<endl;
    return 0;
}
