#include "stdio_impl.h"

// 准备写入
// libc 不允许 读写混合, 要求读写切换中间必须 有一个 intervening call to the fflush
// 也就是刷新缓冲区, 因为读写操作共用一个缓冲区, 也是历史问题, 详细文件见 os/2 museum:
//   - https://www.os2museum.com/wp/i-thought-i-found-a-bug
int __towrite(FILE *f)
{
	// 黑魔法, 状态转移:
	// -1 -> -1
	//  0 -> -1
	//  1 ->  1
	f->mode |= f->mode-1;
	if (f->flags & F_NOWR) {
		f->flags |= F_ERR;
		return EOF;
	}
	/* Clear read buffer (easier than summoning nasal demons) */
	// 撤销 读缓冲
	f->rpos = f->rend = 0;

	// 设置 写缓冲
	// f->wend 是为了简洁
	/* Activate write through the buffer. */
	f->wpos = f->wbase = f->buf;
	f->wend = f->buf + f->buf_size;

	return 0;
}

hidden void __towrite_needs_stdio_exit()
{
	__stdio_exit_needed();
}
