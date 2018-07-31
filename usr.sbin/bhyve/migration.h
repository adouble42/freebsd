#ifndef _BHYVE_MIGRATION_
#define _BHYVE_MIGRATION_

struct vmctx;

int receive_vm_migration(struct vmctx *ctx, char *migration_data);

/* Warm Migration */

enum migration_transfer_req {
	MIGRATION_SEND_REQ	= 0,
	MIGRATION_RECV_REQ	= 1
};

enum message_types {
    MESSAGE_TYPE_SPECS		= 1,
    MESSAGE_TYPE_METADATA	= 2,
    MESSAGE_TYPE_RAM		= 3,
    MESSAGE_TYPE_KERN		= 4,
    MESSAGE_TYPE_PCI		= 5,
    MESSAGE_TYPE_UNKNOWN	= 8,
};

struct __attribute__((packed)) migration_message_type {
    size_t len;
    unsigned int type;		// enum message_type
    unsigned int req_type;	// enum snapshot_req
};

struct __attribute__((packed)) migration_system_specs {
	char hw_machine[MAX_SPEC_LEN];
	char hw_model[MAX_SPEC_LEN];
	size_t hw_pagesize;
};

int vm_send_migrate_req(struct vmctx *ctx, struct migrate_req req);
int vm_recv_migrate_req(struct vmctx *ctx, struct migrate_req req);

#endif
