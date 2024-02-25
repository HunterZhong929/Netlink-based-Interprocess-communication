#include <linux/module.h>
#include <net/sock.h> 
#include <linux/netlink.h>
#include <linux/skbuff.h> 
#include <linux/string.h>
#include <linux/list.h>
#include <linux/slab.h>
#define NETLINK_USER 31


#define SUB_REG_FLAG 'S'
#define PUB_REG_FLAG 'P'


struct sock *nl_sk = NULL;
int pid;
int pub_pid = 0;

struct linkedList {
	int pid;
	struct list_head list;
};

LIST_HEAD(sub_list);




static void hello_nl_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh; //netlink message header
	struct sk_buff *skb_out; //this is the packet we are sending
	int msg_size; //the lenth of the packet data
	char *msg = "Hello from kernel";
	int res;
	int msg_type = 0;

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    	
	//the first message determines if its a publisher or subscriber

	//if the sender pid is not in sub list of pub list, we check the first byte of the message to determine
	



    nlh = (struct nlmsghdr *)skb->data; //get our header from the input
    printk(KERN_INFO "Netlink received msg payload:%s\n", (char *)nlmsg_data(nlh));
	
	msg =nlmsg_data(nlh);
	msg_size = strlen(msg);
	pid = nlh->nlmsg_pid; /*pid of publishing process */
	if(msg[0]==SUB_REG_FLAG){
		msg_type = 1;
	}
	else if(msg[0]== PUB_REG_FLAG){
		msg_type = 2;
	}
	else {
		msg_type = 3;
	}


	if(msg[0]== SUB_REG_FLAG){
		//case when a process is registering as Subscriber
		//add it to the linked list

		struct linkedList *sub_node;
		sub_node = kmalloc(sizeof(struct linkedList), GFP_KERNEL);
		sub_node->pid=pid;
		struct linkedList *ptr = NULL;
		//INIT_LIST_HEAD(sub_node.list);
		
		

		list_add_tail(&sub_node->list, &sub_list);

		printk("new entry in sublist updated:\n");
		list_for_each_entry(ptr, &sub_list, list) {
		printk(" My List - PID: %d", ptr->pid);
	}

	}

	if(msg[0]== PUB_REG_FLAG){
		//case when a registering message is a Publisher
		//for part2 we are supporting one publisher only
		pub_pid = pid;
	}
	//printk("msg = %s", msg);
	//checking the msg to determine if this is a register messag

	
	

	
	skb_out = nlmsg_new(msg_size, 0);





	if (!skb_out) {
		printk(KERN_ERR "Failed to allocate new skb\n");
	      	return;
	}

	
	// printk(KERN_INFO "netlink_skb_parms Information:\n");
    // printk(KERN_INFO "  portid: %u\n", NETLINK_CB(skb).portid);
    // printk(KERN_INFO "  dst_group: %u\n", NETLINK_CB(skb).dst_group);
    // printk(KERN_INFO "  flags: %u\n", NETLINK_CB(skb).flags);
    // //printk(KERN_INFO "  seq: %u\n", NETLINK_CB(skb_out).seq);
    // printk(KERN_INFO "  pid: %u\n", NETLINK_CB(skb).nsid);
    // //printk(KERN_INFO "  protocol: %d\n", NETLINK_CB(skb_out).protocol);
    // printk(KERN_INFO "End of netlink_skb_parms Information\n");
	
	 /* not in mcast group */
	

	if(msg_type == 3){
		nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0); 
		NETLINK_CB(skb_out).dst_group = 0;
		strncpy(nlmsg_data(nlh), msg, msg_size);
		//case when a publisher is trying to broad cast the message to subs
		struct linkedList *ptr = NULL;
		list_for_each_entry(ptr, &sub_list, list) {
			skb_out = nlmsg_new(msg_size, 0);
			if (!skb_out) {
			printk(KERN_ERR "Failed to allocate new skb\n");
	      		return;
			}
			nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0); 
			NETLINK_CB(skb_out).dst_group = 0;
			strncpy(nlmsg_data(nlh), msg, msg_size);
			res = nlmsg_unicast(nl_sk, skb_out,ptr->pid);
			if (res < 0)
			printk(KERN_INFO "Error while sending bak to user\n");
	}
	}
	
	else{
		printk("the msg type is register\n");
		char * reg_msg = "Succesfully registered\n";
		msg_size = strlen(reg_msg);
		nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0); 
		strncpy(nlmsg_data(nlh), reg_msg, msg_size);
		
		res = nlmsg_unicast(nl_sk, skb_out, pid);
		if (res < 0)
			printk(KERN_INFO "Error while sending bak to user\n");
		}
}

static int __init hello_init(void)
{

    printk("Entering: %s\n", __FUNCTION__);
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg); // returns a sock struct
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void)
{

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(hello_init); module_exit(hello_exit);

MODULE_LICENSE("GPL");
