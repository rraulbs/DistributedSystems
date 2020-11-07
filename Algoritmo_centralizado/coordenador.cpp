
/*
 * TP3 - Distributed systems
 * Centralized algorithm: coordenador.cpp
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
#include <csignal>
//====================================
//	Containers
#include <string>
#include <queue>
#include <vector>			//std::vector
//====================================
//	Atomics and threading library	==
#include <atomic>
//====================================
//	Input/Output Stream Library	======
#include <iostream>
using namespace std;
using std::vector;
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
#define N_threads 3
#define REQUEST 1
#define GRANT 2
#define RELEASE 3
//=====================================================================
// Casos de teste (mudar aqui e também no outro arquivo 'processo.cpp'):
#define N_processos 2
#define R 10
//////////////////////////////////////////////////////////
typedef pair<int, int> pedidos;			// <socket, id>
typedef pair<int, bool> sock_atendido;	// <socket_server, sock_client>
pedidos pedido;
queue<pedidos> fila;
vector<sock_atendido> p_acesso;
//////////////////////////////////////////////////////////
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
int client_socket[N_processos];	//	Array que contém os sockets
int addrlen, new_socket, activity, sd, max_sd;
fd_set readfds;
//const char message[30] = "ECHO Daemon v1.0 \r\n";
struct timeval tv;
//////////////////////////////////////////////////////////
//TESTE - FUTURAMENTE  TIRAR ESSAS 3 VARIAVEIS BOOL...
bool running_1 = true;
bool running_2 = true;
bool running_3 = true;
//////////////////////////////////////////////////////////
bool show_execution = false;
bool first_connection = false;
int pid_other;
int pid_this = getpid();
int repetition = 0;

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

void printQueue(queue<pedidos> q)
{
	//printing content of queue
	while (!q.empty()){
		cout<< q.front().first <<endl;
		q.pop();
	}
	cout<<endl;
}

void *configServerSocket(void *arg){
    /*	system call: cria um socket
     * 	int sockfd = socket(domain, type, protocol)
     * 		domain: integer, communication domain e.g., AF_INET (IPv4 protocol) , AF_INET6 (IPv6 protocol)
     * 		type: communication type
     * 			SOCK_STREAM: TCP(reliable, connection oriented)
				SOCK_DGRAM: UDP(unreliable, connectionless)
			protocol: Protocol value for Internet Protocol(IP), which is 0.
			This is the same number which appears on protocol fields in the IP header of a packet.
     * 		*/
	//Configuração da estrutura de dados do socket:
	p_acesso.resize(N_processos);
	struct sockaddr_in address_server;
    address_server.sin_family = AF_INET;	/* Address family		*/
    address_server.sin_port = htons(PORT);	/* Port number			*/
    address_server.sin_addr.s_addr = INADDR_ANY;	/* Internet address		*/
    cout << CYAN << "\n===========================\nInicializando <server>..." << RESET <<endl;
    // Associa o socket ao endereço do socket local:
    if (bind(server_fd, (struct sockaddr*) &address_server, sizeof (address_server)) < 0){
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    // Avisa o socket que novas conexões devem ser aceitas;
    if (listen(server_fd, N_processos) < 0){	// n_processos: Define que é aceito n conexões, onde n = n_processos.
        perror("listen");
        exit(EXIT_FAILURE);
    }
    cout << CYAN << "Aguardando <client>..." << RESET <<endl;
	//accept the incoming connection
	addrlen = sizeof(address_server);
	puts("Waiting for connections ...");
	while (running_1==true){
		//clear the socket set
		FD_ZERO(&readfds);
		//add master socket to set
		FD_SET(server_fd, &readfds);
		max_sd = server_fd;
		//add child sockets to set
		for (int i = 0 ; i < N_processos ; i++){
			//socket descriptor
			sd = client_socket[i];
			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);
			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
		}
		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		// wait until either socket has data ready to be recv()d (timeout 2.5 secs)
		tv.tv_sec = 2;
		tv.tv_usec = 500000;
		activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);
		if ((activity < 0) && (errno!=EINTR)){
			printf("select error");
		}
		//If something happened on the master socket ,
		//then its an incoming connection
		if (FD_ISSET(server_fd, &readfds)){
			if ((new_socket = accept(server_fd,(struct sockaddr *)&address_server, (socklen_t*)&addrlen)) < 0){
				perror("accept");
				exit(EXIT_FAILURE);
			}
			//inform user of socket number - used in send and receive commands
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address_server.sin_addr) , ntohs(address_server.sin_port));
			//send new connection greeting message
