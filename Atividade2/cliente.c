#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio_ext.h>

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

/*
 * Cliente TCP
 */
 
int main(int argc, char **argv) 
{

	unsigned short port;             
    struct hostent *hostnm;    
    struct sockaddr_in server; 
    int s;
    char nome[20];
   	int operacao;
    struct Mensagem mensagem;   
    struct GuardarMensagens mensagensSalvas, mensagensExcluidas; 

    /*
     * O primeiro argumento (argv[1]) e o hostname do servidor.
     * O segundo argumento (argv[2]) e a porta do servidor.
     */
	 
    if (argc != 3) 
	{

        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }

    /*
     * Obtendo o endereco IP do servidor
     */
	 
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0) 
	{

        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(argv[2]);

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
	{

        	perror("Socket()");
        	exit(3);
   	}

    /* 
	 * Estabelece conexao com o servidor 
	 */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) 
	{

        perror("Connect()");
        exit(4);
    }

    while(1) 
	{
		/* 
		 * Menu de opções
		 */
		printf("Opcoes:\n");
		printf("1 - Cadastrar mensagem\n");
		printf("2 - Ler mensagens\n");
		printf("3 - Apagar mensagens\n");
		printf("4 - Sair da Aplicação\n");

		scanf("%i", &operacao);

		/* 
		 * Envia qual operação será realizada para o servidor 
		 * Caso a opcão seja 4, é enviado para o servidor o valor 4 e , assim, o servidor volta a receber clientes
		 */
		if (send(s, &operacao, sizeof(int), 0) < 0) 
		{

			perror("Send()");
			exit(5);
	    }

		switch(operacao) {

			case 1:
				printf("Adicionar nova mensagem:\n");
				printf("Nome: ");
				__fpurge(stdin);
				fgets(mensagem.nome, sizeof(mensagem.nome), stdin);
				printf("Mensagem: ");
				__fpurge(stdin);
				fgets(mensagem.mensagem, sizeof(mensagem.mensagem), stdin);
				
				/* 
				 * Envia a mensagem no buffer de envio para o servidor 
				 */
		    	if (send(s, &mensagem, sizeof(struct Mensagem), 0) < 0) 
				{
					perror("Send()");
					exit(5);
		    	}
				else
		    		printf("Mensagem inserida com sucesso\nNome: %s\nMensagem: %s\n", mensagem.nome, mensagem.mensagem);
				break;
			case 2:
				/* 
				 * Recebe uma mensagem do cliente atraves do socket conectado 
				 */
				if (recv(s, &mensagensSalvas, sizeof(struct GuardarMensagens), 0) == -1) {

					perror("Recv()");
					exit(6);
				}
				/*
				 * Exibe as mensagens para o cliente
				 */
				printf("Mensagens existentes: %i\n", mensagensSalvas.count);
				for(int i = 0; i < mensagensSalvas.count; i++)
					printf("Nome: %sMensagem: %s\n", mensagensSalvas.array[i].nome, mensagensSalvas.array[i].mensagem);
				break;
			case 3:
				printf("Apagar mensagem:\n");
				printf("Nome: ");
				__fpurge(stdin);
				fgets(nome, sizeof(nome), stdin);
				/*
				 * Apaga todas as mensagens pertencentes ao usuário desejado
				 */
				if (send(s, nome, strlen(nome)+1, 0) < 0)
		    	{
					perror("Send()");
					exit(5);
		    	}
				else 
				{
					/*
					 * Exibe as mensagens apagadas e o respectivo número de quantas mensagens foram apagadas
					 */
		    		if (recv(s, &mensagensExcluidas, sizeof(struct GuardarMensagens), 0) == -1) 
					{

						perror("Recv()");
						exit(6);
					}
					printf("Mensagens Apagadas %d\n", mensagensExcluidas.count);
					for(int i = 0; i < mensagensExcluidas.count; i++)
						printf("Nome: %sMensagem: %s\n", mensagensExcluidas.array[i].nome, mensagensExcluidas.array[i].mensagem);
				}
				break;
			case 4:
				/*  
				 * Fecha o socket e termina a execução do programa
				 */
				close(s);
				printf("Cliente terminou com sucesso.\n");
				exit(0);
				break;
			default:
				printf("Opcao invalida!\n");
				break;
	    	}
	}
}
