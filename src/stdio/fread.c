#include "stdio_impl.h"
#include <string.h>

#define MIN(a,b) ((a)<(b) ? (a) : (b))

size_t fread(void *restrict destv, size_t size, size_t nmemb, FILE *restrict f)
{
	unsigned char *dest = destv;
	size_t len = size*nmemb, l = len, k;
	if (!size) nmemb = 0;

	FLOCK(f);

	f->mode |= f->mode-1;

	if (f->rpos != f->rend) {
		/* First exhaust the buffer. */
		k = MIN(f->rend - f->rpos, l);
		memcpy(dest, f->rpos, k);
		f->rpos += k;
		dest += k;
		l -= k;
	}
	
	/* Read the remainder directly */
	for (; l; l-=k, dest+=k) {
		// __toread 不是会重置缓冲区吗, 为什么还要包含在 for 循环内?
		//   __toread 实际上是会清空缓冲区的. 第一次进入的时候缓冲区已经被读出, 所以充值无所谓.
		//   所以接下来 运行 f->read. 若没有数据可以读, 即返回 0. => 退出
		//   如果只读到了一部分数据, 即 (retval) < l, 而读缓冲区是要等到 dest 填充完成之后再填充的,
		//   所以接下来几次调用 __toread 都没有问题.
		// POSIX 的 read 不保证一次就可以读完所有数据, 所以必须要读到 retval = 0 为止
		k = __toread(f) ? 0 : f->read(f, dest, l);
		if (!k) {
			FUNLOCK(f);
			return (len-l)/size;
		}
	}

	FUNLOCK(f);
	return nmemb;
}

weak_alias(fread, fread_unlocked);
