/* required functionality : open, read, close, opendir, readdir, closedir */
#include <sys/types.h>
#include<sys/tarfs.h>
#include <sys/kprintf.h>
#include <strings.h>
#include<sys/elf64.h>
#include <sys/memset.h>
#include <strings.h>
#include<sys/task.h>
#include<sys/page.h>
#include <sys/virmem.h>
#define MIN(a,b)  (a<b)? a : b
Task *runningTask;

char pwd[200];

int
convert_oct_int(char *oct_string)
{
    int file_size = 0;
    int f = 1;
    for (int i = 10; i >= 0; i--)
    {
        file_size += ((oct_string[i] - '0') * f);
        f *= 8;
    }
    return file_size;
}

struct posix_header_ustar *
get_next_tar_header(struct posix_header_ustar *present_header)
{
    int file_size = convert_oct_int(present_header->size);
    uint64_t new_address;
    if (file_size % 512 == 0)
    {
        new_address = (uint64_t) present_header + ((file_size / 512) + 1) * 512;
    }
    else
    {
        new_address = (uint64_t) present_header + ((file_size / 512) + 2) * 512;
    }
    return (struct posix_header_ustar *) new_address;
}

int
file_exists(char *f_name)
{
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar *) &_binary_tarfs_start;
    while (tarfs_iterator < (struct posix_header_ustar *) &_binary_tarfs_end)
    {
//        kprintf("dir name : %s\n", tarfs_iterator->name);
        if (str_compare1(tarfs_iterator->name, f_name) == 0)
        {

            return 1;
        }
        tarfs_iterator = get_next_tar_header(tarfs_iterator);
    }
    return -1;

}

uint64_t
print_elf_file(char *binary_name)
{
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar *) &_binary_tarfs_start;
    while (tarfs_iterator < (struct posix_header_ustar *) &_binary_tarfs_end)
    {
        if (str_compare(tarfs_iterator->name, binary_name) == 0)
        {
            struct Elf64_Ehdr *elf_header = (struct Elf64_Ehdr *) ((uint64_t) tarfs_iterator + 512);
            elf_read(elf_header);
            return (uint64_t) elf_header->e_entry;
        }
        tarfs_iterator = get_next_tar_header(tarfs_iterator);
    }
    return 0;

}

int
get_free_fd(void *file_ptr)
{
    for (int i = 0; i < 100; i++)
    {
        if (runningTask->fd_array[i].alloted == 0)
        {
            runningTask->fd_array[i].alloted = 1;
            runningTask->fd_array[i].file_ptr = file_ptr;
            return i;
        }
    }
    return -1;
}

void
initialise_fds()
{
    for (int i = 0; i < 100; i++)
    {
        runningTask->fd_array[i].alloted = 0;
        runningTask->fd_array[i].file_sz = 0;
        runningTask->fd_array[i].num_bytes_read = 0;
    }
}
int
open(char *d_path)
{
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar *) &_binary_tarfs_start;
    int fd;
    if (file_exists(d_path) == 1)
    {
        while (tarfs_iterator < (struct posix_header_ustar *) &_binary_tarfs_end)
        {
            if (str_compare(tarfs_iterator->name, d_path) == 0)
            {
                fd = get_free_fd((void *) ((uint64_t) tarfs_iterator + 512));
                runningTask->fd_array[fd].file_sz=convert_oct_int(tarfs_iterator->size);
                if(fd == -1)
                {
                    kprintf("out of file descriptors");
                    return 0;
                }
                return fd;
            }
            tarfs_iterator = get_next_tar_header(tarfs_iterator);
        }
        return -1;
    }
    else
    {
        kprintf("Please provide valid path");
        return -1;
    }
}

int
read(int fd, char *buffer, int num_bytes)
{
    int file_sz = runningTask->fd_array[fd].file_sz;
    int num_bytes_read = runningTask->fd_array[fd].num_bytes_read;
    int num_bytes_remaining = file_sz - num_bytes_read;
    void *file_ptr = runningTask->fd_array[fd].file_ptr;
    if (num_bytes_read < file_sz)
    {
        memcopy(file_ptr, buffer, MIN(num_bytes, num_bytes_remaining));
        runningTask->fd_array[fd].num_bytes_read += num_bytes;
        return MIN(num_bytes, num_bytes_remaining);
    }
    else
    {
        return 0;
    }
}

void
close(int fd)
{
    runningTask->fd_array[fd].alloted = 0;
}

//void* opendir(char* d_name)
//{
//    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar*) &_binary_tarfs_start;
//    while(1)
//    {
//        if (tarfs_iterator->name[0] == '\0'||(tarfs_iterator > (struct posix_header_ustar*) &_binary_tarfs_end))
//            break;
//
//        if(str_compare(tarfs_iterator->name,d_name==0)&&strcmp(tarfs_iterator->typeflag, "5"))
//        {
//            return (void*)tarfs_iterator;
//        }
//        tarfs_iterator = get_next_tar_header(tarfs_iterator);
//    }
//    return  NULL;
//
//}
//

void
fetch_cwd(char *output_buffer)
{
    memcopy(pwd, output_buffer, len(pwd));
}

void
set_cwd(char *input_buffer)
{
    memset((void *) input_buffer, '\0', 200);
    memcopy(input_buffer, pwd, len(input_buffer));
}

void
tarfs_test()
{
    //FIXME:: should be done inside task
    char buff[100];
    runningTask = (Task *) page_alloc();
    int fd;
    initialise_fds();
//    kprintf("file bin exits -> %d", file_exists("bin/"));
    fd = open("test1/abc.txt");
    while (read(fd, buff, 100))
    {
        kprintf("value read %s \n", buff);
        memset((void *) buff, '\0', 100);
    }
    close(fd);
//    kprintf("file bin exits -> %d",open("test1/abc.txt"));
}