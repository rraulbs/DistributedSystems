/*
 * TP1 - Distributed systems
 * Signals: emissor.cpp
 *
 *  Created on: 2020
 *      Team:
 *      	Raul
 *			Osmar
 */

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <clocale>
#include <sys/types.h>	/*	The <sys/types.h> header contains a number of basic derived types that should be used whenever appropriate.	*/
#include <unistd.h>		/*	<unistd.h>: This header file is not part of C or C++. It is provided by POSIX-compatible systems	*/

using namespace std;

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

void emissor (int pid, int signal){
	// pid: Número do processo de destino
	// signal: Sinal que deve ser enviado
	switch (kill(pid,0)){
		case 0:
			switch(signal){
				case 0:
					cout << YELLOW << "Emissor: SIGKILL enviado" << RESET <<endl;
					kill(pid, SIGKILL);
					break;
				case 1:
					kill(pid, SIGUSR1);
					cout << YELLOW << "Emissor: SIGUSR1 enviado" << RESET <<endl;
					break;
				case 2:
					kill(pid, SIGUSR2);
					cout << YELLOW << "Emissor: SIGUSR2 enviado" << RESET <<endl;
					break;
				case 3:
					kill(pid, SIGINT);
					cout << YELLOW << "Emissor: SIGINT enviado" << RESET <<endl;
					break;
				default:
					string message;
					message	= 	"Argumento signal inválido!\n"
								"Escolha os seguintes sinais:\n"
								"\tSIGKILL = 0 (Esse sinal termina a execução do programa)\n"
								"\tSIGUSR1 = 1\n"
								"\tSIGUSR2 = 2\n"
								"\tSIGINT = 3";
		            cout << RED << message << RESET <<endl;
		            break;
			}
			break;
		default:
            cout << RED << "Emissor: Argumento inválido! PID incorreto" << RESET <<endl;
            break;
	}
}

int main (int argc, char** argv) {
    setlocale(LC_ALL, "portuguese");
	string message;
	message	= 	"Argumento inválido!\n"
				"Devem ser passados três argumentos:\n"
				"\t1) O próprio programa\n"
				"\t2) O pid do processo receptor (receptor deve ser iniciado primeiro)\n"
				"\t3) Escolha o sinal há ser enviado:\n"
				"\t\t SIGKILL = 0 (Esse sinal termina a execução do programa)\n"
				"\t\t SIGUSR1 = 1\n"
				"\t\t SIGUSR2 = 2\n"
				"\t\t SIGINT = 3\n";
    if(argc != 3){
    	// Verifica se os parâmetros passados estão corretos, se não estiver uma mensagem de erro é exibida,
    	// Caso esteja correto, o programa envia o sinal desejado
        cout << RED << message << RESET <<endl;
        return 0;
    }
	emissor(atoi(argv[1]), atoi(argv[2]));
    return (EXIT_SUCCESS); //Both EXIT_SUCCESS and the value zero indicate successful program execution status (see std::exit)
}
