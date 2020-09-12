/*
 * TP1 - Distributed systems
 * Sockets: consumidor.cpp
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
int client_fd = socket(AF_INET, SOCK_STREAM, 0);

bool isPrimeNumber(int n){
	n = abs(n);
	switch(n){
		case 0:
			return false;
		case 1:
			return false;
		default:
			for(int i = 2; i <= (n/2 + 1); i++ ){
				if(n%i == 0){		// n não é um número primo
					return false;
				}
			}
			break;
	}
    return true; //	n é um número primo
}

void configClientSocket(int socket){
	struct sockaddr_in address_server;
	//	Configuração da estrutura de dados do socket:
	address_server.sin_family = AF_INET;
	address_server.sin_port = htons(PORT);
	address_server.sin_addr.s_addr = INADDR_ANY;
    cout << CYAN << "\n===========================\nInicializando <client>..." << RESET <<endl;

	//	Conecta o socket a um endereço de socket remoto
    if (connect(socket, (struct sockaddr*) &address_server, sizeof(address_server)) < 0)
    {
    	cout << RED << "\nConnection Failed \n" << RESET <<endl;
    }
	cout << CYAN <<"Conexão estabelecida"<< RESET <<endl;

}

void consumidor(int socket){
	string sNumber;	// A mensagem anuncia se número recebido é ou não primo
	char cNumber[SIZE];	// Recebe o número enviado pelo Produtor
    recv(socket, cNumber, SIZE, 0);	// Recebe os dados do socket através da variável cNumber
    bool loop = true;
	while(loop){
		switch(stoi(cNumber)){
			case 0:
				cout << CYAN << "Todos os números foram verificados" << RESET <<endl;
				loop = false;
				break;
			default:
				cout << YELLOW << "Número " << stoi(cNumber) << " recebido!" << RESET <<endl;
				if (isPrimeNumber(stoi(cNumber))){
					cout << YELLOW << "Enviando resposta..." << RESET <<endl;
					//cout << GREEN << "O número " << stoi(cNumber) << " é primo"<< RESET <<endl;
					sNumber = to_string(1);
				}
				else{
					cout << YELLOW << "Enviando resposta..." << RESET <<endl;
					//cout << RED << "O número " << stoi(cNumber) << " não é primo" << RESET <<endl;
					sNumber = to_string(2);
				}
				send(socket, sNumber.c_str(), SIZE, 0); // Envia os dados através do socket (convertido para string)
				recv(socket, cNumber, SIZE, 0);	// Recebe outro número e continua o loop
				cout << endl;
				break;

		}
	}
}

int main(){
    setlocale(LC_ALL, "portuguese");
    configClientSocket(client_fd);
    consumidor(client_fd);
    cout << CYAN << "Encerrando...\n===========================\n" << RESET <<endl;
	//fecha o socket
	close(client_fd);
    return 0;
}
