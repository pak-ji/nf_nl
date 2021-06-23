#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 9999

int main(void)
{
	int sock, n;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t len;
	char msg[1024];

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);
	
	bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	while(1){
		len = sizeof(cli_addr);
		
		n = recvfrom(sock, msg, 1024, 0, (struct sockaddr*)&cli_addr, &len);
		printf("[server] : %s\n", msg);

		msg[n] = '\0';
		sendto(sock, msg, n, 0, (struct sockaddr*)&cli_addr, sizeof(cli_addr));
		memset(&msg, 0, sizeof(msg));
	}

	close(sock);

	return 0;
}
