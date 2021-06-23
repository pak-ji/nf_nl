#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define NL_USER 31
#define NL_PAYLOAD_SIZE 1024

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
struct msghdr msg;
int sock_fd;

int main() 
{
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NL_USER);
	if(sock_fd < 0) {
		perror("socket() ");
		return -1;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; // for Kernel
	dest_addr.nl_groups = 0; // unicast

	nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(NL_PAYLOAD_SIZE));
	memset(nlh, 0, NLMSG_SPACE(NL_PAYLOAD_SIZE));
	nlh->nlmsg_len = NLMSG_SPACE(NL_PAYLOAD_SIZE);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), "Hello");

	iov.iov_base = (void*)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void*)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	if( bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) ){
		perror("bind() ");
		close(sock_fd);
		return -1;
	}

	printf("Sending message to kernel\n");
	sendmsg(sock_fd, &msg, 0);

	printf("Waiting for message from kernel\n");
	recvmsg(sock_fd, &msg, 0);

	printf("Received message : %s\n", (char*)NLMSG_DATA(nlh));
	
	close(sock_fd);

	return 0;
}
