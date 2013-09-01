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

#ifndef _GOOGLE_DRIVE_CACHE_H
#define _GOOGLE_DRIVE_CACHE_H

#include <libxml/tree.h>
#include <pthread.h>
#include "str.h"
//File Types
#include <sys/stat.h>

// Do we need to represent folders differently from files?
// For the time being, ignore folders
struct gd_fs_entry_t {
	struct str_t author; // the file owner?
	struct str_t author_email;

	struct str_t lastModifiedBy; // do we even care about this?
	struct str_t lastModifiedBy_email;

	struct str_t filename; // 'title' in the XML from directory-list
	struct str_t resourceID;
	struct str_t src; // The url for downloading the file
	struct str_t feed; // The url for getting the XML feed for this entry
	struct str_t parent; // 'title' of the parent
	struct str_t parent_href; // 'title' of the parent

	struct str_t cache;
	int cached;
	int mode;	// type of file
	int inode;
	int shared;	// shared or not?

	unsigned long size; // file size in bytes, 'gd:quotaBytesUsed' in XML
	struct str_t md5; // 'docs:md5Checksum' in XML
	int md5set; // indicates if the md5sum was available for this entry

	// Add some data we can use in getattr()

	// Linked list
	struct gd_fs_entry_t *next;
};
/* START DESCRIPTION FOR INODES	*/
#define	MAXINODES   50000
#define	FREEINODE   0
#define	BUSYINODE   1
/* Inode's description */
struct gd_fs_inode_t {
	int mode;		    /* Mode's bits	*/
	struct gd_fs_entry_t *node; /* pointer to f desc*/
	struct str_t src;	    /* Data segment	*/

};

/* Tables of inode - description */
struct gd_fs_tableinode_t {
	unsigned long num;	    /* Inode's num	*/
	unsigned long pnum;		    /* parent's inode	*/
	struct gd_fs_inode_t* inode;/* Inode pointer	*/
	int state;		    /* Free or busy?	*/
	struct gd_fs_tableinode_t *next;
	struct gd_fs_tableinode_t *prev;
};

struct inode_stats {
	unsigned long free;
	unsigned long busy;
	unsigned long allocated;
};

struct inode_stats istat;
struct gd_fs_tableinode_t *free_inodes;
struct gd_fs_tableinode_t *busy_inodes;
struct gd_fs_tableinode_t *inodetable;

unsigned long get_free_inode(void);
unsigned long get_all_free_inodes(void);
unsigned long get_all_busy_inodes(void);
unsigned long get_inode_by_href(struct str_t* href, unsigned long inum);
int init_inode_table(void);
int set_parent_inode(void);

/* END DESCRIPTION FOR INODES	*/

// Since hsearch et al are likely not threadsafe we need to use a read write
// lock to prevent corruption. The write lock should only be taken when we
// need to add a new item.
// What to do about removing a file? Just mark the entry as invalid? There is
// no removal action for hsearch et al.
// Does this also need a condition variable?
struct gd_fs_lock_t {
	pthread_rwlock_t *lock;
};

void gd_fs_entry_destroy(struct gd_fs_entry_t* entry);

struct gd_fs_entry_t* gd_fs_entry_from_xml(xmlDocPtr xml, xmlNodePtr node);
struct gd_fs_entry_t* gd_fs_entry_find(const char* key);

struct str_t* xml_get_md5sum(const struct str_t* xml);

int create_hash_table(size_t size, const struct gd_fs_entry_t* head);
void destroy_hash_table();

#endif
