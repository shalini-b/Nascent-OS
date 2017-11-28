/* required functionality : open, read, close, opendir, readdir, closedir */
#include <sys/types.h>
#include<sys/tarfs.h>
#include <sys/kprintf.h>
#include <strings.h>
#include<sys/elf64.h>

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


int file_exists(char* f_name)
{
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar*) &_binary_tarfs_start;
    while(1)
    {
        if (tarfs_iterator->name[0] == '\0'||(tarfs_iterator < (struct posix_header_ustar*) &_binary_tarfs_end))
            break;

        if(str_compare1(tarfs_iterator->name,f_name)==0)
        {
            return 1;
        }
        tarfs_iterator = get_next_tar_header(tarfs_iterator);
    }
    return -1;

}


uint64_t print_elf_file(char* binary_name)
{
    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar*) &_binary_tarfs_start;
    while(tarfs_iterator < (struct posix_header_ustar*) &_binary_tarfs_end)
    {
        if(str_compare(tarfs_iterator->name,binary_name)==0)
        {
            struct Elf64_Ehdr* elf_header = (struct Elf64_Ehdr*)((uint64_t)tarfs_iterator + 512);
            elf_read(elf_header);
            return(uint64_t) elf_header->e_entry;
        }
        tarfs_iterator = get_next_tar_header(tarfs_iterator);
    }
    return 0;

}
//void* open(char* d_name)
//{
//    struct posix_header_ustar *tarfs_iterator = (struct posix_header_ustar*) &_binary_tarfs_start;
//    while(1)
//    {
//        if (tarfs_iterator->name[0] == '\0'||(tarfs_iterator > (struct posix_header_ustar*) &_binary_tarfs_end))
//            break;
//
//        if(str_compare(tarfs_iterator->name,d_name)==0&&strcmp(tarfs_iterator->typeflag, "5")==0)
//        {
//            /*
//
//             kmalloc logic;
//             */
//            return (void*)tarfs_iterator;
//        }
//        tarfs_iterator = get_next_tar_header(tarfs_iterator);
//    }
//    return  NULL;
//
//}
//
//
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

