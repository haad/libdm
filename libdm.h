
#ifndef _LIB_DM_H_
#define _LIB_DM_H_

#include <prop/proplib.h>

#define IOCTL_TYPE_IN  0x1
#define IOCTL_TYPE_OUT 0x2

typedef prop_object_iterator_t libdm_iter_t;

typedef prop_array_t libdm_cmd_t;

typedef prop_dictionary_t libdm_task_t;
typedef prop_dictionary_t libdm_table_t;
typedef prop_dictionary_t libdm_target_t;
typedef prop_dictionary_t libdm_dev_t;

struct cmd_version {
	const char *cmd;
	uint32_t version[3];
};

void libdm_iter_destroy(libdm_iter_t); 

libdm_task_t libdm_task_create(const char *);
void libdm_task_destroy(libdm_task_t);

char *libdm_task_get_command(libdm_task_t);
int32_t libdm_task_get_cmd_version(libdm_task_t, uint32_t *, size_t);

int libdm_task_set_name(const char *, libdm_task_t);
char *libdm_task_get_name(libdm_task_t);

int libdm_task_set_uuid(const char *, libdm_task_t);
char *libdm_task_get_uuid(libdm_task_t);

int libdm_task_set_minor(uint32_t, libdm_task_t);
uint32_t libdm_task_get_minor(libdm_task_t);

int libdm_task_set_flags(uint32_t, libdm_task_t);
uint32_t libdm_task_get_flags(libdm_task_t);

uint32_t libdm_task_get_target_num(libdm_task_t);

int libdm_task_set_cmd(libdm_cmd_t, libdm_task_t);
libdm_cmd_t libdm_task_get_cmd(libdm_task_t);

/* cmd_data dictionary entry manipulating functions */

libdm_cmd_t libdm_cmd_create();
void libdm_cmd_destroy(libdm_cmd_t);
libdm_iter_t libdm_cmd_iter_create(libdm_cmd_t);

int libdm_cmd_set_table(libdm_table_t, libdm_cmd_t);

/* cmd is array of table dictionaries */
libdm_table_t libdm_cmd_get_table(libdm_iter_t);

/* cmd is array of dev dictionaries */
libdm_dev_t libdm_cmd_get_dev(libdm_iter_t);

/* cmd is array of target dictonaries */
libdm_target_t libdm_cmd_get_target(libdm_iter_t);
/* cmd is array of dev_t's */
uint64_t libdm_cmd_get_deps(libdm_iter_t);

/* Table functions. */
libdm_table_t libdm_table_create();
void libdm_table_destroy(libdm_table_t);

int libdm_table_set_start(uint64_t, libdm_table_t);
uint64_t libdm_table_get_start(libdm_table_t);

int libdm_table_set_length(uint64_t, libdm_table_t);
uint64_t libdm_table_get_length(libdm_table_t);

int libdm_table_set_target(const char *, libdm_table_t);
char * libdm_table_get_target(libdm_table_t);

int libdm_table_set_params(const char *, libdm_table_t);
char * libdm_table_get_params(libdm_table_t);

/* Target manipulating functions. */
void libdm_target_destroy(libdm_target_t);

char * libdm_target_get_name(libdm_target_t);
int32_t libdm_target_get_version(libdm_target_t, uint32_t *, size_t);

/* Dev manipulating functions. */
void libdm_dev_destroy(libdm_dev_t);

char * libdm_dev_get_name(libdm_dev_t);
uint32_t libdm_dev_get_minor(libdm_dev_t);

int libdm_dev_set_newname(const char *, libdm_cmd_t);

/* task run routine */
int libdm_task_run(libdm_task_t *);

/* /dev/mapper/control managing routines */
int libdm_control_open(const char *);
int libdm_control_close(int);

#endif /* _LIB_DM_H_ */
