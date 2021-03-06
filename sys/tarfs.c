/* required functionality : open, read, close, opendir, readdir, closedir */
#include <sys/types.h>
#include <sys/tarfs.h>
#include <sys/kprintf.h>
#include <strings.h>
#include <sys/elf64.h>
#include <sys/memset.h>
#include <strings.h>
#include <sys/page.h>
#include <sys/virmem.h>
#include <sys/process.h>
#include <sys/terminal.h>
#define MIN(a, b)  (a<b)? a : b

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

int
validate_binary(char *f_name)
{
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar *) &_binary_tarfs_start;
    while (tarfs_iterator < (struct posix_header_ustar *) &_binary_tarfs_end)
    {
        if (str_compare1(tarfs_iterator->name, f_name) == 0)
        {
            struct Elf64_Ehdr *elf_header = (struct Elf64_Ehdr *) ((uint64_t) tarfs_iterator + 512);
            if (is_elf_format(elf_header) == 1)
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }
        tarfs_iterator = get_next_tar_header(tarfs_iterator);
    }
    return -1;
}

uint64_t
load_elf(Task *cur_pcb, char *binary_name, char *argv[])
{
    char buff[100];
    memset((void *) buff, '\0', 100);
    path_sanitize(binary_name, buff);
    return load_elf1(cur_pcb, &buff[1], argv);
}

void
remove_d(char *buff, char *b)
{
    int i = 0;
    int j = 0;
    int c = 0;
    int p = 0;
    while (buff[i] != '\0')
    {
        if (buff[i] == '.')
        {
            c++;
        }
        if (c == 2 && i != 2)
        {
            p = j - 3;
            while (b[p] != '/')
            {

                p--;
            }
            p++;
            b[p] = '\0';
            c = 0;
            j = p;
            if (buff[i + 1] != '\0')
            {
                i += 2;
                if (buff[i] == '.')
                {
                    c = 1;
                }
            }
            else
            {
                i++;
            }
        }
        b[j] = buff[i];
        if (b[j] == '\0')
        {
            i--;
        }
        i++;
        j++;
    }
    b[j] = '\0';
}
void
path_sanitize(char *binary_name, char *buff)
{
    if (binary_name[0] == '/')
    {
        str_copy(binary_name, buff);
    }
    else
    {
//        str_copy(RunningTask->cwd,pwd);
        int l = len(RunningTask->cwd);
        str_copy(RunningTask->cwd, buff);
        if (buff[l - 1] != '/')
        {
            buff[l] = '/';
            l++;
        }
        str_copy(binary_name, &buff[l]);
    }
    char b[100];
//    str_copy(buff,b);
    remove_d(buff, b);
    memset(buff, '\0', 100);
    str_copy(b, buff);
//    if(str_compare(binary_name,"cat") == 0)
//    {
//        str_copy("bin/cat", buff);
//    }
//    else if(str_compare(binary_name,"echo") == 0)
//    {
//        str_copy("bin/echo", buff);
//    }
//    else if(str_compare(binary_name,"kill") == 0)
//    {
//        str_copy("bin/kill", buff);
//    }
//    else if(str_compare(binary_name,"ls") == 0)
//    {
//        str_copy("bin/ls", buff);
//    }
//    else if(str_compare(binary_name,"ps") == 0)
//    {
//        str_copy("bin/ps", buff);
//    }
//    else if(str_compare(binary_name,"sleep") == 0)
//    {
//        str_copy("bin/sleep", buff);
//    }
//    else
//    {
//        str_copy(binary_name, buff);
//    }
//    kprintf("value issss %s",buff);
    return;
}

