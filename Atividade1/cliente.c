#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio_ext.h>


/*
 * Cliente UDP
 */

int main(argc, argv)
int argc;
char **argv;
{


	int s, server_address_size;
	unsigned short port;
	struct sockaddr_in server;
	char bufSend[201];
	char bufReceive[2001];

	/* 
	 * O primeiro argumento (argv[1]) é o endereço IP do servidor.
	 * O segundo argumento (argv[2]) é a porta do servidor.
	 */

	if(argc != 3)
	{
		printf("Use: %s enderecoIP porta\n",argv[0]);
		exit(1);
	}
	port = htons(atoi(argv[2]));

	/*
	 * Cria um socket UDP (dgram).
	 */

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket()");
		exit(1);
	}

	/* Define o endereço IP e a porta do servidor */
	server.sin_family      = AF_INET;            /* Tipo do endereço         */
	server.sin_port        = port;               /* Porta do servidor        */
	server.sin_addr.s_addr = inet_addr(argv[1]); /* Endereço IP do servidor  */
	while(1)
	{
		printf("> ");
		__fpurge(stdin);
		/* Recebendo a string desejada */
		fgets(bufSend, sizeof(bufSend), stdin);

		/* Remoção da adição da nova linha */
		if(bufSend[strlen(bufSend)-1] == '\n')
			bufSend[strlen(bufSend)-1] = '\0';

		/* Finalização utilizando o exit */
		if(strcmp(bufSend,"exit") == 0)
		{	
			close(s);
			return 0;
		}

		/* Envia a mensagem no buffer para o servidor */

		if (sendto(s, bufSend, strlen(bufSend)+1, 0, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
			perror("sendto()");
			exit(2);
		}
		
		//recebe resposta do servidor
		server_address_size = sizeof(server);
		if(recvfrom(s, bufReceive, sizeof(buf), 0, (struct sockaddr *)&server,&server_address_size) <0)
		{
			perror("recvfrom()");
			exit(1);
		}

		//print da resposta do servidor
		printf("\nResposta do Servidor:\n%s\n", bufReceive);
	}


   return 0;
}
