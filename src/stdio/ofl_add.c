#include "stdio_impl.h"

// 打开过的文件记录在这里
FILE *__ofl_add(FILE *f)
{
	FILE **head = __ofl_lock();
	f->next = *head;
	if (*head) (*head)->prev = f;
	*head = f;
	__ofl_unlock();
	return f;
}
