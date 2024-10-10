#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

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

    // Safely get the path from the request
    char buffer[1024] = {0};
    int valread = read(new_socket, buffer, 1023);
    if (valread <= 0) continue;

    buffer[valread] = '\0';

    // Find the end of the first line
    char *request_line_end = strstr(buffer, "\r\n");
    if (!request_line_end) continue;

    *request_line_end = '\0';

    char method[16], path[256], version[16];
    int ret = sscanf(buffer, "%15s /%255s %15s", method, path, version);
    if (ret != 3) continue;

    // Construct the response
    char body[512];
    snprintf(body, sizeof(body), "<h1>Hello, %s!</h1>", path);

    int body_length = strlen(body);

    char response[1024];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Length: %d\r\n"
             "Content-Type: text/html\r\n"
             "\r\n"
             "%s",
             body_length, body);

    write(new_socket, response, strlen(response));
    close(new_socket);
  }

  close(server_fd);
  return 0;
}