//			if( send(new_socket, message, 30, 0) != 30 ){
//				perror("send");
//			}
//			puts("Welcome message sent successfully");
			//add new socket to array of sockets
			for (int i = 0; i < N_processos; i++){
				//if position is empty
				if( client_socket[i] == 0 )	{
					client_socket[i] = new_socket;
					p_acesso[i].first = new_socket;
					p_acesso[i].second = false;
//					printf("Adding to list of sockets as %d\n" , i);
					first_connection = true;
					break;
				}
			}
		}

	}
	cout << "Encerrando thread 1" <<endl;
	sleep(1);
	pthread_exit(NULL);
}

void *mutualExclusion(void *arg){
	Spinlock spinlock;
	string sNumber;	// O número será armazenado nessa string
	char cNumber[SIZE];
	int mensagem;
	int identificador;
	pedidos temp_pedido;
	bool tratar_request = true, tratar_grant = false, tratar_release = false;
	int temp_req = 0, temp_gra = 0, temp_rel = 0;
//	repetition = 0;
	//=======================================================
	string PID;
	char pid[SIZE];
	while(first_connection==false){
		//Do nothing
	}
	sleep(2);
	recv(client_socket[0], pid, SIZE, 0);
	for(int i = 0; i<SIZE; i++){
		PID = PID + pid[i];
	}
	pid_other = stoi(PID);
	cout << "PID do outro processo: " << pid_other <<"\n"<<endl;
	//=======================================================
	sleep(3);
	while(repetition < R){
    	if(tratar_request){
        	// Receber mensagem dos clientes e guardar requisições na fila (sem duplicação):
        	while(temp_req < N_processos){ //for (int i = 0; i < N_processos; i++)
        		for (int i = 0; i < N_processos; i++){
                    if (FD_ISSET(client_socket[i], &readfds)) {
                    	if(recv(client_socket[i], cNumber, SIZE, MSG_DONTWAIT) != -1){
                        	temp_req = temp_req + 1;
//                            recv(client_socket[i], cNumber, SIZE, MSG_DONTWAIT);
                        	// Converter cNumber para string e marca o separador 2:
                        	int sep;		// Referente ao segundo separador, já que o primeiro sempre estará na posição 2
                        	for (int i = 0; i < SIZE; i++){
                            	sNumber = sNumber + cNumber[i];
                            	if (cNumber[i]== '|'){
                            		sep = i;
                            	}
                            }
                            // Decodificar a mensagem: Mensagem|Identificador|000...
                            mensagem = stoi(sNumber.substr(0,1));
                            identificador = stoi(sNumber.substr(2,sep));
                            if(show_execution==true){
                                cout << CYAN <<"Recebido\t" << RESET << "Mensagem: " << mensagem << " e identificador: " <<identificador<<endl;
                            }
                    		for (int j = 0; j < N_processos; j++){
                    			if(p_acesso[j].first == client_socket[i] && p_acesso[j].second == false){
                                    pedido.first = client_socket[j];
                                    pedido.second = identificador;
                                    spinlock.acquire();
                            		fila.push(pedido);
                            	    spinlock.release();
                            		p_acesso[j].second = true;
//                            		cout << "Tamanho da fila: " << fila.size() <<endl;
                    			}
//                    			cout<< "<" << fila.front().first << ", " << fila.front().second << ">" <<endl;
//                    			cout<< "<" << fila.back().first << ", " << fila.back().second << ">" <<endl;
                    		}
                    		sNumber = "";
                    	}
                    }
                    else{
//                    	cout << "waiting"<<endl;
                    }
        		}
        	}
        	temp_req = 0;
        	tratar_request = false;
        	tratar_grant = true;
    	}
    	if(tratar_grant){
        	// Liberar acesso a uma requisição na cabeça da fila:
        	while (!fila.empty()==true){
            	temp_pedido = fila.front();
        		// GRANT MESSAGE:
        		sNumber.append(to_string(GRANT));
        		sNumber.append("|");
        		sNumber.append(to_string(temp_pedido.second));
        		sNumber.append("|");
    			while(sNumber.size() < SIZE){
    				sNumber.append("0");
    			}
        		for (int i = 0; i < N_processos; i++){	// Dá para melhorar isso, guardando os indices...
        			if (client_socket[i]== temp_pedido.first){
        				if (show_execution == true){
            				cout << CYAN << "Enviando grant" << RESET << endl;
        				}
        				send(client_socket[i], sNumber.c_str(), SIZE, 0);
        				break;
        			}
        		}
        		sNumber = "";
                spinlock.acquire();
        		fila.pop();
        	    spinlock.release();
        	}
        	tratar_grant = false;
        	tratar_release = true;
    	}
    	if(tratar_release){
        	while(temp_rel != N_processos){ //for (int i = 0; i < N_processos; i++)
        		for (int i = 0; i < N_processos; i++){
            		if (FD_ISSET(client_socket[i], &readfds)) {
                    	if(recv(client_socket[i], cNumber, SIZE, MSG_DONTWAIT) != -1){
                			temp_rel = temp_rel + 1;
//                            recv(client_socket[i], cNumber, SIZE, 0);
                        	// Converter cNumber para string e marca o separador 2:
                        	int sep;		// Referente ao segundo separador, já que o primeiro sempre estará na posição 2
                        	for (int i = 0; i < SIZE; i++){
                            	sNumber = sNumber + cNumber[i];
                            	if (cNumber[i]== '|'){
                            		sep = i;
                            	}
                            }
                            // Decodificar a mensagem: Mensagem|Identificador|000...
                            mensagem = stoi(sNumber.substr(0,1));
                            identificador = stoi(sNumber.substr(2,sep));
                    		sNumber = "";
                    		if (show_execution == true){
                                cout << CYAN << "Recebido\t" << RESET << "Mensagem: " << mensagem << " e identificador: " <<identificador<<endl;
                    		}
                            if(mensagem == 3){
                        		for (int i = 0; i < N_processos; i++){
                        			if(p_acesso[i].first == client_socket[i] && p_acesso[i].second == true){
                                		p_acesso[i].second = false;
                        			}
                        		}
                            }
                    	}
            		}
        		}
        	}
        	temp_rel = 0;
        	tratar_request = true;
        	tratar_grant = false;
        	tratar_release = false;
    	}
    	if(running_2 == false){ // Alguém digitou o comando 3, para fechar o programa no terminal!
    		cout << "Encerrando thread 2" <<endl;
    		running_1 = false;
    		pthread_exit(NULL);
    	}
    	repetition = repetition + 1;
	}
	running_2 = false;
	running_1 = false;	// encerra thread 1 (2 depende da 1 aberta)
	cout << "Encerrando thread 2" <<endl;
	sleep(1);
	pthread_exit(NULL);
}

