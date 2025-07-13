#include "stdio_impl.h"
#include <sys/uio.h>

// 先写缓冲区, 再写 buf
// 返回成功写入 buf 的大小
size_t __stdio_write(FILE *f, const unsigned char *buf, size_t len)
{
	struct iovec iovs[2] = {
		{ .iov_base = f->wbase, .iov_len = f->wpos-f->wbase },
		{ .iov_base = (void *)buf, .iov_len = len }
	};
	struct iovec *iov = iovs;
	size_t rem = iov[0].iov_len + iov[1].iov_len;
	int iovcnt = 2;
	ssize_t cnt;
	for (;;) {
		cnt = syscall(SYS_writev, f->fd, iov, iovcnt);
		// 写完了
		if (cnt == rem) {
			f->wend = f->buf + f->buf_size;
			f->wpos = f->wbase = f->buf;
			return len;
		}
		// 错误
		if (cnt < 0) {
			f->wpos = f->wbase = f->wend = 0;
			f->flags |= F_ERR;
			return iovcnt == 2
				? 0 // iovs[1] 没有用到
				: len-iov[0].iov_len;
		}
		// 计算剩下的
		rem -= cnt;
		// 第一个写完了, 第二个还有数据
		if (cnt > iov[0].iov_len) {
			// 计算 iovs[1] 写了多少字节
			cnt -= iov[0].iov_len;
			// 现在写 iovs[1]
			iov++; iovcnt--;
		}
		// 更新
		iov[0].iov_base = (char *)iov[0].iov_base + cnt;
		iov[0].iov_len -= cnt;
	}
}
