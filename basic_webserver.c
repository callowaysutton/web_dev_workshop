#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                    "<html><body><h1>Hello, World!</h1></body></html>";

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) exit(EXIT_FAILURE);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(80);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) exit(EXIT_FAILURE);
  if (listen(server_fd, 3) < 0) exit(EXIT_FAILURE);

  while (1) {
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) exit(EXIT_FAILURE);
    write(new_socket, response, strlen(response));
    close(new_socket);
  }

  close(server_fd);
  return 0;
}


