#include "stdio_impl.h"

int __overflow(FILE *f, int _c)
{
	unsigned char c = _c;
	if (!f->wend && __towrite(f))
		return EOF;
	if (f->wpos != f->wend && c != f->lbf)
		return *f->wpos++ = c;
	// 要不然满了, 要不然换行了
	if (f->write(f, &c, 1)!=1)
		return EOF;
	return c;
}
