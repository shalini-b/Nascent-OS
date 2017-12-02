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
#define MIN(a, b)  (a<b)? a : b
Task *runningTask;
int MAX_FDS = 100;
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
//        kprintf("address : %p\n", tarfs_iterator);
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
    for (int i = 0; i < MAX_FDS; i++)
    {
        if (runningTask->fd_array[i].alloted == 0)
        {
            runningTask->fd_array[i].alloted = 1;
            runningTask->fd_array[i].file_ptr = file_ptr;
            runningTask->fd_array[i].last_matched_header = file_ptr;
            return i;
        }
    }
    return -1;
}

void
initialise_fds()
{
    for (int i = 0; i < MAX_FDS; i++)
    {
        runningTask->fd_array[i].alloted = 0;
        runningTask->fd_array[i].file_sz = 0;
        runningTask->fd_array[i].num_bytes_read = 0;
        runningTask->fd_array[i].is_dir = 0;
        runningTask->fd_array[i].last_matched_header = 0;
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
                fd = get_free_fd((void *) ((char *) tarfs_iterator + 512));
                if (fd == -1)
                {
                    kprintf("out of file descriptors\n");
                    return 0;
                }
                runningTask->fd_array[fd].file_sz = convert_oct_int(tarfs_iterator->size);
                return fd;
            }
            tarfs_iterator = get_next_tar_header(tarfs_iterator);
        }
        return -1;
    }
    else
    {
        kprintf("Please provide valid path\n");
        return -1;
    }
}

int
read(int fd, char *buffer, int num_bytes)
{
    if (file_des_validator(fd) != 0)
    {
        kprintf("Invalid fd\n");
        return 0;
    }
    int file_sz = runningTask->fd_array[fd].file_sz;
    int num_bytes_read = runningTask->fd_array[fd].num_bytes_read;
    int num_bytes_remaining = file_sz - num_bytes_read;
    void *file_ptr = runningTask->fd_array[fd].file_ptr;
    if (num_bytes_read < file_sz)
    {
        memcopy(file_ptr, buffer, MIN(num_bytes, num_bytes_remaining));
        char *temp_fp = (char *) runningTask->fd_array[fd].file_ptr;
        temp_fp += num_bytes;
        runningTask->fd_array[fd].file_ptr = (void *) temp_fp;
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
    runningTask->fd_array[fd].file_sz = 0;
    runningTask->fd_array[fd].num_bytes_read = 0;
}

int
open_dir(char *d_path)
{

    kprintf("dir contents ->%s\n", d_path);
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar *) &_binary_tarfs_start;
    int fd;
    if (file_exists(d_path) == 1)
    {
        while (tarfs_iterator < (struct posix_header_ustar *) &_binary_tarfs_end)
        {

//            kprintf("name %s type %s\n",tarfs_iterator->name,tarfs_iterator->typeflag);
            if (str_compare(tarfs_iterator->name, d_path) == 0 && str_compare(tarfs_iterator->typeflag, "5") == 0)
            {
                fd = get_free_fd((void *) (tarfs_iterator));
                if (fd == -1)
                {
                    kprintf("out of file descriptors\n");
                    return 0;
                }
                runningTask->fd_array[fd].last_matched_header = (void *) (tarfs_iterator);
                return fd;
            }
            tarfs_iterator = get_next_tar_header(tarfs_iterator);
        }
        return -1;
    }
    else
    {
        kprintf("Please provide valid path\n");
        return -1;
    }
}

