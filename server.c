// Server side C/C++ program to demonstrate Socket
// programming
// gcc server.c -o server -lpthread && ./server L A
// gcc server.c -o server -lpthread && ./server L B
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#define PORT_A 8080
#define PORT_B 8081

#define CLI_START_PORT_A "6000"
#define CLI_START_PORT_B "7000"

#define SERVER_PROP_PORT 5000
#define size 1024
char serverName, Other_serverIP[15], *clientPortCount = NULL;
int currentPort, acceptConn = 0, count = 0;

void *serverA()
{
  printf("server A initializing....\n");
  acceptConn = 1;
  int sock = 0, valread, client_fd;
  struct sockaddr_in serv_addr;
  char buffer[size] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    exit(EXIT_FAILURE);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PROP_PORT);

  if (inet_pton(AF_INET, Other_serverIP, &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    exit(EXIT_FAILURE);
  }

retry:
  if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
  {
    printf("\nConnection failed...reconnecting \n");
    goto retry;
  }
  else
    printf("connected\n");

  while (count < 5)
    ;
  acceptConn = 0;
  send(sock, "connect", strlen("connect"), 0);
  printf("sending connect to B\n");
  valread = read(sock, buffer, size);
  if (strcmp(buffer, "ack") == 0)
  {
    printf("received %s from B\n", buffer);
    while (1)
    {
      memset(buffer, 0x00, strlen(buffer));
      valread = read(sock, buffer, size);
      if (strcmp(buffer, "connect") == 0)
      {
        printf("received %s from B\n", buffer);
        acceptConn = 1;
        printf("sending ack to B\n");
        send(sock, "ack", strlen("ack"), 0);
        while (acceptConn)
          ;
        printf("sending connect to B\n");
        send(sock, "connect", strlen("connect"), 0);

        memset(buffer, 0x00, strlen(buffer));
        valread = read(sock, buffer, size);
        if (strcmp(buffer, "ack") == 0)
          printf("server A not accepting connections\n");
        else
          fprintf(stderr, "received %s instead of ACK\n", buffer);
      }
      else
        fprintf(stderr, "received %s instead of connect\n", buffer);
    }
  }
  else
    fprintf(stderr, "received %s instead of intial ACK\n", buffer);
}

