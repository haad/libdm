
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdm.h"
#include "netbsd-dm.h"


struct cmd_version cmd_ver[] = {
		{"version", {4, 0, 0}},
		{"targets", {4, 0, 0}},
		{"create", {4, 0, 0}},
		{"info",   {4, 0, 0}},
		{"mknodes",{4, 0, 0}},
		{"names",  {4, 0, 0}},
		{"suspend",{4, 0, 0}},
		{"remove", {4, 0, 0}},
		{"rename", {4, 0, 0}},
		{"resume", {4, 0, 0}},
		{"clear",  {4, 0, 0}},
		{"deps",   {4, 0, 0}},
		{"reload", {4, 0, 0}},
		{"status", {4, 0, 0}},
		{"table",  {4, 0, 0}},
		{NULL, {0, 0, 0}}
};

int
libdm_control_open(const char *path)
{
	int fd;
	
	if ((fd = open(path, O_RDWR)) < 0)
		return -1;

	return fd;
}	

int
libdm_control_close(int fd)
{
	return close(fd);
}

/* Destroy iterator for arrays such as version strings, cmd_data. */
void
libdm_iter_destroy(libdm_iter_t libdm_iter)
{
	prop_object_iterator_release(libdm_iter);
}

/*
 * Issue ioctl call to kernel, releasing both dictionaries is
 * left on callers.
 */
int
libdm_task_run(libdm_task_t *libdm_task)
{
	prop_dictionary_t dict;
	int libdm_control_fd = -1;
	
	if ((libdm_control_fd = libdm_control_open("/dev/mapper/control")) < 0)
		err(EXIT_FAILURE, "libdm_task_run");
	
//	prop_dictionary_externalize_to_file((prop_dictionary_t)*libdm_task,
//		"/tmp/test_in");

	if (prop_dictionary_sendrecv_ioctl((prop_dictionary_t)*libdm_task, libdm_control_fd,
		NETBSD_DM_IOCTL, &dict) != 0) {

		libdm_task_destroy(*libdm_task);
		libdm_task = NULL;
		libdm_control_close(libdm_control_fd);
		return EXIT_FAILURE;
	}

	libdm_control_close(libdm_control_fd);
	libdm_task_destroy(*libdm_task);
	*libdm_task = dict;
	
	return EXIT_SUCCESS;
}


/* Create libdm General task structure */
libdm_task_t
libdm_task_create(const char *command)
{
	libdm_task_t task;
	size_t i,len,slen;
	prop_array_t ver;

	task = NULL;

	if ((task = (libdm_task_t)prop_dictionary_create()) == NULL)
		return NULL;
	
	if ((prop_dictionary_set_cstring((prop_dictionary_t)task,
		    DM_IOCTL_COMMAND, command)) == false) {
		prop_object_release(task);
		return NULL;
	}
	
	len = strlen(command);
	
	for(i=0; cmd_ver[i].cmd != NULL; i++){
		slen = strlen(cmd_ver[i].cmd);

		if (len != slen)
			continue;

		if ((strncmp(command, cmd_ver[i].cmd, slen)) == 0) {
			ver = prop_array_create();
			prop_array_add_uint32(ver, cmd_ver[i].version[0]);
			prop_array_add_uint32(ver, cmd_ver[i].version[1]);
			prop_array_add_uint32(ver, cmd_ver[i].version[2]);

			prop_dictionary_set((prop_dictionary_t)task,
			    DM_IOCTL_VERSION, ver);

			prop_object_release(ver);
			break;
		}
	}
	
	return task;
}

void
libdm_task_destroy(libdm_task_t libdm_task)
{
	if (libdm_task != NULL)
		prop_object_release((prop_dictionary_t)libdm_task);
}

/* Set device name */
int
libdm_task_set_name(const char *name, libdm_task_t libdm_task)
{

	if ((prop_dictionary_set_cstring((prop_dictionary_t)libdm_task,
		    DM_IOCTL_NAME, name)) == false)
		return ENOENT;

	return 0;
}

/* Set device name */
char*
libdm_task_get_name(libdm_task_t libdm_task)
{
	char *name;
	
	prop_dictionary_get_cstring_nocopy((prop_dictionary_t)libdm_task,
	    DM_IOCTL_NAME, (const char **)&name);

	return name;
}

/* Set device uuid */
int
libdm_task_set_uuid(const char *uuid, libdm_task_t libdm_task)
{

	if ((prop_dictionary_set_cstring((prop_dictionary_t)libdm_task,
		    DM_IOCTL_NAME, uuid)) == false)
		return ENOENT;
	
	return 0;
}

