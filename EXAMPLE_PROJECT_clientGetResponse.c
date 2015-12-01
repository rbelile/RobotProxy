#include "client.h"
#include "methods.h"

void getResponse(int sock) {
      struct sockaddr_in recAddress;
      struct sockaddr_in clientAddress;
      char buffer[MAX];

      socklen_t clientAddressLen = sizeof(clientAddress);

      if (recvfrom(sock, buffer, MAX, 0, (struct sockaddr *) &clientAddress, &clientAddressLen) < 0) {
            err_n_die("recvfrom() failed");
      }
      processResponse(buffer);
}
