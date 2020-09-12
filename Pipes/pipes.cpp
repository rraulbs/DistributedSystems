/*
 * TP1 - Distributed systems
 * pipes.cpp
 *
 *  Created on: 2020
 *      Team:
 *      	Raul
 *			Osmar
 */

#include <iostream>
#include <string>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>		/*	<unistd.h>: This header file is not part of C or C++. It is provided by POSIX-compatible systems	*/
using namespace std;
//=====================================================================
//the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RED			"\033[31m"			/* Red			*/
#define GREEN		"\033[32m"			/* Green		*/
#define RESET		"\033[0m"
//=====================================================================
#define READ 0
#define WRITE 1
#define SIZE 20			// Tamanho fixo da string
int delta_size = 100;	// Varia��o do delta. N�mero aleat�rio entre 1 e 100
int fd[2];
/*	Cria estrutura para o pipe;
//	fd[0]; -> for using read end; [0]: leitura;
	fd[1]; -> for using write end; [1]: escrita.	*/

bool isPrimeNumber(int n){
	n = abs(n);
	switch(n){
		case 0:
			return false;
		case 1:
			return false;
		default:
			for(int i = 2; i <= (n/2 + 1); i++ ){
				if(n%i == 0){		// n n�o � primo
					return false;
				}
			}
			break;
	}
    return true; //	n � primo
}

void consumidor(){
    char number[SIZE];
    close(fd[WRITE]);
    while(read(fd[READ], number, SIZE) > 0){
        if(isPrimeNumber(stoi(number))){
        	cout << stoi(number) << GREEN << " � um n�mero primo!" << RESET <<endl;
        }
        else{
        	cout << stoi(number) << RED << " N�o � um n�mero primo!" << RESET <<endl;
        }
    }
    close(fd[READ]);
    exit(0);
}

void produtor(int n){
    /*	srand(time(NULL)): Objetiva inicializar o gerador de n�meros aleat�rios
    com o valor da fun��o time(NULL). Este por sua vez, � calculado
    como sendo o total de segundos passados desde 1 de janeiro de 1970
    at� a data atual. Desta forma, a cada execu��o o valor da "semente" ser� diferente.	*/
    srand(time(NULL));

    string number;		// O n�mero ser� armazenado nessa string
    int N_i = 1;		// N_0 = 1;
    int delta = rand()%delta_size;
    close(fd[READ]);	// Fecha a leitura do processo

    int count = 1;
    while(count<= n){
    	number = to_string(N_i);	// Converte o n�mero para string
        write(fd[WRITE], number.c_str(), SIZE);
        N_i = N_i + delta;			// N_i = N_i-1 + delta
        count++;
    }
    number = to_string(0);
    write(fd[WRITE], 0, SIZE);		// 0: Enviado para sinalizar o fim da sequ�ncia.
    close(fd[WRITE]);	// Fecha a escrita do processo
    exit(0);
}

int main (int argc, char** argv){
    setlocale(LC_ALL, "portuguese");
    if(argc!=2){
    	string message;
    	message	= 	"Argumento inv�lido!\n"
    				"Devem ser passados dois argumentos:\n"
    				"\t1) O pr�prio programa\n"
    				"\t2) O n�mero de n�meros que devem ser gerados pelo programa Produtor.\n";
        cout << RED << message << RESET <<endl;
        return 0;
    }
    pipe(fd); 			// pipe() is used for passing information from one process to another. pipe() is unidirectional
    pid_t pid = fork(); // To create child process we use fork()
    if(pid > 0){ 		// Processo: pai ou produtor (When >0 parent process will execute.)
		produtor(atoi(argv[1]));
	}
	else if(pid == 0){	// Processo: filho ou consumidor (=0 for child process)
		consumidor();
	}
    else{ 				//fork falhou (<0 fail to create child (new) process)
        cout << RED << "A fun��o fork() falhou!" << RESET <<endl;
    }
    return (EXIT_SUCCESS);
}