void *interface(void *arg){
	Spinlock spinlock;
	string comando;
	string manual;
	sleep(1);
	manual ="Bem vindo a interface!\n"
			"Após se conectar com os processos:\n"
			"Escolha um comando: 1, 2 ou 3\n"
			"1)\tImprimi a fila de pedidos atual;\n"
			"2)\tImprimi quantas vezes cada processo foi atendido;\n"
			"3)\tEncerra a execução.\n";
	cout << CYAN << manual << RESET <<endl;
	while (running_3==true){
		cout << "Aguardando comando" <<endl;
		cin >> comando;
		cout << "Seu comando foi: " <<comando<<endl;
		if(stoi(comando)==1){
			// NÃO FUNCIONA, PROVAVELMENTE PELA ESTRUTURA DO CÓDIGO, A THREAD 2 DEVE ESTAR RODAR O LOOP INTEIRO
			// SEM TROCAR PARA A THREAD 3 E
			// ASSIM A FILA SEMPRE MOSTRA ESTAR VAZIA... (*.*)'
		    spinlock.acquire();
			cout << YELLOW << "Tamanho da fila: " << fila.size() << RESET <<endl;
			cout << YELLOW << "Imprimindo fila: " << RESET <<endl;
			printQueue(fila);
		    spinlock.release();
		}
		if(stoi(comando)==2){
			cout << YELLOW << "Cada processo foi atendido: " << repetition << RESET <<endl;
		}
		if(stoi(comando)==3){
			running_2 = false;	// TAVA DANDO PROBLEMA, ACABAMOS USANDO KILL.. SEM FECHAR AS threads e sockets
			running_3 = false;
			cout << YELLOW << "SIGKILL enviado" << RESET <<endl;
			kill(pid_other, SIGKILL);
			kill(pid_this, SIGKILL);
		}
	}
	cout << "Encerrando thread 3" <<endl;
	pthread_exit(NULL);
}

