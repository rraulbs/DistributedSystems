/*
 * TP2 - Distributed systems
 * somador_Spinlocks.cpp
 *
 *  Created on: 2020
 *      Team:
 *      	Raul
 *			Osmar
 */
#include <pthread.h> //	This header file is not part of C or C++.
//	C Library ========================
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <clocale>
//====================================
//	Containers
#include <vector>
//====================================
//	Atomics and threading library	==
#include <atomic>
//====================================
//	Input/Output Stream Library	======
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

vector<int8_t> valores;	//	Aloca mem�ria utilizando apenas 1 byte para cada n�mero
long int soma = 0;		//	Acumulador compartilhado
long int N;				//	Quantidade de n�meros a serem gerados e somados
int K;					//	O n�mero de threads
int parcela;			//	Quantidade m�xima de n�meros que cada thread deve somar.
double mean_time = 0;	//  Usado para o estudo de caso, rodar 10 vezes para calcular o tempo m�dio de execu��o em cada combina��o N,K

class Spinlock{
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
    void acquire()
    {
        while(locked.test_and_set()){}
    }
    void release()
    {
        locked.clear();
    }
};

void *somador(void *arg){
	//	Cada thread deve executar um somador com sua devida parcela de n�meros a serem somados
	int t_id = *((int*)arg); // Or: long *p = (int*)arg; int t_id = *p;
    int soma_parcela = 0;
    int init = parcela*t_id;
	Spinlock spinlock;
    for (long int i=0; i < parcela; i++){
        if ((i+init) >= N){
            break;
        }
        soma_parcela += ("%d", valores[i+init]);	//	Converte int8_t para int
    }

    spinlock.acquire();
    soma += soma_parcela;	// Regi�o cr�tica.
    spinlock.release();

    pthread_exit(NULL);
}

void somadorComSpinlocks (long int n, int k){
	clock_t inicio, fim;
	valores.resize(n);	//	Redimensiona o vetor para N
	srand(time(NULL));
	for(long int i = 0; i < n; i++){			//	Gera os N n�meros e preenche o vetor com N posi��es
		valores[i] = ("%x", rand()%201-100);	//	Gera n�meros no intervalo [-100, 100] utilizando apenas um byte para cada n�mero
//        cout << "valores[" << i << "] = " << signed(valores[i]) <<endl;
	}

    pthread_t threads[k];	//	Array que cont�m as threads
    int tNum[k];			//	Array criado para resolver o problema de condi��o de corrida na cria��o das threads durante o loop e na passagem do argumento.
	inicio = clock();
    for(long i=0; i<k; i++){// Esse loop cria as threads
        tNum[i] = i;		// This resolve the problem of race condition.
    	int success = pthread_create(&threads[i],NULL,&somador, &tNum[i]);
        if(success != 0){
            cout << RED << "Failed to create thread." << RESET <<endl;
        }
    }
    //	Esse loop aguarda as threads terminarem de executar.
    for (int i = 0; i < k; i++){
        pthread_join(threads[i], NULL);
		//	Having main join with other threads is a common way to
		//	make sure that main doesn�t finish before the other threads,
		//	and kill them all via exit method one.
    }
    fim = clock();
//	PARA ESTUDO CASO: TEMPO M�DIO (Rodando n vezes)
    mean_time = mean_time + double(fim-inicio)/CLOCKS_PER_SEC;
//	PARA TEMPO DE EXECU��O:
    cout << GREEN << "Soma Total: " << soma << RESET <<endl;
    cout << GREEN << "Tempo de execu��o: "<<  double(fim-inicio)/CLOCKS_PER_SEC << " segundos." << RESET <<endl;
}

int main(int argc, char** argv){
    setlocale(LC_ALL, "portuguese");
	if(argc != 3){
    	string message;
    	message	= 	"Argumento inv�lido!\n"
    				"Devem ser passados dois argumentos:\n"
    				"\t1) O pr�prio programa\n"
					"\t2) O n�mero de threads\n"
    				"\t3) O n�mero de n�meros que devem ser gerados pelo programa.\n";
        cout << BOLDRED << message << RESET <<endl;
        return 0;
	}
	cout << CYAN << "K = " << argv[1] << " (K = N�mero de threads)" << RESET <<endl;
	cout << CYAN << "N = " << argv[2] << " (N = N�mero de n�meros)" << RESET <<endl;
	K = atoi(argv[1]);	//	K = N�mero de threads
	N =	atoi(argv[2]);	//	N = quantidade de n�meros gerados
	//	Calcula o valor da parcela. O n�mero de n�meros que cada thread deve somar
	parcela = trunc(N/K);
	float resto = N%K;
	if (resto != 0){
		parcela += 1;
	}
//	cout << CYAN << "Parcela = " << parcela << " (Parcela m�xima de cada thread)" << RESET <<endl;
	somadorComSpinlocks(N, K);
//	PARA ESTUDO CASO: TEMPO M�DIO (Rodando n vezes)
//	int n = 10;
//	for(int i=0; i<n; i++){
//		soma = 0;
//		somadorComSpinlocks(N, K);
//	}
//	cout << GREEN << "Tempo m�dio de execu��o: "<< mean_time/n << " segundos." << RESET <<endl;
    return 0;
}
