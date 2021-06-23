#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9999

int main(int argc, char **argv)
{
	int sock, n;
	socklen_t cli_size;
	struct sockaddr_in serv_addr, cli_addr;
	
	char buff[1024];

	if(argc != 3)
	{
		printf("usage : %s [IP Address] [Message]\n", argv[0]);
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(PORT);

	sendto(sock, argv[2], strlen(argv[2]), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	
	memset(&buff, 0, sizeof(buff));
	cli_size = sizeof(cli_addr);
	n = recvfrom(sock, buff, 1024, 0, (struct sockaddr*)&cli_addr, &cli_size);
	printf("[client] : %s\n", buff);

	close(sock);

	return 0;
}