int
file_des_validator(int fd)
{

    if (fd < MAX_FDS)
    {
        if (runningTask->fd_array[fd].alloted == 1)
        {
            return 0;
        }
        return 1;
    }
    else
    {
        return 1;
    }

}
int
read_dir(int fd, char *buffer)
{
    if (file_des_validator(fd) == 1)
    {
        kprintf("Invalid fd\n");
        return 1;
    }
    struct posix_header_ustar
        *tarfs_iterator = (struct posix_header_ustar *) runningTask->fd_array[fd].last_matched_header;
    char *dir_name = ((struct posix_header_ustar *) runningTask->fd_array[fd].file_ptr)->name;
    char *last_matched_name = ((struct posix_header_ustar *) runningTask->fd_array[fd].last_matched_header)->name;
    while (tarfs_iterator < (struct posix_header_ustar *) &_binary_tarfs_end)
    {
        if (is_sub_string(dir_name, tarfs_iterator->name) == 0
            && str_compare1(last_matched_name, tarfs_iterator->name) != 0)
        {
            //Fail to match with prev name
            if (str_compare1(dir_name, last_matched_name) == 0 ||
                is_sub_string(last_matched_name, tarfs_iterator->name) != 0)
            {
                runningTask->fd_array[fd].last_matched_header = (void *) tarfs_iterator;
                string_sub(tarfs_iterator->name, dir_name, buffer, '/');
                return 0;
            }

        }
        tarfs_iterator = get_next_tar_header(tarfs_iterator);
    }
    return 1;

}

void
close_dir(int fd)
{
    runningTask->fd_array[fd].alloted = 0;
    runningTask->fd_array[fd].file_sz = 0;
    runningTask->fd_array[fd].num_bytes_read = 0;
}

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


    //TEST 1 : FILE CALLS TEST
    int SIZE=100;
    char buff[SIZE];
    runningTask = (Task *) page_alloc();
    int fd;
    //FIXME:: should be done inside task
    initialise_fds();
    fd = open("test1/abc.txt");
    memset((void *) buff, '\0', SIZE+10);
    while (read(fd, buff, SIZE)!=0)
    {
        kprintf("%s", buff);
        memset((void *) buff, '\0', SIZE+10);
    }
    close(fd);


    fd = open("test1/abc.tx");
    memset((void *) buff, '\0', SIZE+10);
    while (read(fd, buff, SIZE)!=0)
    {
        kprintf("%s", buff);
        memset((void *) buff, '\0', SIZE+10);
    }
    close(fd);
    //*************************************************************************
    //TEST 2 : DIR CALLS TEST
    runningTask = (Task *) page_alloc();
    int DIR_SIZE = 100;
    //FIXME:: should be done inside task
    char dir_buff[DIR_SIZE];
    initialise_fds();
    fd = open_dir("test1/");
    memset((void *) dir_buff, '\0', DIR_SIZE);
    while (read_dir(fd, dir_buff) == 0)
    {
        kprintf("%s\n", dir_buff);
        memset((void *) dir_buff, '\0', DIR_SIZE);
    }
    close_dir(fd);
    kprintf("value is %d\n", file_exists("test1/test2/"));
//
//    //------------------------------------------------
//    //    NOTE :: -1 check return value
    char dir_buff2[DIR_SIZE];
    fd = open_dir("test1/test2/");
    memset((void *) dir_buff2, '\0', DIR_SIZE);
    while (read_dir(fd, dir_buff2) == 0)
    {
        kprintf("%s\n", dir_buff2);
        memset((void *) dir_buff2, '\0', DIR_SIZE);
    }
    close_dir(fd);

    fd = open_dir("test1/test3/");
    char dir_buff1[DIR_SIZE];
    memset((void *) dir_buff1, '\0', DIR_SIZE);
    while (read_dir(fd, dir_buff1) == 0)
    {
        kprintf("%s\n", dir_buff1);
        memset((void *) dir_buff1, '\0', DIR_SIZE);
    }
    close_dir(fd);

    fd = open_dir("abc");
    memset((void *) dir_buff1, '\0', DIR_SIZE);
    while (read_dir(fd, dir_buff1) == 0)
    {
        kprintf("%s\n", dir_buff1);
        memset((void *) dir_buff1, '\0', DIR_SIZE);
    }
    close_dir(fd);
//    //**********************************************************************************
//    //TEST 3 : FIND FILES TEST
    kprintf("file bin exits -> %d\n", file_exists("bin/"));
    kprintf("file test1/abc.txt exits -> %d\n", file_exists("test1/abc.txt"));
    kprintf("file test1/ exists %d\n", file_exists("test1/"));

}