// FIXME: change return type
uint64_t
load_elf1(Task *cur_pcb, char *binary_name, char *argv[])
{
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar *) &_binary_tarfs_start;
    while (tarfs_iterator < (struct posix_header_ustar *) &_binary_tarfs_end)
    {
        if (str_compare(tarfs_iterator->name, binary_name) == 0)
        {
            struct Elf64_Ehdr *elf_header = (struct Elf64_Ehdr *) ((uint64_t) tarfs_iterator + 512);
            if (elf_read(elf_header, cur_pcb, binary_name, argv) == 0)
            {
//                kprintf("elf read done");
                return (uint64_t) elf_header->e_entry;
            }

            else
            {
                return -1;
            }
        }
        tarfs_iterator = get_next_tar_header(tarfs_iterator);
    }
    return -1;
}

int
get_free_fd(void *file_ptr)
{
    for (int i = 0; i < MAX_FDS; i++)
    {
        if (RunningTask->fd_array[i].alloted == 0)
        {
            RunningTask->fd_array[i].alloted = 1;
            RunningTask->fd_array[i].file_ptr = file_ptr;
            RunningTask->fd_array[i].last_matched_header = file_ptr;
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
        RunningTask->fd_array[i].alloted = 0;
        RunningTask->fd_array[i].file_sz = 0;
        RunningTask->fd_array[i].num_bytes_read = 0;
        RunningTask->fd_array[i].is_dir = 0;
        RunningTask->fd_array[i].last_matched_header = 0;
    }
}
int
open_s(char *d_path1, int flags)
{

    char d_path2[100];
    path_sanitize(d_path1, d_path2);
    char *d_path = (char *) &d_path2[1];
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar *) &_binary_tarfs_start;
    int fd;
    kprintf("dpath is %s", d_path);
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
                RunningTask->fd_array[fd].file_sz = convert_oct_int(tarfs_iterator->size);
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
read_s(int fd, char *buffer, int num_bytes)
{
    if (fd == 0)
    {

        schedule_terminal_task(buffer, num_bytes);
    }
    else
    {
        if (file_des_validator(fd) != 0)
        {
            kprintf("file not found\n");
            return 0;
        }
        int file_sz = RunningTask->fd_array[fd].file_sz;
        int num_bytes_read = RunningTask->fd_array[fd].num_bytes_read;
        int num_bytes_remaining = file_sz - num_bytes_read;
        void *file_ptr = RunningTask->fd_array[fd].file_ptr;
        if (num_bytes_read < file_sz)
        {
            memcopy(file_ptr, buffer, MIN(num_bytes, num_bytes_remaining));
            char *temp_fp = (char *) RunningTask->fd_array[fd].file_ptr;
            temp_fp += num_bytes;
            RunningTask->fd_array[fd].file_ptr = (void *) temp_fp;
            RunningTask->fd_array[fd].num_bytes_read += num_bytes;
            return MIN(num_bytes, num_bytes_remaining);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

int
fgets_s(int fd, char *buffer, int num_bytes)
{
    if (fd == 0)
    {

        schedule_terminal_task(buffer, num_bytes);
    }
    else
    {
        if (file_des_validator(fd) != 0)
        {
            kprintf("Invalid fd\n");
            return 0;
        }
        int file_sz = RunningTask->fd_array[fd].file_sz;
        int num_bytes_read = RunningTask->fd_array[fd].num_bytes_read;
        int num_bytes_remaining = file_sz - num_bytes_read;
        void *file_ptr = RunningTask->fd_array[fd].file_ptr;
        if (num_bytes_read < file_sz)
        {
            memcopy(file_ptr, buffer, MIN(num_bytes, num_bytes_remaining));
            char *temp_fp = (char *) RunningTask->fd_array[fd].file_ptr;
            temp_fp += num_bytes;
            RunningTask->fd_array[fd].file_ptr = (void *) temp_fp;
            RunningTask->fd_array[fd].num_bytes_read += num_bytes;
            return MIN(num_bytes, num_bytes_remaining);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

int
close_s(int fd)
{
    RunningTask->fd_array[fd].alloted = 0;
    RunningTask->fd_array[fd].file_sz = 0;
    RunningTask->fd_array[fd].num_bytes_read = 0;
    return 0;
}

int
open_dir(char *d_path1)
{
    if (str_compare("/", d_path1) == 0)
    {
//        kprintf("usr \n  bin  \n  etc \n lib  ");
        return 25;
    }

    int fd;
    char d_path2[100];
    path_sanitize(d_path1, d_path2);
    if (d_path2[len(d_path2) - 1] != '/')
    {
        d_path2[len(d_path2)] = '/';
        d_path2[len(d_path2) + 1] = '\0';
    }
    char *d_path = (char *) &d_path2[1];
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar *) &_binary_tarfs_start;
//    kprintf("dpath is %s",d_path);
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
                RunningTask->fd_array[fd].last_matched_header = (void *) (tarfs_iterator);
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
        if (RunningTask->fd_array[fd].alloted == 1)
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
    if (fd == 25)
    {
        kprintf("usr\n bin\netc\nlib\n ");
        return 1;
    }

    if (file_des_validator(fd) == 1)
    {
        kprintf("Invalid fd\n");
        return 1;
    }
    struct posix_header_ustar
        *tarfs_iterator = (struct posix_header_ustar *) RunningTask->fd_array[fd].last_matched_header;
    char *dir_name = ((struct posix_header_ustar *) RunningTask->fd_array[fd].file_ptr)->name;
    char *last_matched_name = ((struct posix_header_ustar *) RunningTask->fd_array[fd].last_matched_header)->name;
    while (tarfs_iterator < (struct posix_header_ustar *) &_binary_tarfs_end)
    {
        if (is_sub_string(dir_name, tarfs_iterator->name) == 0
            && str_compare1(last_matched_name, tarfs_iterator->name) != 0)
        {
            //Fail to match with prev name
            if (str_compare1(dir_name, last_matched_name) == 0 ||
                is_sub_string(last_matched_name, tarfs_iterator->name) != 0)
            {
                RunningTask->fd_array[fd].last_matched_header = (void *) tarfs_iterator;
                string_sub(tarfs_iterator->name, dir_name, buffer, '/');
//                kprintf("buffer value is %s",buffer);
                return 0;
            }

        }
        tarfs_iterator = get_next_tar_header(tarfs_iterator);
    }
    return 1;

}

int
close_dir(int fd)
{

    if (fd == 25)
    {

        return 0;
    }
    RunningTask->fd_array[fd].alloted = 0;
    RunningTask->fd_array[fd].file_sz = 0;
    RunningTask->fd_array[fd].num_bytes_read = 0;
    return 0;
}

void
fetch_cwd(char *output_buffer)
{
    str_copy(RunningTask->cwd, output_buffer);
}

void
set_cwd(char *input_buffer)
{
    char buff[100];
    path_sanitize(input_buffer, buff);
    if (buff[len(buff) - 1] != '/')
    {
        buff[len(buff)] = '/';
        buff[len(buff) + 1] = '\0';
    }
    if (buff[1] == '\0' && buff[0] == '/')
    {
        str_copy(buff, RunningTask->cwd);
    }
    else if (file_exists(&buff[1]) == 1)
    {
        str_copy(buff, RunningTask->cwd);
    }
    else
    {
        kprintf("invalid path\n");
    }
}

void
tarfs_test()
{
    //TEST 1 : FILE CALLS TEST
    int SIZE = 100;
    char buff[SIZE];
    RunningTask = (Task *) page_alloc();
    int fd;
    //FIXME:: should be done inside task
    initialise_fds();
    fd = open_s("test1/abc.txt", 1);
    memset((void *) buff, '\0', SIZE + 10);
    while (read_s(fd, buff, SIZE) != 0)
    {
        kprintf("%s", buff);
        memset((void *) buff, '\0', SIZE + 10);
    }
    close_s(fd);

    fd = open_s("test1/abc.tx", 1);
    memset((void *) buff, '\0', SIZE + 10);
    while (read_s(fd, buff, SIZE) != 0)
    {
        kprintf("%s", buff);
        memset((void *) buff, '\0', SIZE + 10);
    }
    close_s(fd);
    //*************************************************************************
    //TEST 2 : DIR CALLS TEST
    RunningTask = (Task *) page_alloc();
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
