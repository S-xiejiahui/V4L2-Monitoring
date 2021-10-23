#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "queue.h"

/***************************************************************
		Initialize a queue
		@parameter:	none
		@return : success return head of queue
				：failed return NULL
***************************************************************/
Queue *Ini_queue(void)
{
	Queue *queue = (Queue *)malloc(sizeof(Queue));//malloc 分配空间
	if(queue == NULL)
	{
		perror("malloc error");
		return NULL;
	}
	queue->head = queue->rear = NULL;
	queue->len = 0;
	
	return queue;
}

/***************************************************************
		Determines if the queue is empty
		@parameter:	
			@queue: queue head
		@return : if empty return 1
				：if unempty return 0
***************************************************************/
int queue_is_empty(Queue *queue)
{
	if(queue->len == 0)
		return 1;
	return 0;
}

/***************************************************************
		push queue
		@parameter:	
			@queue: queue head
			@num: queue data
		@return : success return 0
				：failed return -1
***************************************************************/
int push_queue(Queue *queue, Element *num)
{
	if(queue == NULL)
	{
		perror("queue is NULL:");
		return -1;
	}
	Node *p = (Node *)malloc(sizeof(Node));
	if(p == NULL)
	{
		perror("malloc error");
		return -1;
	}

	memcpy(p->data,num,480*640*2);
	p->next = NULL;
	
	if(queue->head == NULL)
	{
		queue->head = queue->rear = p;
		queue->len++;
	}
	else
	{
		queue->rear->next = p;
		queue->rear = p;
		queue->len++;
	}
	
	return 0;
}

/***************************************************************
		pop queue
		@parameter:	
			@queue: queue head
			@num: queue data
		@return : success return 0
				：failed return -1
***************************************************************/
int pop_queue(Queue *queue, Element *data)
{
	if(NULL == queue)
	{
		return -1;
	}
	Node *r = queue->head;
	memcpy(data,queue->head->data,480*640*2);
	queue->head = queue->head->next;
	r->next = NULL;
	free(r);//释放空间
	r = NULL;
	queue->len--;
	
	return 0;
}

/***************************************************************
		clean queue
		@parameter:	
			@queue: queue head
		@return : none
***************************************************************/
void clear_queue(Queue *queue)
{
	if(NULL == queue)
	{
		return;
	}

	Element data;
	while(!queue_is_empty(queue))
	{
		pop_queue(queue, &data);
	}
	printf("clean success!!\n");
}

/***************************************************************
		destory queue
		@parameter:	
			@queue: queue head
		@return : none
***************************************************************/
void destory_queue(Queue *queue)
{
	if(NULL == queue)
	{
		return;
	}
	clear_queue(queue);
	free(queue);
	queue = NULL;
	printf("销毁成功\n");
}

