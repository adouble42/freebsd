#include <sys/types.h>
#include <sys/systm.h>

#include <machine/vmm_snapshot.h>

void
vm_snapshot_buf_err(const char *bufname, const enum vm_snapshot_op op)
{
	const char *__op;

	if (op == VM_SNAPSHOT_SAVE)
		__op = "save";
	else if (op == VM_SNAPSHOT_RESTORE)
		__op = "restore";
	else
		__op = "unknown";

	printf("%s: snapshot-%s failed for %s\r\n", __func__, __op, bufname);
}

int
vm_snapshot_buf(volatile void *data, size_t data_size,
	     struct vm_snapshot_meta *meta)
{
	struct vm_snapshot_buffer *buffer;
	int op;
	void *_data = *(void **)(void *)&data;

	buffer = &meta->buffer;
	op = meta->op;

	if (buffer->buf_rem < data_size) {
		printf("%s: buffer too small\r\n", __func__);
		return (E2BIG);
	}

	if (op == VM_SNAPSHOT_SAVE)
		copyout(_data, buffer->buf, data_size);
	else if (op == VM_SNAPSHOT_RESTORE)
		copyin(buffer->buf, _data, data_size);
	else
		return (EINVAL);

	buffer->buf += data_size;
	buffer->buf_rem -= data_size;

	return (0);
}

size_t
vm_get_snapshot_size(struct vm_snapshot_meta *meta)
{
	size_t length;
	struct vm_snapshot_buffer *buffer;

	buffer = &meta->buffer;

	if (buffer->buf_size < buffer->buf_rem) {
		printf("%s: Invalid buffer: size = %zu, rem = %zu\r\n",
		       __func__, buffer->buf_size, buffer->buf_rem);
		length = 0;
	} else {
		length = buffer->buf_size - buffer->buf_rem;
	}

	return (length);
}

int
vm_snapshot_gaddr(void **addr_var, size_t gaddr_len, int restore_null,
		  struct vm_snapshot_meta *meta)
{
	/* The kernel devices/structures should not map guest memory */
	return (0);
}

int
vm_snapshot_buf_cmp(volatile void *data, size_t data_size,
		    struct vm_snapshot_meta *meta)
{
	struct vm_snapshot_buffer *buffer;
	int op;
	int ret;
	void *_data = *(void **)(void *)&data;

	buffer = &meta->buffer;
	op = meta->op;

	if (buffer->buf_rem < data_size) {
		printf("%s: buffer too small\r\n", __func__);
		ret = E2BIG;
		goto done;
	}

	if (op == VM_SNAPSHOT_SAVE) {
		ret = 0;
		copyout(_data, buffer->buf, data_size);
	} else if (op == VM_SNAPSHOT_RESTORE) {
		ret = memcmp(_data, buffer->buf, data_size);
	} else {
		ret = EINVAL;
		goto done;
	}

	buffer->buf += data_size;
	buffer->buf_rem -= data_size;

done:
	return (ret);
}
