#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "./utils.c"

int mySocket;
int conn;
int connected;

int quitChat(char *msg)
{
	if (msg[0] == 'q' && msg[1] == 'u' && msg[2] == 'i' && msg[3] == 't')
		return 1;
	else
		return 0;
}

void *receivedHandler(void *arg)
{
	chatMessage cm;
	while (connected)
	{
		if (read(mySocket, &cm, sizeof(chatMessage)) > 0)
			printf("%s: %s\n", cm.from, cm.message);
		else
			connected = 0;
	}
	printf("\nDisconnecting...\n");
	close(mySocket);
	close(conn);
	exit(0);
}

void main(int argc, char *argv[])
{

	struct sockaddr_in target;
	pthread_t t_accept;
	char *msg;
	chatMessage cm;

	mySocket = socket(AF_INET, SOCK_STREAM, 0);
	target.sin_family = AF_INET;
	target.sin_port = htons(8081);
	target.sin_addr.s_addr = inet_addr("127.0.0.1");

	system("clear");
	printf("Connecting...\n");
	conn = connect(mySocket, (struct sockaddr *)&target, sizeof target);
	if (conn == 0)
	{
		connected = 1;
		pthread_create(&t_accept, NULL, receivedHandler, (void *)&conn);
		msg = &cm.from[0];
		printf("\tDigite 'quit' para sair do chat!\n\n");
		printf("Your name: ");
		fgets(msg, MAX_MESSAGE_LENGTH, stdin);
		copyString(msg, &cm.from[0]);
		msg = &cm.message[0];

		cm.type = 0;

		if (send(mySocket, &cm, sizeof(chatMessage), 0) < 0)
			printf("error\n");
		cm.type = 1;

		do
		{
			fgets(msg, MAX_MESSAGE_LENGTH, stdin);

			copyString(msg, &cm.message[0]);

			if (send(mySocket, &cm, sizeof(chatMessage), 0) < 0)
				printf("error\n");
			printf("");
		} while (!quitChat(&cm.message[0]));

		close(mySocket);
		close(conn);
		printf("\nQuiting...\n");
	}
	else
		printf("Server not found.\n");
}