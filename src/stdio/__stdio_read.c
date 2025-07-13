#include "stdio_impl.h"
#include <sys/uio.h>

size_t __stdio_read(FILE *f, unsigned char *buf, size_t len)
{
	// 预留一个字节:
	// Linux 把每个 iovec 当作独立读取处理, 并且在 tty 模式下, 它有行缓冲行为
	// 但是我不理解: https://git.musl-libc.org/cgit/musl/commit/src/stdio/__stdio_read.c?id=2cff36a84f2
	// 我感觉就是降低阻塞的概率
	struct iovec iov[2] = {
		{ .iov_base = buf, .iov_len = len - !!f->buf_size },
		{ .iov_base = f->buf, .iov_len = f->buf_size }
	};
	ssize_t cnt;

	cnt = iov[0].iov_len ? syscall(SYS_readv, f->fd, iov, 2)
		: syscall(SYS_read, f->fd, iov[1].iov_base, iov[1].iov_len);
	if (cnt <= 0) {
		f->flags |= cnt ? F_ERR : F_EOF;
		return 0;
	}
	if (cnt <= iov[0].iov_len) return cnt;
	cnt -= iov[0].iov_len;
	f->rpos = f->buf;
	f->rend = f->buf + cnt;
	// 这里把之前抠下来的以字节给还回去了
	if (f->buf_size) buf[len-1] = *f->rpos++;
	return len;
}
