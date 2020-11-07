/*
 * TP3 - Distributed systems
 * Centralized algorithm: processo.cpp
 *
 *  Created on: 2020
 *      Team:
 *      	Raul
 *			Osmar
 */
#include <pthread.h> //	This header file is not part of C or C++.
#include <sys/types.h>	/*	The <sys/types.h> header contains a number of basic derived types that should be used whenever appropriate.	*/
#include <sys/socket.h>	/* See here what the <sys/socket.h> header defines: https://pubs.opengroup.org/onlinepubs/007908799/xns/syssocket.h.html 	*/
#include <netinet/in.h>	/* See here what the <netinet/in.h> header defines: https://pubs.opengroup.org/onlinepubs/007908799/xns/netinetin.h.html 	*/
#include <arpa/inet.h>	/* See here what the <arpa/inet.h> header defines: https://pubs.opengroup.org/onlinepubs/007908799/xns/arpainet.h.html 	*/
#include <unistd.h>		/*	<unistd.h>: This header file is not part of C or C++. It is provided by POSIX-compatible systems	*/
//	C Library ========================
#include <cstdlib>
#include <cstdio>
#include <clocale>
#include <ctime>
#include <chrono>
#include <cstring>
#include <csignal>
//====================================
//	Containers
#include <string>
//====================================
//	Atomics and threading library	==
#include <atomic>
//====================================
//	Input/Output Stream Library	======
#include <iostream>
#include <fstream>
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
#define PORT 1234
#define SIZE 10
#define REQUEST 1
#define GRANT 2
#define RELEASE 3
//=====================================================================
// Casos de teste (mudar aqui e também no outro arquivo 'coordenador.cpp'):
#define N_processos 2
#define R 10
#define K 1
//=====================================================================
int lista_sockets[N_processos];	//	Array que contém os sockets
fd_set readfds;
int max_sd = 0;
int activity;
struct timeval tv;
ofstream outfile ("resultado.txt");
ofstream foutput;
ifstream finput;
bool running = true;
double runtime = 0;	//  Usado para estudo de caso

string getCurrentTimestamp(){
	using std::chrono::system_clock;
	auto currentTime = std::chrono::system_clock::now();
	char buffer[80];

	auto transformed = currentTime.time_since_epoch().count() / 1000000;

	auto millis = transformed % 1000;

	std::time_t tt;
	tt = system_clock::to_time_t ( currentTime );
	auto timeinfo = localtime (&tt);
	strftime (buffer,80,"%F %H:%M:%S",timeinfo);
	sprintf(buffer, "%s:%03d",buffer,(int)millis);

	return std::string(buffer);
}

void init_Sockets(){
	for(int i = 0; i < N_processos; i++){
		lista_sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
	}
}

void *configClientSocket(void *arg){
	init_Sockets();
	for(int i = 0; i < N_processos; i++){
			struct sockaddr_in address_server;
			//	Configuração da estrutura de dados do socket:
			address_server.sin_family = AF_INET;
			address_server.sin_port = htons(PORT);
			address_server.sin_addr.s_addr = INADDR_ANY;
		    cout << CYAN << "\n===========================\nInicializando <client>..." << RESET <<endl;

			//	Conecta o socket a um endereço de socket remoto
		    if (connect(lista_sockets[i], (struct sockaddr*) &address_server, sizeof(address_server)) < 0){
		    	cout << RED << "\nConnection Failed \n" << RESET <<endl;
		    }
			cout << CYAN <<"Conexão estabelecida"<< RESET <<endl;
	}
//	while(running==true){
//		FD_ZERO(&readfds);
//		for(int i = 0; i < N_processos; i++){
//		    // Set Socket fd
//			FD_SET(lista_sockets[i], &readfds);
//			max_sd = (max_sd>lista_sockets[i])?max_sd:lista_sockets[i];
//		}
//		tv.tv_sec = 2;
//		tv.tv_usec = 500000;
//	   	activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);// &tv
//	    if (activity < 0){
//	           printf("select failed\n ");
//	    }
//	}
	pthread_exit(NULL);
}