/* Set device name */
char*
libdm_task_get_uuid(libdm_task_t libdm_task)
{
	char *uuid;
	
	prop_dictionary_get_cstring_nocopy((prop_dictionary_t)libdm_task,
	    DM_IOCTL_UUID, (const char**)&uuid);

	return uuid;
}

/* Set device name */
char*
libdm_task_get_command(libdm_task_t libdm_task)
{
	char *command;
	
	prop_dictionary_get_cstring_nocopy((prop_dictionary_t)libdm_task,
	    DM_IOCTL_COMMAND, (const char**)&command);

	return command;
}

int32_t
libdm_task_get_cmd_version(libdm_task_t libdm_task, uint32_t *ver, size_t size)
{
	prop_array_t prop_ver;
	prop_object_t obj;
	prop_object_iterator_t iter;

	size_t i;

	prop_ver = prop_dictionary_get((prop_dictionary_t)libdm_task,
	    DM_IOCTL_VERSION);

	i = prop_array_count(prop_ver);
	
	if (i > size)
		return -i;
	
	iter = prop_array_iterator(prop_ver);

	for (i = 0; i < size; i++) {
		if ((obj = prop_object_iterator_next(iter)) == NULL)
			break;
		
		ver[i] = (uint32_t)prop_number_unsigned_integer_value(obj);
	}

	prop_object_iterator_release(iter);
	
	return i;
}

/* Select device minor number. */
int
libdm_task_set_minor(uint32_t minor, libdm_task_t libdm_task)
{

	if ((prop_dictionary_set_uint32((prop_dictionary_t)libdm_task,
		    DM_IOCTL_MINOR, minor)) == false)
		return ENOENT;

	return 0;
}

/* Select device minor number. */
uint32_t
libdm_task_get_minor(libdm_task_t libdm_task)
{
	uint32_t minor;
	
	prop_dictionary_get_uint32((prop_dictionary_t)libdm_task,
		    DM_IOCTL_MINOR, &minor);

	return minor;
}

/* Set ioctl protocol status flags. */
int
libdm_task_set_flags(uint32_t flags, libdm_task_t libdm_task)
{

	if ((prop_dictionary_set_uint32((prop_dictionary_t)libdm_task,
		    DM_IOCTL_FLAGS, flags)) == false)
		return ENOENT;

	return 0;
}

/* Get ioctl protocol status flags. */
uint32_t
libdm_task_get_flags(libdm_task_t libdm_task)
{
	uint32_t flags;
	
	prop_dictionary_get_uint32((prop_dictionary_t)libdm_task,
		    DM_IOCTL_FLAGS, &flags);

	return flags;
}

/* Set ioctl protocol status flags. */
uint32_t
libdm_task_get_target_num(libdm_task_t libdm_task)
{
	uint32_t count;
	
	prop_dictionary_get_uint32((prop_dictionary_t)libdm_task,
		    DM_IOCTL_TARGET_COUNT, &count);

	return count;
}


/* Set cmd_data dictionary entry to task struct. */
int
libdm_task_set_cmd(libdm_cmd_t libdm_cmd, libdm_task_t libdm_task)
{
	
	if ((prop_dictionary_set((prop_dictionary_t)libdm_task,
		    DM_IOCTL_CMD_DATA, libdm_cmd)) == false)
		return ENOENT;

	return 0;
}

/* Get cmd_data dictionary entry from task struct */
libdm_cmd_t
libdm_task_get_cmd(libdm_task_t libdm_task)
{
	return prop_dictionary_get((prop_dictionary_t)libdm_task, DM_IOCTL_CMD_DATA);
}

/* Command functions */

/* Functions for creation, destroing, set, get of command area of
   ioctl dictionary. */
libdm_cmd_t
libdm_cmd_create(void)
{
	return prop_array_create();
}

void
libdm_cmd_destroy(libdm_cmd_t libdm_cmd)
{
	prop_object_release((prop_array_t)libdm_cmd);
}

libdm_iter_t
libdm_cmd_iter_create(libdm_cmd_t libdm_cmd)
{
	return (libdm_iter_t)prop_array_iterator((prop_array_t)libdm_cmd);
}

int
libdm_cmd_set_table(libdm_table_t libdm_table, libdm_cmd_t libdm_cmd)
{
	return prop_array_add((prop_array_t)libdm_cmd,
	    (prop_dictionary_t) libdm_table);
}


libdm_target_t
libdm_cmd_get_target(libdm_iter_t iter)
{
	return (libdm_target_t)prop_object_iterator_next(iter);
}

libdm_table_t
libdm_cmd_get_table(libdm_iter_t iter)
{
	return (libdm_table_t)prop_object_iterator_next(iter);
}