void coordenador(){
    pthread_t threads[N_threads];	//	Array que contém as threads
    int tNum[N_threads];			//	Array criado para resolver o problema de condição de corrida na criação das threads durante o loop e na passagem do argumento.
    tNum[0] = 0;	// This resolve the problem of race condition.
    //==================================================================================================
    // Thread para receber a conexão de um processo:
    //==================================================================================================
    int success_1 = pthread_create(&threads[0],NULL,&configServerSocket, &tNum[0]); //&tNum[i]
    if(success_1 != 0){
        cout << RED << "Failed to create thread 1." << RESET <<endl;
    }
    //==================================================================================================
    // Thread para exclusão mútua:
    //==================================================================================================
    int success_2 = pthread_create(&threads[1],NULL,&mutualExclusion, &tNum[1]); //&tNum[i]
    if(success_2 != 0){
        cout << RED << "Failed to create thread 2." << RESET <<endl;
    }
    //==================================================================================================
    // Thread para atender a interface:
    //==================================================================================================
    int success_3 = pthread_create(&threads[2],NULL,&interface, &tNum[2]); //&tNum[i]
    if(success_3 != 0){
        cout << RED << "Failed to create thread 2." << RESET <<endl;
    }
    ///////////////////////////////////////////////////////////////////////////////
    //	Esse loop aguarda as threads terminarem de executar.
    for (int i = 0; i < N_threads; i++){
        pthread_join(threads[i], NULL);
		//	Having main join with other threads is a common way to
		//	make sure that main doesn´t finish before the other threads,
		//	and kill them all via exit method one.
    }
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "portuguese");
    if(argc!=1){
    	string message;
    	message	= 	"Argumento inválido!\n"
    				"Devem ser passados como argumento(s):\n"
    				"\t1) O próprio programa\n";
        cout << BOLDRED << message << RESET <<endl;
        return 0;
    }
	for (int i = 0; i < N_processos; i++)
	{
		client_socket[i] = 0;
	}
    coordenador();
    cout << CYAN << "Encerrando...\n===========================\n" << RESET <<endl;
    // Fecha os sockets
	for (int i = 0; i < N_processos; i++){
		close(client_socket[i]);
	}
    close(server_fd);
	return 0;
}
