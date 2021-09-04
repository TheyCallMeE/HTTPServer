#include "HTTPHeader.h"

#include <unistd.h>

HTTPHeader::HTTPHeader(char* buffer, size_t len, int clientsock)
    : clientsock_(clientsock) {
  char method[5];
  char uri[200];
  sscanf(buffer, "%s %s", method, uri);
  if (strcmp(method, "GET") == 0) {
    method_ = HTTPMethod::kGet;
  } else if (strcmp(method, "POST") == 0) {
    method_ = HTTPMethod::kPost;
  }
  uri_ = uri;

  if (method_ == HTTPMethod::kGet) {
    char pathname[4096];
    getcwd(pathname, sizeof(pathname));
    strcat(pathname, uri);
    args_ = pathname;
  } else if (method_ == HTTPMethod::kPost) {
    std::string comm = (uri + 1);
    args_ = comm + " ";
    // Parse the mime header into a std::map
    // Avoid the first line that has already been processed
    char* n_buff = buffer + len;
    while (buffer != n_buff && *buffer != '\r') {
      buffer++;
    }
    buffer += 2;  // Skips the accompanying \n
    bool processed{false};
    std::string line{};
    while (!processed) {
      if (buffer != n_buff && *buffer == '\r') {
        if (buffer != n_buff && *(buffer + 2) == '\r') {
          processed = true;
          buffer += 2;
        }
        size_t n = line.find(": ");
        if (n == std::string::npos) {
          continue;
        }
        std::string label = line.substr(0, n);
        std::string val = line.substr(n + 2, std::string::npos);
        mime_.insert(std::pair<std::string, std::string>(label, val));
        line.clear();
        buffer += 2;
      } else {
        if (line.empty()) {
          line = *buffer;
        } else {
          line += *buffer;
        }
        buffer++;
      }
    }

    // Gather the arguments
    int num_bytes = FindInt("Content-Length");
    if (num_bytes >= 0) {
      for (int i = 0; i < num_bytes; i++) {
        args_ += *buffer;
        buffer++;
      }
    }
  }
}

int HTTPHeader::FindInt(const std::string& to_find) {
  std::map<std::string, std::string>::iterator it;
  it = mime_.find(to_find);
  if (it == mime_.end()) {
    return -1;
  }
  int n = atoi(it->second.c_str());
  return n;
}

const std::string& HTTPHeader::FindInMime(const std::string& to_find) {
  static std::string empty;
  std::map<std::string, std::string>::iterator it;
  it = mime_.find(to_find);
  if (it == mime_.end()) {
    return empty;
  }
  return it->second;
}
