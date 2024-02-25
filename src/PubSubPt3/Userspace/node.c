#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/netlink.h>
#include <string.h>
#include <sys/socket.h>

#include <pthread.h>

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024 /* maximum payload size */

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct nlmsghdr *nlh_p = NULL;
struct nlmsghdr *nlh_s = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;
struct msghdr sub_msg;
struct msghdr pub_msg;
pthread_mutex_t lock;
//pthread_cond_t cond;
pthread_t tid[2];
//int new_input = 0;//flag for when receiving message from kernel

void *publisher(void *vargp)
{   
    // nlh_p = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    // memset(nlh_p, 0, NLMSG_SPACE(MAX_PAYLOAD));
    // nlh_p->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    // nlh_p->nlmsg_pid = getpid(); 
    // nlh_p->nlmsg_flags = 0;

    for(;;){
        
        //printf("publisher thread accquired lock\n");
        char *userInput = malloc(sizeof(char) * MAX_PAYLOAD); 
        
        memset(userInput, '\0', sizeof(char) * MAX_PAYLOAD);
        printf("Enter message to send to kernel (or 'exit' to quit): ");
        fgets(userInput, MAX_PAYLOAD, stdin);
        userInput[strcspn(userInput, "\n")] = '\0';  // Remove trailing newline

        if (strcmp(userInput, "exit") == 0) {
            // If the user enters 'exit', break out of the loop
            printf("Exiting...\n");
            break;
        }
        



        strcpy(NLMSG_DATA(nlh), userInput);
        nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
        iov.iov_base = (void *)nlh;
        iov.iov_len = nlh->nlmsg_len;
        pub_msg.msg_name = (void *)&dest_addr;
        pub_msg.msg_namelen = sizeof(dest_addr);
        pub_msg.msg_iov = &iov;
        pub_msg.msg_iovlen = 1;
       
        

        // Send the user input message to the kernel
        printf("Sending message to kernel: %s\n", (char *)NLMSG_DATA(nlh));


        pthread_mutex_lock(&lock);
        sendmsg(sock_fd, &pub_msg, 0);
        //new_input = 1;
        //pthread_cond_signal(&cond);
        free(userInput);
        pthread_mutex_unlock(&lock);

        //printf("publisher thread release lock\n");
        usleep(100000);
    }

    // Cleanup and close the socket
    free(nlh);
    close(sock_fd);
	
}

void *subsciber(void *vargp)
{   
    // nlh_s = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    // memset(nlh_s, 0, NLMSG_SPACE(MAX_PAYLOAD));
    // nlh_s->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    // nlh_s->nlmsg_pid = getpid(); 
    // nlh_s->nlmsg_flags = 0;
    int rcv_len;
    //printf("Entered subscriber thread\n");
    /* Read message from kernel */
    for(;;){
        
        //printf("subscriber thread accquire lock\n");


        rcv_len = recvmsg(sock_fd, &sub_msg, 0);
        printf("received len = %d",rcv_len);
        pthread_mutex_lock(&lock);
        printf("Received message payload: %s\n", NLMSG_DATA(nlh));
        memset(NLMSG_DATA(nlh), '\0', sizeof(NLMSG_DATA(nlh)));
        pthread_mutex_unlock(&lock);
        //printf("subscriber thread release lock\n");
        usleep(100000);
    }
    close(sock_fd);
	
}

int main()
{   sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
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
    

    //--------USER input to determine whether this process is a publisher/subscriber/or both
    char processType;
    printf("Enter 'P' for Publisher, 'S' for Subscriber, or 'B' for Both: ");
    scanf("%c", &processType);
    getchar();//remove the leftover newline
    // Create the list
    pthread_mutex_init(&lock, NULL);

    // 1) create the threads based on user input
    if (processType == 'P' || processType == 'B') {
        

        strcpy(NLMSG_DATA(nlh), "PUBTEST PART 2");//these message doesn't really matter as long as it start with 'P' and len > 0

        iov.iov_base = (void *)nlh;
        iov.iov_len = nlh->nlmsg_len;
        msg.msg_name = (void *)&dest_addr;
        msg.msg_namelen = sizeof(dest_addr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        //init pub_msg
        pub_msg.msg_name = (void *)&dest_addr;
        pub_msg.msg_namelen = sizeof(dest_addr);
        pub_msg.msg_iov = &iov;
        pub_msg.msg_iovlen = 1;

        printf("registering as a publisher\n");
        sendmsg(sock_fd, &msg, 0);
        //recvmsg(sock_fd, &msg, 0);
        //printf("registering result: %s\n", NLMSG_DATA(nlh));




        pthread_create(&tid[0], NULL, &publisher, NULL);
    }

    if (processType == 'S' || processType == 'B') {
        strcpy(NLMSG_DATA(nlh), "SUBTEST DATA 1");

        iov.iov_base = (void *)nlh;
        iov.iov_len = nlh->nlmsg_len;
        msg.msg_name = (void *)&dest_addr;
        msg.msg_namelen = sizeof(dest_addr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        printf("registering as a subscriber\n");
        sub_msg.msg_name = (void *)&dest_addr;
        sub_msg.msg_namelen = sizeof(dest_addr);
        sub_msg.msg_iov = &iov;
        sub_msg.msg_iovlen = 1;
        sendmsg(sock_fd, &msg, 0);
        //recvmsg(sock_fd, &msg, 0);
        //printf("registering result: %s\n", NLMSG_DATA(nlh));

        pthread_create(&tid[1], NULL, &subsciber, NULL);
    }

    // Wait for the threads to finish

    





    if (processType == 'P' ) {
        pthread_join(tid[0], NULL);
    }

    if (processType == 'S' ) {
        pthread_join(tid[1], NULL);
    }

    if (processType == 'B' ) {
        pthread_join(tid[0], NULL);
        pthread_join(tid[1], NULL);
    }

	
	pthread_mutex_destroy(&lock);

	exit(0);
}


