#include <pthread.h>
#define MAX_MESSAGE_LENGTH 100

struct ChatMessage
{
  char message[MAX_MESSAGE_LENGTH];
  char from[20];
  int type; // 0 - subscribe | 1 - message
};

struct ChatSocket
{
  int socketId;
  char clientName[20];
  pthread_t thread;
  int subscribed;
};

typedef struct ChatMessage chatMessage;
typedef struct ChatSocket chatSocket;

void copyString(char *from, char *to)
{
  for (int i = 0; i < strlen(from); i++)
    to[i] = from[i];

  if (to[strlen(to) - 1] == '\n')
    to[strlen(to) - 1] = '\0';
}