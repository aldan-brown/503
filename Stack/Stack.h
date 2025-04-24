#pragma once
 
typedef struct Node
{
	int value;
	Node *next;
};

class Stack
{
public:
	Stack();
	bool push(int val);
	bool pop(int &val);
	~Stack();

private:
	Node *head;
};

