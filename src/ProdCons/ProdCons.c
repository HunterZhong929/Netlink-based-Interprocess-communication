#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

const int max = 10000000; //number of message to be sent
pthread_mutex_t lock;
int length = 0; //number of entries in the linked list
pthread_t tid[2];
int counter = 0;
struct node* head;
struct node* tail;

struct node
{
	struct node* next;
	int data;
};
void add_node(int data)
{
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    if (new_node == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for a new node\n");
        exit(EXIT_FAILURE);
    }

    new_node->data = data;
    new_node->next = NULL;

    if (tail == NULL)
    {
        // The list is empty
        head = tail = new_node;
    }
    else
    {
        // Add the new node to the end of the list
        tail->next = new_node;
        tail = new_node;
    }
    counter++;
    length++;
    if (counter%1000==0){
	    printf("Produced: %d\n", data);}
}

int remove_head()
{
    if (head == NULL)
    {
        fprintf(stderr, "remove from an empty list\n");
        exit(EXIT_FAILURE);
    }

    int data = head->data;
    struct node *temp = head;

    head = head->next;
    free(temp);

    if (head == NULL)
    {
        // The list is now empty
        tail = NULL;
    }
    counter++;
    length--;
    if(counter%1000==0){
	printf("Consumed: %d\n", data);}

    return data;
}

// (head, 0) -> (1) -> (2) -> (3, tail)
//count(producer) = 3, count(consumer) = 0
// if consumer goes next: (head, 1) -> (2) -> (3, tail)
// if producer goes next: (head, 0) -> (1) -> (2) -> (3) -> (4, tail)
void *consumer(void *vargp)
{
	int count = 0;
    while (count < max)
    {
        pthread_mutex_lock(&lock);
        if (length > 0)
        {
            int data = remove_head();
            if (data != count)
            {
                printf("ERROR! data %d should be %d!\n", data, count);
            }
            count++;
        }
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void *producer(void *vargp)
{
	int count = 0;
    while (count < max)
    {
        pthread_mutex_lock(&lock);
        add_node(count);
        count++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main()
{
	// 1) create the list
	pthread_mutex_init(&lock, NULL);
	pthread_create(&tid[0], NULL, &producer, NULL);
	pthread_create(&tid[1], NULL, &consumer, NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[0], NULL);
	//make the main thread wait until this two thread finish
	if(head != NULL) {printf("ERROR! List not empty\n");}
	pthread_mutex_destroy(&lock);

	exit(0);
}

/*
Useful commands:
pthread_mutex_init(&lock, NULL)
pthread_create(&tid[0], NULL, &producer, NULL);
pthread_join(&tid[1], NULL);
pthread_mutex_lock(&lock);
pthread_mutex_unlock(&lock);
*/
