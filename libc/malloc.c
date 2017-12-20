//REFERENCE :: https://stackoverflow.com/questions/13159564/explain-this-implementation-of-malloc-from-the-kr-book
//https://stackoverflow.com/questions/25966610/explain-this-implementation-of-free-from-the-kr-book

#include <defs.h>
#include <malloc.h>
#include <syscalls.h>

MHeader* morecore(int num_u)
{
    char *cp;
    MHeader *up;

    if (num_u < 1024)
        num_u = 1024;

    cp = (char*)sbrk((int)(num_u * sizeof(MHeader)));
    if (cp == (char *) -1)
        return NULL;

    up = (MHeader *) cp;
    up->s.size = num_u;
    free(((void*)(up + 1)));

    return freeptr;
}

void* sbrk(int limit)
{
    return (void *) sys_sbrk(limit);
}

void* malloc (size_t nbytes)
{
    MHeader *p, *prevptr;
    size_t   nunits;
    int     is_allocating;
    void*    result;

    nunits = (nbytes + sizeof(MHeader) - 1) / sizeof(MHeader) + 1;

    prevptr = freeptr;
    if (prevptr == NULL)
    {
        base.s.ptr = freeptr = prevptr = &base;
        base.s.size = 0;
    }

    is_allocating = 1;
    for (p = prevptr->s.ptr; is_allocating == 1; p = p->s.ptr)
    {
        if (p->s.size >= nunits)
        {
            if (p->s.size == nunits)
            {
                prevptr->s.ptr = p->s.ptr;
            }
            else
            {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }

            freeptr = prevptr;
            is_allocating = 0;
            result = p+1;
        }

        if (p == freeptr)
        {
            p = morecore(nunits);
            if (p == NULL)
            {
                is_allocating = 0;
                result = NULL;
            }
        }
        prevptr = p;
    }
    return result;
}


void free(void *ap)
{
    MHeader *bp, *p;

    bp = (MHeader *)ap - 1;
    for (p = freeptr; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break;

    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else {
        bp->s.ptr = p->s.ptr;
    }
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else {
        p->s.ptr = bp;
    }
    freeptr = p;
}