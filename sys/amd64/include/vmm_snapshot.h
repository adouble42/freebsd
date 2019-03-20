#ifndef _VMM_SNAPSHOT_
#define _VMM_SNAPSHOT_

#include <sys/errno.h>
#include <sys/types.h>

struct vmctx;

enum snapshot_req {
	STRUCT_VMX,
	STRUCT_VIOAPIC,
	STRUCT_VM,
	STRUCT_VLAPIC,
	STRUCT_LAPIC,
	VM_MEM,
	STRUCT_VHPET,
	STRUCT_VMCX,
	STRUCT_VATPIC,
	STRUCT_VATPIT,
	STRUCT_VPMTMR,
	STRUCT_VRTC,
};

struct vm_snapshot_buffer {
	/*
	 * R/O for device-specific functions;
	 * written by generic snapshot functions.
	 */
	uint8_t *const buf_start;
	const size_t buf_size;

	/*
	 * R/W for device-specific functions used to keep track of buffer
	 * current position and remaining size.
	 */
	uint8_t *buf;
	size_t buf_rem;

	/*
	 * Length of the snapshot is either determined as (buf_size - buf_rem)
	 * or (buf - buf_start) -- the second variation returns a signed value
	 * so it may not be appropriate.
	 *
	 * Use vm_get_snapshot_size(meta).
	 */
};

enum vm_snapshot_op {
	VM_SNAPSHOT_SAVE,
	VM_SNAPSHOT_RESTORE,
};

struct vm_snapshot_meta {
	struct vmctx *ctx;
	void *dev_data;
	const char *dev_name;      /* identify userspace devices */
	enum snapshot_req dev_req; /* identify kernel structs */

	struct vm_snapshot_buffer buffer;

	enum vm_snapshot_op op;
};


void vm_snapshot_buf_err(const char *bufname, const enum vm_snapshot_op op);
int vm_snapshot_buf(volatile void *data, size_t data_size,
		    struct vm_snapshot_meta *meta);
size_t vm_get_snapshot_size(struct vm_snapshot_meta *meta);
int vm_snapshot_gaddr(void **addr_var, size_t gaddr_len, int restore_null,
		      struct vm_snapshot_meta *meta);
int vm_snapshot_buf_cmp(volatile void *data, size_t data_size,
			      struct vm_snapshot_meta *meta);

#define	SNAPSHOT_BUF_OR_LEAVE(DATA, LEN, META, RES, LABEL)			\
do {										\
	(RES) = vm_snapshot_buf((DATA), (LEN), (META));				\
	if ((RES) != 0) {							\
		vm_snapshot_buf_err(#DATA, (META)->op);				\
		goto LABEL;							\
	}									\
} while (0)

#define	SNAPSHOT_VAR_OR_LEAVE(DATA, META, RES, LABEL)				\
	SNAPSHOT_BUF_OR_LEAVE(&(DATA), sizeof(DATA), (META), (RES), LABEL)

/*
 * Address variables are pointers to guest memory.
 *
 * When RNULL != 0, do not enforce invalid address checks; instead, make the
 * pointer NULL at restore time.
 */
#define	SNAPSHOT_GADDR_OR_LEAVE(ADDR_VAR, GADDR_SIZE, RNULL, META, RES, LABEL)	\
do {										\
	(RES) = vm_snapshot_gaddr((void **)&(ADDR_VAR), (GADDR_SIZE), (RNULL),	\
				  (META));					\
	if ((RES) != 0) {							\
		if ((RES) == EFAULT)						\
			fprintf(stderr, "%s: invalid address: %s\r\n",		\
				__func__, #ADDR_VAR);				\
		goto LABEL;							\
	}									\
} while (0)

/* compare the value in the meta buffer with the data */
#define	SNAPSHOT_BUF_CMP_OR_LEAVE(DATA, LEN, META, RES, LABEL)			\
do {										\
	(RES) = vm_snapshot_buf_cmp((DATA), (LEN), (META));			\
	if ((RES) != 0) {							\
		vm_snapshot_buf_err(#DATA, (META)->op);				\
		goto LABEL;							\
	}									\
} while (0)

#define	SNAPSHOT_VAR_CMP_OR_LEAVE(DATA, META, RES, LABEL)			\
	SNAPSHOT_BUF_CMP_OR_LEAVE(&(DATA), sizeof(DATA), (META), (RES), LABEL)

#endif