void *serverB()
{
  printf("server B initializing....\n");
  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[size] = {0};

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
  {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(SERVER_PROP_PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
  {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  valread = read(new_socket, buffer, size);

  if (strcmp(buffer, "connect") == 0)
  {
    memset(buffer, 0x00, strlen(buffer));
    send(new_socket, "ack", strlen("ack"), 0);
    acceptConn = 1;
    while (count < 5)
      ;

    acceptConn = 0;
    printf("sending connect to A\n", buffer);
    send(new_socket, "connect", strlen("connect"), 0);
    valread = read(new_socket, buffer, size);
    if (strcmp(buffer, "ack") == 0)
    {

      while (1)
      {
        memset(buffer, 0x00, strlen(buffer));
        valread = read(new_socket, buffer, size);
        if (strcmp(buffer, "connect") == 0)
        {
          acceptConn = 1;
          send(new_socket, "ack", strlen("ack"), 0);
          while (acceptConn)
            ;
          send(new_socket, "connect", strlen("connect"), 0);
          memset(buffer, 0x00, strlen(buffer));
          valread = read(new_socket, buffer, size);
          if (strcmp(buffer, "ack") == 0)
            printf("server B not accepting connections\n");
          else
            fprintf(stderr, "received %s instead of ACK\n", buffer);
        }
        else
          fprintf(stderr, "received %s instead of connect\n", buffer);
      }
    }
  }
  else
    fprintf(stderr, "received %s instead of initial connect\n", buffer);

  close(new_socket);
  shutdown(server_fd, SHUT_RDWR);
}

int incPort()
{
  char temp[4];
  int curPort = atoi(clientPortCount);
  currentPort = curPort;
  curPort++;
  sprintf(temp, "%d", curPort);
  strcpy(clientPortCount, temp);
  return 0;
}

void serviceClient()
{

  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1, count;
  int addrlen = sizeof(address);
  char buffer[size] = {0}, *dir, *temp; 

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
  {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(currentPort);

  // Forcefully attaching socket to the port 8080
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  printf("server listening on %d\n", currentPort);

  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
  {
    perror("accept");
    exit(EXIT_FAILURE);
  }
  printf("server accepted on %d\n", currentPort);

  // dup2(new_socket,stdin);

  while (1)
  {
    // printf("awaiting read\n");
    valread = read(new_socket, buffer, size);
    // printf("cmd is %s\n", buffer);
    if (strcmp(buffer, "quit\n") == 0)
    {
      printf("exiting...\n");
      break;
    }
    else if (buffer[0] == 'c' && buffer[1] == 'd')
    {
      count = 0;
      temp = malloc(strlen(buffer));
      strcpy(temp,buffer);
      temp = strtok(temp, " ");
      // printf("temp is %s\n",temp);
      while (temp != NULL)
      {
        // printf("%s\n", temp);
        dir = temp;
        temp = strtok(NULL, " ");
        count++;
      }

      if (count < 3)
      {
        if (chdir(strtok(dir, "\n")) != 0)
          perror("chdir() to dir failed\n");
      }
      send(new_socket, "\n$", strlen("\n$"), 0);
      free(temp); 
      memset(dir, 0x00, strlen(dir));
      memset(buffer, 0x00, strlen(buffer));
    }
    else
    {
      int status, fd[2];
      pipe(fd);
      pid_t pid = fork();
      if (pid == 0)
      {
        dup2(fd[1], STDOUT_FILENO);
        dup2(fd[1], STDERR_FILENO);
        // close(fd[1]);
        system(buffer);
        write(fd[1], "\n$", strlen("\n$"));
        exit(0);
      }
      else
      {
        memset(buffer, 0x00, strlen(buffer));
        // printf("waiting child death\n");
        wait(&status);
        // printf("child dead\n");
        read(fd[0], buffer, size);
        // printf("read %s\n", buffer);
        send(new_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0x00, strlen(buffer));
      }
    }
    // char *command = malloc(size);
    //   dup2(2, new_socket);
    //   printf("%s\n",command);
    //   free(command);
  }

  // closing the connected socket
  close(new_socket);
  // closing the listening socket
  shutdown(server_fd, SHUT_RDWR);
}

void connectServer()
{
  while (1)
  {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[size] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror("socket failed");
      exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    if (serverName == 'A')
      address.sin_port = htons(PORT_A);
    else
      address.sin_port = htons(PORT_B);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
      perror("bind failed");
      exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
      perror("listen");
      exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    valread = read(new_socket, buffer, size);
    // printf("%s\n", buffer);
    if (strcmp(buffer, "init") == 0)
    {
      if (acceptConn)
      {
        if (incPort() == 0)
        {
          pid_t pid = fork();
          if (pid == 0)
          {
            send(new_socket, clientPortCount, strlen(clientPortCount), 0);
            close(new_socket);
            shutdown(server_fd, SHUT_RDWR);
            serviceClient();
            break;
          }
          else if (pid > 0)
          {
            if (count < 5)
            {
              printf("Count is %d\n", count);
              count++;
            }
            else
            {
              printf("acceptConn is %d\n", 0);
              acceptConn = 0;
            }
          }
          else
            send(new_socket, "server_fork_error", strlen("server_fork_error"), 0);
        }
      }
      else
      {
        send(new_socket, Other_serverIP, strlen(Other_serverIP), 0);
      }

      // if (incPort() == 0)
      // {
      //   pid_t pid = fork();
      //   if (pid == 0)
      //   {
      //     send(new_socket, clientPortCount, strlen(clientPortCount), 0);
      //     close(new_socket);
      //     shutdown(server_fd, SHUT_RDWR);
      //     serviceClient();
      //     break;
      //   }
      //   else if (pid > 0)
      //   {
      //     count++;
      //   }
      //   else
      //     send(new_socket, "server_fork_error", strlen("server_fork_error"), 0);
      // }
    }
    else
      send(new_socket, "invalid_command", strlen("invalid command"), 0);

    close(new_socket);
    shutdown(server_fd, SHUT_RDWR);
  }
}

int main(int argc, char const *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "3 args required [progname] [IP of other server/L [localhost]] [Current server A/B]");
    return 1;
  }
  else
  {
    pthread_t tid;

    if (strcmp(argv[1], "l") == 0 || strcmp(argv[1], "L") == 0)
      strcpy(Other_serverIP, "127.0.0.1");
    else
      strcpy(Other_serverIP, argv[1]);

    serverName = argv[2][0];
    clientPortCount = malloc(sizeof(char) * 4);
    if (serverName == 'A')
    {
      strcpy(clientPortCount, CLI_START_PORT_A);
      pthread_create(&tid, NULL, serverA, NULL);
    }
    else if (serverName == 'B')
    {
      strcpy(clientPortCount, CLI_START_PORT_B);
      pthread_create(&tid, NULL, serverB, NULL);
    }
    else
    {
      fprintf(stderr, "Unknown server name\n");
      return 2;
    }

    connectServer();
  }

  return 0;
}
