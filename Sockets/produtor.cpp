/*
 * TP1 - Distributed systems
 * Sockets: produtor.cpp
 *
 *  Created on: 2020
 *      Team:
 *      	Raul
 *			Osmar
 */

#include <sys/types.h>	/*	The <sys/types.h> header contains a number of basic derived types that should be used whenever appropriate.	*/
#include <sys/socket.h>	/* See here what the <sys/socket.h> header defines: https://pubs.opengroup.org/onlinepubs/007908799/xns/syssocket.h.html 	*/
#include <netinet/in.h>	/* See here what the <netinet/in.h> header defines: https://pubs.opengroup.org/onlinepubs/007908799/xns/netinetin.h.html 	*/
#include <arpa/inet.h>	/* See here what the <arpa/inet.h> header defines: https://pubs.opengroup.org/onlinepubs/007908799/xns/arpainet.h.html 	*/
#include <unistd.h>		/*	<unistd.h>: This header file is not part of C or C++. It is provided by POSIX-compatible systems	*/
#include <cstdlib>
#include <cstdio>
#include <clocale>
#include <string>
#include <iostream>
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
#define SIZE 20
int delta_size = 100;	// Variação do delta. Número aleatório entre 1 e 100
int server_fd = socket(AF_INET, SOCK_STREAM, 0);

void configServerSocket(int socket){
    /*	system call: cria um socket
     * 	int sockfd = socket(domain, type, protocol)
     * 		domain: integer, communication domain e.g., AF_INET (IPv4 protocol) , AF_INET6 (IPv6 protocol)
     * 		type: communication type
     * 			SOCK_STREAM: TCP(reliable, connection oriented)
				SOCK_DGRAM: UDP(unreliable, connectionless)
			protocol: Protocol value for Internet Protocol(IP), which is 0.
			This is the same number which appears on protocol fielsd in the IP header of a packet.
     * 		*/
	struct sockaddr_in address_server;
	//Configuração da estrutura de dados do socket:
    address_server.sin_family = AF_INET;	/* Address family		*/
    address_server.sin_port = htons(PORT);	/* Port number			*/
    address_server.sin_addr.s_addr = INADDR_ANY;	/* Internet address		*/
    cout << CYAN << "\n===========================\nInicializando <server>..." << RESET <<endl;
    // Associa o socket ao endereço do socket local:
    if (bind(socket, (struct sockaddr*) &address_server, sizeof (address_server)) < 0){
        perror("bind failed");
        close(socket);
        exit(EXIT_FAILURE);
    }
    // Avisa o socket que novas conexões devem ser aceitas;
    if (listen(socket, 1) < 0){	// 1: Define que só é aceito uma conexão.
        perror("listen");
        exit(EXIT_FAILURE);
    }
    cout << CYAN << "Aguardando <client>..." << RESET <<endl;
}

void produtor(int socket, int n){
	srand (time(NULL));
	string sNumber;	// O número será armazenado nessa string
	char cNumber[SIZE];
    int N_i = 1;	// N_0 = 1;
    int delta = rand()%delta_size;
	for (int i = n; i > 0; i--){
		sNumber = to_string(N_i);
		send(socket, sNumber.c_str(),SIZE, 0);
        N_i = N_i + delta;	// N_i = N_i-1 + delta

		cout << YELLOW <<"Enviando número "<< sNumber <<"..."<< RESET <<endl;

		recv(socket, cNumber, SIZE, 0);
		cout << YELLOW <<"Resposta recebida! "<< RESET <<endl;
		// O consumidor envia '1' ou '2', cada número associado a uma resposta;
		if ((stoi(cNumber))==1){
			cout << GREEN <<"O número "<< sNumber <<" é primo"<< RESET <<endl;
		}
		else if ((stoi(cNumber))==2){
			cout << RED <<"O número "<< sNumber <<" não é primo"<< RESET <<endl;
		}
		cout << endl;
	}
	sNumber = to_string(0);	// '0'é o sinal de encerramento do programa
	send(socket, sNumber.c_str(), SIZE, 0);	//	Sinaliza que todos os números foram enviados
}

int main(int argc, char** argv){
    setlocale(LC_ALL, "portuguese");
    if(argc!=2){
    	string message;
    	message	= 	"Argumento inválido!\n"
    				"Devem ser passados dois argumentos:\n"
    				"\t1) O próprio programa\n"
    				"\t2) O número de números que devem ser gerados pelo programa Produtor.\n";
        cout << BOLDRED << message << RESET <<endl;
        return 0;
    }
	configServerSocket(server_fd);

	struct sockaddr_in address_client;
	socklen_t size_address_client = sizeof (address_client);
    // accept(): É usado para aceitar uma solicitação de conexão no soquete do servidor
    int new_socket = accept(server_fd, (struct sockaddr*) &address_client, &size_address_client);

    cout << CYAN << "Conexão estabelecida" << RESET <<endl;
    produtor(new_socket, atoi(argv[1]));
    cout << CYAN << "Encerrando...\n===========================\n" << RESET <<endl;
    // Fecha os sockets
    close(new_socket);
    close(server_fd);
    return 0;
}
