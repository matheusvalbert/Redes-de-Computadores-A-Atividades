#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

/*
 * Servidor UDP
 */

int main(argc, argv)
int argc;
char **argv;
{
	FILE *fp;
	int s, server_address_size;
	unsigned short port;
	struct sockaddr_in server;
	char bufReceive[201];
	char bufSend[2001];
	char bufAux[2001];

	if(argc != 2)
	{
		printf("Use: %s porta\n",argv[0]);
		exit(1);
	}

	/*
	 * Cria um socket UDP (dgram). 
	 */

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket()");
		exit(1);
	}

	port = htons(atoi(argv[1]));

	/*
	 * Define a qual endereço IP e porta o servidor estará ligado.
	 * Porta = 0 -> faz com que seja utilizada uma porta qualquer livre.
	 * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
	 * os endereços IP
	 */

	server.sin_family      = AF_INET;   /* Tipo do endereço             */
	server.sin_port        = port;      /* Escolhe uma porta disponível */
	server.sin_addr.s_addr = INADDR_ANY;/* Endereço IP do servidor      */

	/*
	 * Liga o servidor à porta definida anteriormente.
	 */

	if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("bind()");
		exit(1);
	}

	/* Imprime qual porta foi utilizada. 
	 */

	printf("Porta utilizada eh %d\n", ntohs(server.sin_port));

	/*
	 * Recebe uma mensagem do cliente.
	 * O endereço do cliente será armazenado em "client".
	 */

	while(1)
	{
		//recebe o comando do client
		server_address_size = sizeof(server);
		if(recvfrom(s, bufReceive, sizeof(bufReceive), 0, (struct sockaddr *) &server,&server_address_size) <0)
		{
			perror("recvfrom()");
			exit(1);
		}

		//executa o comando do cliente
		fp = popen(bufReceive, "r");
  		if (fp == NULL) {
    			printf("Erro ao executar comando");
			exit(1);
  		}

		//recebe a resposta do cliente em um buffer auxiliar e colocar no buff para ser enviado de volta ao cliente
		while (fgets(bufAux, sizeof(bufAux), fp) != NULL) {

			strcat(bufSend, bufAux);
   		}

		pclose(fp);

		//resposta para comandos que nao apresentam output
		if(bufSend[0] == '\0')
			strcpy(bufSend, "Comando Executado");

		//envia resposta do comando de volta para o cliente
		if (sendto(s, bufSend, strlen(bufSend)+1, 0, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
			perror("sendto()");
			exit(2);
		}

		//limpa buffer de resposta
		bufSend[0] = '\0';
	   /*
	    * Imprime a mensagem recebida, o endereço IP do cliente
	    * e a porta do cliente 
	    */
		printf("Comando Recebido IP: %s Porta: %d\n",inet_ntoa(server.sin_addr),ntohs(server.sin_port));
	}

	/*
	 * Fecha o socket.
	 */

	close(s);

	return 0;
}
