/*
	fuse-google-drive: a fuse filesystem wrapper for Google Drive
	Copyright (C) 2012  James Cline

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
 	published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef _GOOGLE_DRIVE_INTERFACE_H
#define _GOOGLE_DRIVE_INTERFACE_H

#define FUSE_USE_VERSION 26

#define MAXDIRNAME 256
#define MAXENCODELEN 12

#include <curl/curl.h>
#include <curl/multi.h>
#include <fuse.h>
#include <stdlib.h>

#include "gd_cache.h"
#include "stack.h"
#include "str.h"

//#define KEY_HELP	0
//#define KEY_VERSION     1
#define GDI_OPT(t, p, v) { t, offsetof(struct gdi_config, p), v }

struct gdi_config {
	char *encoding;
};

enum {
	KEY_HELP,
	KEY_VERSION,
};

static struct fuse_opt gdi_opts[] = {
	GDI_OPT("--encoding=%s",       encoding, 0),

	FUSE_OPT_KEY("-V",             KEY_VERSION),
	FUSE_OPT_KEY("--version",      KEY_VERSION),
	FUSE_OPT_KEY("-h",             KEY_HELP),
	FUSE_OPT_KEY("--help",         KEY_HELP),
	FUSE_OPT_END
};

struct gdi_state {
	char *clientsecrets;
	char *redirecturi;
	char *clientid;
	CURLM *curlmulti;
	char *code;

	// So we can identify files owned by this user
	char *email;

	struct str_t access_token;
	struct str_t id_token;
	struct str_t refresh_token;
	long token_expiration;
	struct str_t token_type;

	struct gd_fs_entry_t *head;
	struct gd_fs_entry_t *tail;
	size_t num_files;

	struct stack_t *stack;

	int callback_error;

	struct str_t oauth_header;
};

char* encoding;

char* urlencode (const char *url, size_t* length);

int gdi_get_credentials();

int gdi_init(struct gdi_state *state);
void gdi_destroy(struct gdi_state *state);

/* Interface for various operations */
void gdi_get_file_list(struct gdi_state *state);
const char* gdi_strip_path(const char* path);
int gdi_load(struct gdi_state* state, struct gd_fs_entry_t* entry);
const char* gdi_read(size_t *size, struct gd_fs_entry_t* entry, off_t offset);

#endif
