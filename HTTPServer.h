#ifndef POSIX_STUFF_HTTPSERVER_HTTPSERVER_H_
#define POSIX_STUFF_HTTPSERVER_HTTPSERVER_H_

#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

class HTTPServer {
 public:
  HTTPServer(int port);
  ~HTTPServer();

  void Run();
  static void SendContents(int sockfd, const char* filename);
  static void DoTask(std::string args, int fd);

 private:
  int sockfd_;
  int port_;
  struct sockaddr_in serv_addr_;
};

#endif  // POSIX_STUFF_HTTPSERVER_HTTPSERVER_H_
