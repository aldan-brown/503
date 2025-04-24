#include "stdafx.h"
#include "Stack.h"


Stack::Stack()
{
	head = NULL;
}

bool Stack::push(int val)
{
	Node *insNode = new Node();
	insNode->value = val;
	insNode->next = NULL;
	insNode->next = head;
	head = insNode;
	return true;
}

bool Stack::pop(int &val)
{
	if (head != NULL)
	{
		val = head->value;
		Node* temp = head;
		head = head->next;
		delete temp;
		return true;
	}
	else
	{
		return false;
	}
}

Stack::~Stack()
{
	int val;
	while (pop(val));
}
