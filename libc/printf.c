//
// Created by MOHANGANDHI on 04-12-2017.
//

#define OUTPUT_BUFFER_MAX_LENGTH 200
#include <stdarg.h>
#include <stdio.h>
#include <sys/page.h>
#include <strings.h>
#include<syscalls.h>
#include<printf.h>
int OUTPUT_BUFFER_LENGTH = 0;
static  char OUTPUT_BUFFER[OUTPUT_BUFFER_MAX_LENGTH];
int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);
char *gets(char *s);





//Print Main Function
int
printf(const char *string_to_format, ...)
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
            char_buffer = va_arg(args, int);
            OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH] = (char) char_buffer;
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
            hex_buffer = va_arg(args, unsigned
                int);
            length_of_string = num(hex_buffer, &OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH], 16);
            OUTPUT_BUFFER_LENGTH += length_of_string;
            i++;
        }
        else if (string_to_format[i] == '%' && string_to_format[i + 1] == 's')
        {
            char *str_buffer;
            int string_length;
            str_buffer = va_arg(args, char*);
            string_length = len(str_buffer);
            str_copy(str_buffer, &OUTPUT_BUFFER[OUTPUT_BUFFER_LENGTH]);
            OUTPUT_BUFFER_LENGTH += string_length;
            i++;
        }
        else if (string_to_format[i] == '%' && string_to_format[i + 1] == 'p')
        {
            unsigned long ptr_buffer;
            int length_of_string;
            ptr_buffer = va_arg(args, unsigned
                long);
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
    write(1, OUTPUT_BUFFER, OUTPUT_BUFFER_LENGTH + 1);
    return 0;
}

void
clear_global_array(char *OUTPUT_BUFFER)
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
            number *= -1;
        }
    }
    int index = 0;
    int rem = 0;
    for (index = 0; number != 0; index++)
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
        return 'a' + (num - 10);
    }
}

void
reverse(char *string, int length)
{
    int begin = 0;
    char tmp = '\0';
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
    for (index = 0; number != 0; index++)
    {
        rem = number % 16;
        output_buffer[index] = convert_num_to_char(rem);
        number /= 16;
    }
    output_buffer[index] = 'x';
    index++;
    output_buffer[index] = '0';
    reverse(output_buffer, index + 1);
    return index + 1;
}