libdm_dev_t
libdm_cmd_get_dev(libdm_iter_t iter)
{
	return (libdm_dev_t)prop_object_iterator_next(iter);
}

/* deps manipulation routines */
uint64_t
libdm_cmd_get_deps(libdm_iter_t libdm_iter)
{
	prop_object_t obj;
	uint64_t deps;
      
	obj = prop_object_iterator_next(libdm_iter);

	deps = prop_number_unsigned_integer_value(obj);

	return deps;
}

/* Table manipulation routines */
libdm_table_t
libdm_table_create(void)
{
	return prop_dictionary_create();
}

void
libdm_table_destroy(libdm_table_t libdm_table)
{
	prop_object_release((prop_dictionary_t)libdm_table);
}

int
libdm_table_set_start(uint64_t start, libdm_table_t libdm_table)
{
	return prop_dictionary_set_uint64(libdm_table, DM_TABLE_START, start);
} 

uint64_t
libdm_table_get_start(libdm_table_t libdm_table)
{
	uint64_t start;
	
	prop_dictionary_get_uint64(libdm_table, DM_TABLE_START, &start);

	return start;
}

int
libdm_table_set_length(uint64_t length, libdm_table_t libdm_table)
{
	return prop_dictionary_set_uint64(libdm_table, DM_TABLE_LENGTH, length);
}

uint64_t
libdm_table_get_length(libdm_table_t libdm_table)
{
	uint64_t length;
	
	prop_dictionary_get_uint64(libdm_table, DM_TABLE_LENGTH, &length);

	return length;
} 

int
libdm_table_set_target(const char *name, libdm_table_t libdm_table)
{
	return prop_dictionary_set_cstring(libdm_table, DM_TABLE_TYPE, name);
}

char *
libdm_table_get_target(libdm_table_t libdm_table)
{
	char *target;
	
	prop_dictionary_get_cstring_nocopy(libdm_table, DM_TABLE_TYPE, (const char**)&target);

	return target;
}

int
libdm_table_set_params(const char *params, libdm_table_t  libdm_table)
{
	return prop_dictionary_set_cstring(libdm_table, DM_TABLE_PARAMS, params);
}

/*
 * Get tableparams string from libdm_table_t
 * returned char * is dynamically allocated caller should free it.
 */
char *
libdm_table_get_params(libdm_table_t  libdm_table)
{
	char *params;
	
	prop_dictionary_get_cstring_nocopy(libdm_table, DM_TABLE_PARAMS, (const char**)&params);

	return params;
}

/* Target manipulation routines */
void
libdm_target_destroy(libdm_target_t libdm_target)
{
	prop_object_release((prop_dictionary_t)libdm_target);
}

char*
libdm_target_get_name(libdm_target_t libdm_target)
{
	char *name;
	
	prop_dictionary_get_cstring_nocopy((prop_dictionary_t)libdm_target, DM_TARGETS_NAME, (const char**)&name);

	return name;
} 

int32_t
libdm_target_get_version(libdm_target_t libdm_target, uint32_t *ver, size_t size)
{
	prop_array_t prop_ver;
	prop_object_t obj;
	prop_object_iterator_t iter;

	size_t i;

	prop_ver = prop_dictionary_get((prop_dictionary_t)libdm_target,
	    DM_TARGETS_VERSION);
	
	i = prop_array_count(prop_ver);
	
	if (i > size)
		return -i;
	
	iter = prop_array_iterator(prop_ver);

	for (i = 0; i < size; i++) {
		if ((obj = prop_object_iterator_next(iter)) == NULL)
			break;
		
		ver[i] = (uint32_t)prop_number_unsigned_integer_value(obj);
	}

	prop_object_iterator_release(iter);
	
	return i;
}


/* dev manipulation routines */

void
libdm_dev_destroy(libdm_dev_t libdm_dev)
{
	prop_object_release((prop_dictionary_t)libdm_dev);
}

char *
libdm_dev_get_name(libdm_dev_t libdm_dev)
{
	char *name;
	
	prop_dictionary_get_cstring_nocopy((prop_dictionary_t)libdm_dev, DM_DEV_NAME, (const char**)&name);

	return name;
} 

uint32_t
libdm_dev_get_minor(libdm_dev_t libdm_dev)
{
	uint32_t dev;
	
	prop_dictionary_get_uint32((prop_dictionary_t)libdm_dev, DM_DEV_DEV, &dev);

	return dev;
}

int
libdm_dev_set_newname(const char *newname, libdm_cmd_t libdm_cmd)
{
	return prop_array_set_cstring((prop_array_t)libdm_cmd, 0, newname);
}

