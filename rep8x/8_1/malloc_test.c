#include <stdio.h>
#include <stdlib.h>
#include <string.h>


long MEMORY   = 16000*1000*100;  // test memory limit
long ALLOCATED   = 0;
long COUNT   = 8000;   // repeat count
long ACTIVE  = 1000;   // number of memroy segment
long MAXSIZE = 400000;  // 
long MINSIZE = 400;
long MIN_UNIT = 16 ;

typedef
struct mem_list {
	void *address;
	long size;
	struct mem_list *next;
} MEM_LIST, *MEM_LIST_PTR;


void
print_mem_list(MEM_LIST_PTR m)
{
    MEM_LIST_PTR n;
    if (!m) return;
    n = m->next;
    for(;n&&n!=m;n=n->next) {
       // some malloc returns very large value. remove it to keep output clear
      //	 if (n->address && ! (((long)n->address) & 0x700000000000))
	     printf("%p 0x%08lx\n",n->address,n->size);
    }
}

void
die(char *msg)
{
    fprintf(stderr,"%s\n",msg);
    exit(0);
}

void
option(long ac, char *av[])
{
    long i = 1;
    while(ac>1) {
	if (av[i][0] == '-') {
	    switch (av[i][1]) {
	    case 'c': COUNT = atol(av[++i]); break;
	    case 'a': ACTIVE = atol(av[++i]); break;
	    case 'u': MIN_UNIT = atol(av[++i]); break;
	    case 'm': MAXSIZE = atol(av[++i]); break;
	    case 'l': MINSIZE = atol(av[++i]); break;
	    case 'M': MEMORY = atol(av[++i]); break;
	    }
            ac--;
	}
	ac--; i++;
    }
}

int
main(int ac, char *av[])
{
    MEM_LIST mlist;
    MEM_LIST_PTR last = &mlist;
    MEM_LIST_PTR new;
    long i,size;

    option(ac,av);

    mlist.address = NULL;
    mlist.size = 0;
    for(i=0;i<ACTIVE;i++) {
	new = (MEM_LIST_PTR)malloc(sizeof(MEM_LIST)); 
	if (!new) die("malloc error");
	new->address = NULL;
	new->next = NULL;
	last->next = new;
	last = new;
    }
    last->next = &mlist;

    for(i=0;i++<COUNT;last=last->next) {
	size = ((random()%(MAXSIZE-MINSIZE))+MINSIZE)*MIN_UNIT;
	if (last->address) {
            ALLOCATED -= last->size;
            last->size = 0;
	    free(last->address);
	    last->address = 0;
	}
        if ( ALLOCATED + size > MEMORY) {
             // printf("%ld %ld size error\n",ALLOCATED , size);
             // continue;
        }
	last->size = size;
	last->address = (void *)malloc(size);
	if (!last->address) die("malloc error");
        memset(last->address, random(), size);
        ALLOCATED += size;
    }
    print_mem_list(&mlist);
    return 0;
}
