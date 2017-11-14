//
// Created by mohan on 9/9/17.
//
#define OUTPUT_BUFFER_MAX_LENGTH 20480
#include <stdarg.h>
#include <stdio.h>
#include <sys/page.h>

int OUTPUT_BUFFER_LENGTH = 0;
int array_pointer = 0;
char OUTPUT_BUFFER[OUTPUT_BUFFER_MAX_LENGTH];
char TIME[OUTPUT_BUFFER_MAX_LENGTH];
//Prototypes
int
len(char *string);
void
reverse(char *string, int length);
void
str_copy(char *source_string, char *destination_string);
char
convert_num_to_char(int num);
int
num(int number, char *output_buffer, int base);
int
pointer(unsigned long number, char *output_buffer);
void
clear_global_array();
void
print_to_console();
void
print_key(int shift_flag,char c);
void
print_time(int time);


//Print Main Function
void
kprintf(const char *string_to_format, ...)
{
    va_list args;
    va_start(args, string_to_format);
    OUTPUT_BUFFER_LENGTH = 0;
    clear_global_array(OUTPUT_BUFFER);
    for (int i = 0; string_to_format[i] != '\0'; i++)
    {
        if (string_to_format[i] == '%' && string_to_format[i + 1] == 'c')
        {
            int char_buffer;
            char_buffer = va_arg(args,int);
            OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH] = (char)char_buffer;
            OUTPUT_BUFFER_LENGTH++;
            i++;
        }
        else if (string_to_format[i] == '%' && string_to_format[i + 1] == 'd')
        {
            int int_buffer, length_of_string;
            int_buffer = va_arg(args, int);
            length_of_string = num(int_buffer, &OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH], 10);
            OUTPUT_BUFFER_LENGTH += length_of_string;
            i++;
        }
        else if (string_to_format[i] == '%' && string_to_format[i + 1] == 'x')
        {
            unsigned int hex_buffer;
            int length_of_string;
            hex_buffer = va_arg(args, unsigned int);
            length_of_string = num(hex_buffer, &OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH], 16);
            OUTPUT_BUFFER_LENGTH += length_of_string;
            i++;
        }
        else if (string_to_format[i] == '%' && string_to_format[i + 1] == 's')
        {
            char *str_buffer;
            int string_length;
            str_buffer = va_arg(args, char* );
            string_length = len(str_buffer);
            str_copy(str_buffer, &OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH]);
            OUTPUT_BUFFER_LENGTH += string_length;
            i++;
        }
        else if (string_to_format[i] == '%' && string_to_format[i + 1] == 'p')
        {
            unsigned long ptr_buffer;
            int length_of_string;
            ptr_buffer = va_arg(args, unsigned long);
            length_of_string = pointer(ptr_buffer, &OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH]);
            OUTPUT_BUFFER_LENGTH += length_of_string;
            i++;
        }
        else
        {
            OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH] = string_to_format[i];
            OUTPUT_BUFFER_LENGTH++;
        }

    }
    OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH] = '\0';
    va_end(args);
    print_to_console();
}

void
clear_global_array(char* OUTPUT_BUFFER)
{
    for (int i = 0; i < OUTPUT_BUFFER_MAX_LENGTH; i++)
    {
        OUTPUT_BUFFER[i] = '\0';
    }
}

int
num(int number, char *output_buffer, int base)
{
    int n_flag = 0;
    if (number == 0)
    {
        output_buffer[0] = '0';
        return 2;
    }
    if (number < 0)
    {
        if (base == 16)
        {
            number -= 0xffffffff;
            number++;
        }
        else
        {
            n_flag = 1;
            number *=-1;
        }
    }
    int index = 0;
    int rem=0;
    for (index = 0;number!=0; index++)
    {
        rem = number % base;
        output_buffer[index] = convert_num_to_char(rem);
        number /= base;
    }
    if (n_flag == 1)
    {
        output_buffer[index] = '-';
        index++;
    }
    reverse(output_buffer, index);
    return index;
}

