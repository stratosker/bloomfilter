#ifndef LIST_H_
#define LIST_H_

struct ListNode{
	void *data;
    struct ListNode *next;
};


void listInsertBeginning(struct ListNode **head, char* value);
int delete_head(struct ListNode **head);
int is_empty(struct ListNode *head);

#endif
