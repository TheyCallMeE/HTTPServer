#ifndef POSIX_STUFF_HTTPSERVER_HTTPHEADER_H_
#define POSIX_STUFF_HTTPSERVER_HTTPHEADER_H_

#include <stdio.h>

#include <cstring>
#include <map>

enum class HTTPMethod { kGet, kPost, kUnknown };

class HTTPHeader {
 public:
  HTTPHeader(char* buffer, size_t len, int clientsock);
  ~HTTPHeader() = default;

  const std::string& GetArgs() const { return args_; }
  const std::map<std::string, std::string>& GetMime() const { return mime_; }
  HTTPMethod GetMethod() const { return method_; }
  const std::string& GetURI() const { return uri_; }
  int GetClientSock() const { return clientsock_; }
  int FindInt(const std::string& to_find);
  const std::string& FindInMime(const std::string& to_find);

 private:
  int clientsock_;
  HTTPMethod method_ = HTTPMethod::kUnknown;
  std::string uri_;
  std::string args_;
  std::map<std::string, std::string> mime_;
};

#endif  // POSIX_STUFF_HTTPSERVER_HTTPHEADER_H_
