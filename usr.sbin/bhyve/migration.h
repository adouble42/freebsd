#ifndef _BHYVE_MIGRATION_
#define _BHYVE_MIGRATION_

struct vmctx;

int receive_vm_migration(struct vmctx *ctx, char *migration_data);

#endif
