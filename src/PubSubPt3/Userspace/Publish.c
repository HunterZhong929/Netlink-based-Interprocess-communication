#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024 /* maximum payload size */

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main() {
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    printf("sock_fd= %d\n", sock_fd);
    if (sock_fd < 0)
        return -1;

    // Initialize the socket
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */
    printf("pub pid= %d\n", getpid());
    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    // Initialize the destination address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    // Allocate memory for the initial message
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid(); // This is the pid of the process you want to send to
    nlh->nlmsg_flags = 0;

    strcpy(NLMSG_DATA(nlh), "PUBTEST PART 2");

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Sending initial message to kernel\n");
    sendmsg(sock_fd, &msg, 0);

    // Enter the while loop to continuously send user input
    while (1) {
        // User input buffer
        char userInput[MAX_PAYLOAD];
        
        // Prompt for user input
        printf("Enter message to send to kernel (or 'exit' to quit): ");
        fgets(userInput, sizeof(userInput), stdin);
        userInput[strcspn(userInput, "\n")] = '\0';  // Remove trailing newline

        if (strcmp(userInput, "exit") == 0) {
            // If the user enters 'exit', break out of the loop
            printf("Exiting...\n");
            break;
        }

        // Update the NLMSG_DATA with user input
        strcpy(NLMSG_DATA(nlh), userInput);

        // Send the user input message to the kernel
        printf("Sending message to kernel: %s\n", userInput);
        sendmsg(sock_fd, &msg, 0);
    }

    // Cleanup and close the socket
    free(nlh);
    close(sock_fd);

    return 0;
}