void processo(){
	clock_t inicio, fim;
    pthread_t threads[1];	//	Array que contém as threads
    int tNum[1];			//	Array criado para resolver o problema de condição de corrida na criação das threads durante o loop e na passagem do argumento.
    tNum[0] = 0;	// This resolve the problem of race condition.
    //==================================================================================================
    // Thread
    //==================================================================================================
    int success_1 = pthread_create(&threads[0],NULL,&configClientSocket, &tNum[0]); //&tNum[i]
    if(success_1 != 0){
        cout << RED << "Failed to create thread 1." << RESET <<endl;
    }

    sleep(2);
    string pid = to_string(getpid());
    send(lista_sockets[0], pid.c_str(),pid.size(), 0);

	string sNumber;	// A mensagem será armazenada nessa string
	char cNumber[SIZE];
	int mensagem;
	int identificador;
	int sep;		// Referente ao segundo separador, já que o primeiro sempre estará na posição 2
	bool tratar_request = true, tratar_grant = false, tratar_release = false;
	int repetition = 0;
	sleep(3);
	inicio = clock();
	while (repetition < R){
		repetition = repetition + 1;
		if(tratar_request == true){
        	for (int i = 0; i < N_processos; i++){
//        		sleep(1);
        		// REQUEST MESSAGE:
				sNumber.append(to_string(REQUEST));
				sNumber.append("|");
				sNumber.append(to_string(lista_sockets[i]));
				sNumber.append("|");
				while(sNumber.size() < SIZE){
					sNumber.append("0");
				}
//				cout << "Enviando requisições" <<  sNumber <<endl;
				////////////////////////////////////////////////
		        send(lista_sockets[i], sNumber.c_str(), SIZE, 0);
		        sNumber = "";
//		        sleep(2);
        	}
        	tratar_request=false;
        	tratar_grant=true;
		}
    	if(tratar_grant){
//    		sleep(5);
    		int loop_grant = 0;
    		// GRANT MESSAGE:
    		while(loop_grant != N_processos){
        		for (int i = 0; i<N_processos; i++){
            		if(recv(lista_sockets[i], cNumber, SIZE, 0)){ //MSG_DONTWAIT: NON-BLOCKING //recv(lista_sockets[i], cNumber, SIZE, MSG_DONTWAIT) != -1
            			loop_grant = loop_grant + 1;
    		        	for (int i = 0; i < SIZE; i++){
    		            	sNumber = sNumber + cNumber[i];
    		            	if (cNumber[i]== '|'){
    		            		sep = i;
    		            	}
    		            }
    		            // Decodificar a mensagem: Mensagem|Identificador|000...
    		            mensagem = stoi(sNumber.substr(0,1));
    		            identificador = stoi(sNumber.substr(2,sep));
//    		            sleep(1);
//    		            cout << "Mensagem: " << mensagem << " e identificador: " <<identificador<<endl;
//    		            sleep(1);
    			        sNumber = "";
    			        // ABRIR O ARQUIVO resultado.txt para escrita em modo append obter a hora atual do sistema, escrever o seu identificador e a hora atual (incluindo milisegundos)
    			        finput.open("resultado.txt");
    			        foutput.open("resultado.txt", ios::app);
    			        if(finput.is_open()){
    				        foutput << "Identificador\t" << identificador << "\tHora atual\t" << getCurrentTimestamp() <<endl;
    			        } // Data has been appended to file
    			        sleep(K);
    			        finput.close();
    			        foutput.close();
    			        // RELEASE MESSAGE:
    					sNumber.append(to_string(RELEASE));
    					sNumber.append("|");
    					sNumber.append(to_string(lista_sockets[i]));
    					sNumber.append("|");
    					while(sNumber.size() < SIZE){
    						sNumber.append("0");
    					}
//    					cout << "Enviando liberação: " << sNumber << endl;
    			        send(lista_sockets[i],sNumber.c_str(), SIZE, 0);
    			        sNumber = "";
    					////////////////////////////////////////////////
            		}
        		}
    		}
//            sleep(1);
        	tratar_request=true;
        	tratar_grant=false;
//            sleep(1);
    	}
    	cout << "Loop> r = " << repetition <<endl;
	}
    fim = clock();
    runtime = double(fim-inicio)/CLOCKS_PER_SEC;
    cout << GREEN << "Tempo de execução: "<<  runtime << " segundos." << RESET <<endl;
	running = false;
    pthread_join(threads[0], NULL);
}

int main(){
    setlocale(LC_ALL, "portuguese");
    processo();
    cout << CYAN << "Encerrando...\n===========================\n" << RESET <<endl;
    // Fecha os sockets
	for (int i = 0; i < N_processos; i++){
		close(lista_sockets[i]);
	}
    return 0;
}