char
convert_num_to_char(int num)
{
    if (num <= 9)
    {
        return '0' + num;
    }
    else
    {
        return 'a' + (num-10);
    }
}

void
reverse(char *string, int length)
{
    int begin = 0;
    char tmp='\0';
    int end = length - 1;
    while (begin < end)
    {
        tmp = string[begin];
        string[begin] = string[end];
        string[end] = tmp;
        end--;
        begin++;
    }
}

void
str_copy(char *source_string, char *destination_string)
{
    int str_itr = 0;
    while (source_string[str_itr] != '\0')
    {
        destination_string[str_itr] = source_string[str_itr];
        str_itr++;
    }
    destination_string[str_itr] = '\0';
}

int
len(char *string)
{
    int string_length = 0;
    while (string[string_length] != '\0')
    {
        string_length++;
    }
    return string_length;
}

int
pointer(unsigned long number, char *output_buffer)
{

    if (number == 0)
    {
        output_buffer[0] = '0';
        output_buffer[1] = 'x';
        output_buffer[2] = '0';
        return 3;
    }
    int index = 0;
    int rem;
    for (index = 0;number!=0; index++)
    {
        rem = number%16;
        output_buffer[index] = convert_num_to_char(rem);
        number /= 16;
    }
    output_buffer[index] = 'x';
    index++;
    output_buffer[index] = '0';
    reverse(output_buffer, index+1);
    return index+1;
}

void print_key(int shift_flag,char c)
{
    register char *video_address;
    register char *base_address;
    base_address = (char *) get_viraddr(0xb8000);
    char str[14]="Key Entered: ";
    for (int i=0;i<14;i++)
    {
        video_address =base_address+24*160+80+i*2;
        *video_address = str[i];
    }

    if(shift_flag)
    {
        *(base_address+24*160+80+28) = '^';
        *(base_address+24*160+82+28) = c;
    }
    else
    {
        *(base_address+24*160+80+28) = c;
        *(base_address+24*160+82+28) = ' ';
    }

}

void print_time(int time)
{
    register char *video_address;
    register char *base_address;
    int array_pointer =0;
    int length;
    base_address = (char *) get_viraddr(0xb8000);
    clear_global_array(TIME);
    length = num(time, TIME, 10);
    char str[20]="Time Since Reboot: ";
    for (int i=0;i<20;i++)
    {
        video_address =base_address+24*160+i*2;
        *video_address = str[i];
     }
    for (int final_ary_itr = 0; final_ary_itr < length; final_ary_itr++)
    {
        video_address =base_address+24*160+40+array_pointer;
        *video_address = TIME[final_ary_itr];
        array_pointer += 2;
    }
}

void
print_to_console()
{
    register char *video_address;
    register char *base_address;
    base_address = (char *) get_viraddr(0xb8000);
    int  row = 0;
    //scroll
    if(array_pointer>=160*24)
    {
        for(array_pointer=160;array_pointer<=160*24;array_pointer+=2)
        {
            video_address =base_address+array_pointer;
            *(video_address-160)=*video_address;
        }
        array_pointer = 160*23;

        //clear last row
        for(int tmp=160*23;tmp<160*24;tmp++)
        {
            video_address =base_address+tmp;
            *video_address = ' ';
            tmp += 2;
        }
    }
    //print output buffer
    for (int final_ary_itr = 0; final_ary_itr < OUTPUT_BUFFER_LENGTH; final_ary_itr++)
    {
        if (OUTPUT_BUFFER[final_ary_itr] == '\n')
        {

            row = (array_pointer) / 160;
            array_pointer = 160*(row+1)-2;
            final_ary_itr++;
        }
        if (OUTPUT_BUFFER[final_ary_itr] == '\r')
        {
            row = (array_pointer) / 160;
            array_pointer = 160*(row);
            final_ary_itr++;
        }
        video_address =base_address+array_pointer;
        *video_address = OUTPUT_BUFFER[final_ary_itr];
        array_pointer += 2;
    }
}
