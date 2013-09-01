#include "gd_cache.h"
#include "str.h"
#include <string.h>

unsigned long 
get_free_inode(void)
{
	unsigned long num;
	struct gd_fs_inode_t* inodedesc;
	struct gd_fs_tableinode_t* inode;
	struct gd_fs_tableinode_t* binode;

	if (istat.free && free_inodes)
	{
		inode = free_inodes;
		num = free_inodes->num;
	}
	else
		return 0;
	if (inode->next)
	{
		inode->next->prev = NULL;
		free_inodes = inode->next;
	}
	istat.free--;
	istat.busy++;
	inodedesc = malloc(sizeof(struct gd_fs_inode_t));
	inodetable[num].inode = inodedesc;
	memset(&inodetable[num].inode->src, 0, sizeof(struct str_t));
	inodetable[num].inode->node=NULL;

	binode = busy_inodes;
	busy_inodes=&inodetable[num];
	inodetable[num].next = binode;
	inodetable[num].prev = NULL;
	inodetable[num].state = BUSYINODE;
	return num;
};

int
free_inode(unsigned long num)
{
	if (inodetable[num].state!=BUSYINODE || num > istat.allocated)
	{
		fprintf(stderr, "Free[%d:%d] Alloc[%d] F[%d] B[%d]\n", num, inodetable[num].state, istat.allocated, istat.free, istat.busy);
		return 1;
	}
	if ((busy_inodes==&inodetable[num]) && !inodetable[num].prev)
	{
	    busy_inodes=inodetable[num].next;
	    busy_inodes->prev = NULL;
	}
	else
	{
	    inodetable[num].prev->next = inodetable[num].next;
	    inodetable[num].next->prev = inodetable[num].prev;
	}
	free(inodetable[num].inode);
	inodetable[num].next = free_inodes;
	free_inodes = &inodetable[num];
	free_inodes->prev = NULL;
	inodetable[num].state = FREEINODE;
	istat.free++;
	istat.busy--;
	return 0;
}

int
init_inode_table(void)
{
	struct gd_fs_inode_t* inodedesc;
	unsigned long i = 0;
	unsigned long initmem = 0;
	int ii = 0;
	

	inodetable = malloc(MAXINODES*(sizeof(struct gd_fs_tableinode_t)));
	initmem+=MAXINODES*(sizeof(struct gd_fs_tableinode_t));
	istat.free = istat.busy = istat.allocated = 0;
	free_inodes = busy_inodes = NULL;
	for (ii=0; ii<2; ii++, istat.busy++, istat.allocated++)
	{
	    inodetable[ii].num = ii;
	    inodetable[ii].state = BUSYINODE;
	    inodetable[ii].pnum = 0;    
	    inodedesc = malloc(sizeof(struct gd_fs_inode_t));
	    initmem+=sizeof(struct gd_fs_inode_t);
	    inodetable[ii].inode = inodedesc;
	    memset(&inodetable[ii].inode->src, 0, sizeof(struct str_t));
	    inodetable[ii].inode->node=NULL;
	    inodetable[ii].inode->mode= S_IFDIR | 0700;
	    busy_inodes = &inodetable[ii];
	    if (!ii)
	    {
		busy_inodes->next=NULL;
		busy_inodes->prev=NULL;
	    }
	    else
	    {
		busy_inodes->next=&inodetable[ii]-1;
		busy_inodes->prev=NULL;
	    }
	};
	for (i=2; i<MAXINODES; i++, istat.free++, istat.allocated++)
	{
		inodetable[i].num=i;
		inodetable[i].pnum = 1;
		inodetable[i].state = FREEINODE;
		inodetable[i].inode = NULL;
		if (i==2)
		{
		    inodetable[i].prev = NULL;
		    free_inodes = &inodetable[i];
		}
		else
		    inodetable[i].prev = &inodetable[i-1];	
		if (i==MAXINODES-1)
		    inodetable[i].next = NULL;
		else
		    inodetable[i].next = &inodetable[i+1];	
	};
	if ((ii+i-2)==MAXINODES)
	{

		fprintf(stderr, "Init memory for inode: %ld bytes. sizeof(inode) = %ld bytes\n", initmem, sizeof(struct gd_fs_tableinode_t)+sizeof(struct gd_fs_inode_t));
		return 0;
	}
	fprintf(stderr, "Inode init count: %d, Max: %d\n", ii+i, MAXINODES);
	return 1;
}

/* get_all_busy_inodes && get_all_free_inodes for test only */
unsigned long
get_all_busy_inodes(void)
{
    unsigned long i;	
    struct gd_fs_tableinode_t *binode;

    fprintf(stderr, "[TEST_BUSY] GET all busy inodes\n");
    for(binode = busy_inodes, i=1 ;binode->next!=NULL;binode=binode->next, i++)	
    {
	fprintf(stderr, "\r\b[TEST_BUSY][%d] Inode num[%ld] State[%d] Address[%p]", i, binode->num, binode->state, binode);
	if (!binode->state)
		sleep(5);
    };
    if (binode && binode->next==NULL)
    {
	fprintf(stderr, "\r\b[TEST_BUSY][%d] Inode num[%ld] State[%d] Address[%p]", i, binode->num, binode->state, binode);
	if (!binode->state)
		sleep(5);
    }
    fprintf(stderr, "\n");
    return i;
}

unsigned long
get_all_free_inodes(void)
{
    unsigned long i;	
    struct gd_fs_tableinode_t *finode;

    fprintf(stderr, "[TEST_FREE] GET all free inodes\n");
    for(finode = free_inodes, i=1 ;finode->next!=NULL;finode=finode->next, i++)	
    {
	fprintf(stderr, "\r\b[TEST_FREE][%d] Inode num[%ld] State[%d] Address[%p]", i, finode->num, finode->state, finode);
	if (finode->state)
		sleep(5);
    };
    if (finode && finode->next==NULL)
    {
	fprintf(stderr, "\r\b[TEST_BUSY][%d] Inode num[%ld] State[%d] Address[%p]", i, finode->num, finode->state, finode);
	if (finode->state)
		sleep(5);
    }
    fprintf(stderr, "\n");
    return i;
}

unsigned long
get_inode_by_href(struct str_t* href, unsigned long inum)
{
    struct gd_fs_tableinode_t *binode;

    for(binode = busy_inodes ;binode->next!=NULL;binode=binode->next)	
    {
	if (binode->num == inum || binode->num < 2)
	    continue;
	if (!strcmp(binode->inode->node->feed.str, href->str))
	{
		return binode->num;
	}
    }
    return 0;
}

int
set_parent_inode(void)
{
    int res;	
    unsigned long inode = 0;
    struct gd_fs_tableinode_t *binode;

    for(binode = busy_inodes ;binode->next!=NULL;binode=binode->next)	
    {
	if (binode->num < 2)
		continue;
	if (binode->inode->node->parent_href.len!=0)
	{
	    inode = get_inode_by_href(&binode->inode->node->parent_href, binode->num);
	    if (!inode)
		    return 1;
	    binode->pnum = inode;
	}
	else
	    binode->pnum = 1;
    }
    return 0;

}
