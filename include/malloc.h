typedef long Align;

typedef union header
{
    struct
    {
        union header *ptr;
        size_t size;
    } s;

    Align x;

} MHeader;

MHeader base = {0};
MHeader* freeptr = NULL;

void free(void *ap);
void* malloc (size_t nbytes);
MHeader* morecore(int num_u);
void* sbrk(int limit);