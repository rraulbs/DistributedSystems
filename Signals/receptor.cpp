/*
 * TP1 - Distributed systems
 * Signals: receptor.cpp
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

void signalHandler(int signal){
	if (signal == SIGINT){
		cout << YELLOW << "Receptor: SIGUINT tratado" << RESET<<endl;
	}
    else if(signal == SIGUSR1){
        cout << YELLOW << "Receptor: SIGUSR1 tratado" << RESET<<endl;
    }
    else if(signal == SIGUSR2){
        cout << YELLOW << "Receptor: SIGUSR2 tratado" << RESET<<endl;
    }
}

void receptor(int wait){
    cout << CYAN << "Receptor: ID do Processo = " << GREEN << getpid() << RESET <<endl;

    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    signal(SIGINT, signalHandler);

    if(wait==0){
        cout << GREEN <<"Busy wait" << RESET<<endl;
        while(1){
            sleep(1);
        }
    }
    else if(wait==1){
        cout << GREEN << "Blocking wait" << RESET<<endl;
        while(1){
            pause();
        }
    }
    else{
        cout << RED << "Argumento inválido!\n" << RESET<<endl;
    }
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "portuguese");
	string message;
	message	= 	"Argumento inválido!\n"
				"Devem ser passados dois argumentos:\n"
				"\t1) O próprio programa\n"
				"\t2) Escolha o tipo de wait: Busy = 0 ou Blocking = 1\n";
    if(argc != 2){
        cout << RED << message << RESET <<endl;
        return 0;
    }
    receptor(atoi(argv[1]));
    return (EXIT_SUCCESS);	//Both EXIT_SUCCESS and the value zero indicate successful program execution status (see std::exit)
}
