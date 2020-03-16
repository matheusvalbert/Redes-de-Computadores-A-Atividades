#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

struct Mensagem 
{
	char nome[20];
	char mensagem[80];
};

struct GuardarMensagens 
{
	struct Mensagem array[10];
	int count;
};

struct infocliente
{
	struct sockaddr_in client;
	int ns;
};

struct GuardarMensagens mensagensSalvas;
pthread_mutex_t 	mutex;
void 			INThandler(int);
int s;                     /* Socket para aceitar conexoes       */

void *tratamento(void *informacoes)
{
	int operacao, flag = 0, podeInserir, i, j;
	char nome[20];
	struct GuardarMensagens mensagensExcluidas;
	struct Mensagem mensagem;
	struct sockaddr_in client;
	struct infocliente info;
	int ns;
	info = *(struct infocliente*) informacoes;
	client = info.client;
	ns = info.ns;
	
	while(operacao != 4) 
	{

		/*
		 * Recebe qual operacao o cliente deseja realizar
		 */
		if (recv(ns, &operacao, sizeof(int), 0) == -1) 
		{
			perror("Recv()");
			exit(6);
		}
		switch(operacao) 
		{		

			case 1:
				pthread_mutex_lock(&mutex);
				if(mensagensSalvas.count < 10)
				{

					podeInserir = 0;
					if (send(ns, &podeInserir, sizeof(int), 0) < 0) 
					{
							perror("Send()");
							exit(5);
					}
					/* 
					 *Recebe e salva uma mensagem e o nome do cliente
					 */
					if (recv(ns, &mensagem, sizeof(struct Mensagem), 0) == -1) 
					{
						perror("Recv()");
						exit(6);
					}
					strcpy(mensagensSalvas.array[mensagensSalvas.count].nome, mensagem.nome);
					strcpy(mensagensSalvas.array[mensagensSalvas.count].mensagem, mensagem.mensagem);
					mensagensSalvas.count++;
					printf("Operacao realizada com sucesso. Nova mensagem inserida - IP: %s - Porta: %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
				}
				else 
				{
					podeInserir = 1;
					if (send(ns, &podeInserir, sizeof(int), 0) < 0) 
					{
						perror("Send()");
						exit(5);
					}
					printf("Erro de execucao. Nova mensagem nao inserida - IP: %s - Porta: %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
				}
				pthread_mutex_unlock(&mutex);

			break;
			case 2:
				/* 
				 * Envia todas as mensagens inseridas para o cliente
				 */
				pthread_mutex_lock(&mutex);
				if (send(ns, &mensagensSalvas, sizeof(struct GuardarMensagens), 0) < 0) 
				{
					perror("Send()");
					exit(5);
				}
				pthread_mutex_unlock(&mutex);
				printf("Operacao realizada com sucesso. Exibicao de mensagens solicitada IP: %s Porta: %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
			break;
			case 3:
				/* 
				 * Recebe o nome do usuario que possuira suas mensagens deletas
		 		 */
				mensagensExcluidas.count = 0;
				if (recv(ns, nome, sizeof(nome), 0) == -1) 
				{
					perror("Recv()");
					exit(6);
				};
				j = 0;
				pthread_mutex_lock(&mutex);
				while(j <= mensagensSalvas.count) 
				{
					for(i = 0; i < mensagensSalvas.count; i++) 
					{
						if(strcmp(mensagensSalvas.array[i].nome, nome) == 0 && flag == 0) 
						{
							flag = 1;
							strcpy(mensagensExcluidas.array[mensagensExcluidas.count].nome,mensagensSalvas.array[i].nome);
							strcpy(mensagensExcluidas.array[mensagensExcluidas.count].mensagem,mensagensSalvas.array[i].mensagem);
							mensagensExcluidas.count++;
						}

						if(flag == 1) 
						{
							strcpy(mensagensSalvas.array[i].nome,mensagensSalvas.array[i+1].nome);
							strcpy(mensagensSalvas.array[i].mensagem,mensagensSalvas.array[i+1].mensagem);
						}
					}
						
					if(flag == 1)
						mensagensSalvas.count--;
					flag = 0;
					j++;
				}
				j = 0;
				/* 
				 * Envia as mensagens excluidas para o cliente 
				 */
				if (send(ns, &mensagensExcluidas, sizeof(struct GuardarMensagens), 0) < 0) 
				{
					perror("Send()");
					exit(5);
				}
				pthread_mutex_unlock(&mutex);
				printf("Operacao realizada com sucesso. Nova exclusao de mensagens solicitada IP: %s Porta: %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
			break;
		}

	}
	close(ns);
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	pthread_t tratarClientes;
	unsigned short port; 
	int ns;                    /* Socket conectado ao cliente        */                   
	struct sockaddr_in client;
	struct sockaddr_in server; 
	struct infocliente informacoes;
	int namelen, tc, i = 0;
	void *ret;
	mensagensSalvas.count = 0;
	signal(SIGINT, INThandler);

	if (pthread_mutex_init(&mutex, NULL) != 0) 
    	{ 
        	printf("falha iniciacao semaforo\n");
       		return 1; 
    	}
	
	if (argc != 2) 
	{

		fprintf(stderr, "Use: %s porta\n", argv[0]);
		exit(1);
	}

	port = (unsigned short) atoi(argv[1]);
	
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
	{

		perror("Socket()");
		exit(2);
	}

	server.sin_family = AF_INET;   
   	server.sin_port   = htons(port);       
   	server.sin_addr.s_addr = INADDR_ANY;

		 
    	if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) 
	{
       		perror("Bind()");
       		exit(3);
   	}
	if (listen(s, 1) != 0) 
	{
		perror("Listen()");
       		exit(4);
   	}
	namelen = sizeof(client);

	while(1)
	{
		if ((ns = accept(s, (struct sockaddr *) &client, (socklen_t *) &namelen)) == -1)
		{
			perror("Accept()");
			exit(5);
		}
    		/*
		 * Cria uma thread para atender o cliente
		 */
		informacoes.ns = ns;
		informacoes.client = client;
    		tc = pthread_create(&tratarClientes, NULL, tratamento, &informacoes);
    		if (tc) 
		{
     			printf("ERRO: impossivel criar um thread consumidor\n");
      			exit(-1);
    		}
		/*
		 * A thread principal dorme por um pequeno período de tempo
		 * para a thread de tratamento retirar as informacoes do client
		 */
    		usleep(250);
  	}
	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);
}

/*
 * Handler caso o usuario aperte "CTRL" e "c"(ou "C") ao mesmo tempo
 * para interromper o processo do servidor
 */

void  INThandler(int sig)
{	
	int i = 0;
	void *ret;
	pthread_mutex_destroy(&mutex);
	close(s);
	pthread_exit(NULL);
	exit(0);
}







