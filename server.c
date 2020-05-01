#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include "./utils.c"

#define PORT "8081" /* Port to listen on */
#define BACKLOG 10  /* Passed to listen() */

struct NodeList
{
  chatSocket socket;
  struct NodeList *next;
} NodeList;

typedef struct NodeList Node;

int sock;
Node listSocket;

void removeClient(int socketId);

void *clientHandler(void *arg);

void addClient(int newSocketId);

int countClients();

void *acceptHandler(void *arg);

int main(void)
{
  struct addrinfo hints, *res;
  int reuseaddr = 1;
  int connectedClients = 0;
  int i = 0;
  chatMessage cm;
  chatSocket cs[20];
  pthread_t t_accept;

  listSocket.next = NULL;

  system("clear");

  /* Get the address info */
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(NULL, PORT, &hints, &res) != 0)
  {
    perror("getaddrinfo");
    return 1;
  }

  /* Create the socket */
  sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sock == -1)
  {
    perror("socket");
    return 1;
  }

  /* Enable the socket to reuse the address */
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1)
  {
    perror("setsockopt");
    return 1;
  }

  /* Bind to the address */
  if (bind(sock, res->ai_addr, res->ai_addrlen) == -1)
  {
    perror("bind");
    return 1;
  }

  /* Listen */
  if (listen(sock, BACKLOG) == -1)
  {
    perror("listen");
    return 1;
  }

  freeaddrinfo(res);

  pthread_create(&t_accept, NULL, acceptHandler, (void *)&listSocket);

  printf("Press any button for shutdown...");

  getchar();

  close(sock);

  printf("Shutting down server...");

  return 0;
}

void removeClient(int socketId)
{
  Node *list = &listSocket;
  Node *removedNode = NULL;
  Node *prev = NULL;

  while (list->next != NULL && removedNode == NULL)
  {
    prev = list;
    list = list->next;
    if (list->socket.socketId == socketId)
      removedNode = list;
  }

  if (removedNode != NULL)
  {
    prev->next = removedNode->next;
    free(removedNode);
  }
}

void *clientHandler(void *arg)
{
  chatMessage cm;
  chatSocket *socket = (chatSocket *)arg;
  int connected = 1;
  while (connected)
  {
    if (read(socket->socketId, &cm, sizeof(chatMessage)) > 0)
    {
      Node *list = &listSocket;
      switch (cm.type)
      {
      case 0:
        copyString(&cm.from[0], &socket->clientName[0]);
        socket->subscribed = 1;
        break;
      case 1:        
        printf("%s: %s\n", cm.from, cm.message);
        while (list->next != NULL)
        {
          list = list->next;
          if (list->socket.socketId != socket->socketId && list->socket.subscribed)
            if (send(list->socket.socketId, &cm, sizeof(chatMessage), 0) <= 0)
              printf("error in broadcast message\n");
        }
      }
    }
    else
      connected = 0;
  }
  printf("socketId %i - %s is disconnected!\n", socket->socketId, socket->clientName);
  removeClient(socket->socketId);
}

chatSocket *getSocket(int socketId)
{
  Node *list = &listSocket;
  while (list != NULL)
  {
    if (list->socket.socketId == socketId)
      return &list->socket;
    list = list->next;
  }
  return NULL;
}

void addClient(int newSocketId)
{
  Node *list = &listSocket;
  Node *newNode = (Node *)(malloc(sizeof(Node)));
  newNode->next = NULL;
  newNode->socket.socketId = newSocketId;
  newNode->socket.subscribed = 0;
  while (list->next != NULL)
    list = list->next;
  list->next = newNode;
  pthread_create(&newNode->socket.thread, NULL, clientHandler, (void *)&newNode->socket);
}

int countClients()
{
  Node *list = &listSocket;
  int i = 0;
  while (list->next != NULL)
  {
    list = list->next;
    i++;
  }
  return i;
}

void *acceptHandler(void *arg)
{
  Node *list = &listSocket;
  socklen_t size = sizeof(struct sockaddr_in);
  struct sockaddr_in their_addr;

  while (1)
  {
    int newSocket = accept(sock, (struct sockaddr *)&their_addr, &size);
    if (newSocket == -1)
      perror("accept");
    else
      addClient(newSocket);
  }
}