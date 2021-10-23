#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef char Element;
/*****************队列*******************/
typedef struct node{
	Element data[640*480*2];
	struct node *next;
}Node;

typedef struct queue{
	Node *head;
	Node *rear;
	int len;
}Queue;

/***************************************************************
		Initialize a queue
		@parameter:	none
		@return : success return head of queue
				：failed return NULL
***************************************************************/
Queue *Ini_queue(void);

/***************************************************************
		Determines if the queue is empty
		@parameter:	
			@queue: queue head
		@return : if empty return 1
				：if unempty return 0
***************************************************************/
int queue_is_empty(Queue *queue);

/***************************************************************
		push queue
		@parameter:	
			@queue: queue head
			@num: queue data
		@return : success return 0
				：failed return -1
***************************************************************/
int push_queue(Queue *queue, Element *num);

/***************************************************************
		pop queue
		@parameter:	
			@queue: queue head
			@num: queue data
		@return : success return 0
				：failed return -1
***************************************************************/
int pop_queue(Queue *queue, Element *data);

/***************************************************************
		clean queue
		@parameter:	
			@queue: queue head
		@return : none
***************************************************************/
void clear_queue(Queue *queue);

/***************************************************************
		destory queue
		@parameter:	
			@queue: queue head
		@return : none
***************************************************************/
void destory_queue(Queue *queue);

#endif