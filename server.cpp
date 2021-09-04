#include <signal.h>
#include <stdlib.h>

#include <iostream>

#include "HTTPServer.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Error: no port provided" << std::endl;
    return 1;
  }

  sigignore(SIGPIPE);

  HTTPServer server(atoi(argv[1]));

  server.Run();

  return 0;
}
