#include "HTTPServer.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "HTTPHeader.h"
#include "WorkQueue.h"

HTTPServer::HTTPServer(int port) : port_(port) {
  // Create a socket, port 8080
  sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd_ < 0) {
    std::cerr << "Error opening socket" << std::endl;
  }
  std::cout << "Socket opened" << std::endl;

  // Prepare the sockaddr_in structure
  std::memset(&serv_addr_, 0, sizeof(struct sockaddr_in));
  serv_addr_.sin_family = AF_INET;
  serv_addr_.sin_addr.s_addr = INADDR_ANY;
  serv_addr_.sin_port = htons(port_);

  // Allows successive reconnections to socket without timeout
  int optval = 1;
  setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  // Bind the socket to the current IP address on the port
  if (bind(sockfd_, (struct sockaddr *)&serv_addr_, sizeof(serv_addr_)) < 0) {
    std::cerr << "Error on binding." << std::endl;
  }
  std::cout << "Socket bound" << std::endl;

  // Listen for incoming connections
  listen(sockfd_, 5);
  std::cout << "Socket listening..." << std::endl;
}

HTTPServer::~HTTPServer() { close(sockfd_); }

void HTTPServer::SendContents(int sockfd, const char *filename) {
  char message[200];
  FILE *fp;
  struct stat filestat;
  int status;
  if ((status = stat(filename, &filestat)) != 0) {
    snprintf(message, sizeof(message),
             "HTTP/1.1 404 Not Found\r\n\r\nFile cannot be opened.\r\n");
    write(sockfd, message, strlen(message) + 1);
    return;
  }

  if ((fp = fopen(filename, "r")) == nullptr) {
    snprintf(message, sizeof(message),
             "HTTP/1.1 404 Not Found\r\n\r\nFile cannot be opened.\r\n");
    write(sockfd, message, strlen(message) + 1);
  } else {
    snprintf(message, sizeof(message),
             "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n",
             filestat.st_size);
    write(sockfd, message, strlen(message) + 1);
    char buffer[200];
    while (!feof(fp)) {
      ssize_t n = fread(buffer, 1, sizeof(buffer), fp);
      snprintf(message, n + 1, "%s", buffer);
      ssize_t a = write(sockfd, message, n);
      ssize_t diff = n - a;
      while (diff > 0) {
        a = write(sockfd, message + a, diff);
        diff -= a;
      }
    }
    fclose(fp);
  }
}

void HTTPServer::DoTask(std::string args, int fd) {
  char *comm[4];

  char temp[256];
  snprintf(temp, sizeof(temp), "/tmp/fooXXXXXX");
  args.append("> " + std::string(temp));

  std::string path{"/bin/bash"};
  std::string c{"-c"};

  comm[0] = (char *)path.c_str();
  comm[1] = (char *)c.c_str();
  comm[2] = (char *)args.c_str();
  comm[3] = NULL;

  pid_t pid = fork();
  if (pid == 0) {
    execv(comm[0], comm);
    exit(1);
  }
  int status;
  waitpid(pid, &status, 0);
  if (WIFEXITED(status)) {
    int exit_status = WEXITSTATUS(status);
    if (exit_status == 0) {
      HTTPServer::SendContents(fd, temp);
      return;
    }
  }
  HTTPServer::SendContents(fd, "/filedoesntexist");
}

void HTTPServer::Run() {
  int client_sock;
  struct sockaddr_in cli_addr;
  char buffer[256];

  // The data necessary for the threads
  auto work_queue = new WorkQueue;

  // The pthread pool
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; i++) {
    threads.push_back(std::thread([work_queue]() {
      while (true) {
        std::unique_ptr<HTTPHeader> request = work_queue->Get();

        if (request->GetMethod() == HTTPMethod::kGet) {
          HTTPServer::SendContents(request->GetClientSock(),
                                   request->GetArgs().c_str());
        } else if (request->GetMethod() == HTTPMethod::kPost) {
          HTTPServer::DoTask(request->GetArgs(), request->GetClientSock());
        }

        close(request->GetClientSock());
      }
    }));
  }

  while (true) {
    // Accept any incoming connections, this is blocking
    socklen_t clilen = sizeof(cli_addr);
    client_sock = accept(sockfd_, (struct sockaddr *)&cli_addr, &clilen);
    if (client_sock < 0) {
      std::cerr << "Error on accepting." << std::endl;
    }

    ssize_t n = read(client_sock, buffer, 255);
    if (n < 0) {
      std::cerr << "Error reading from socket." << std::endl;
    }

    // Create the client's request
    auto request = std::make_unique<HTTPHeader>(buffer, n, client_sock);

    work_queue->Add(std::move(request));
  }
}
