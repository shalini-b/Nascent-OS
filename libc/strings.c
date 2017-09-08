#include <stdio.h>
#include <unistd.h>
#include <syscalls.h>
#include <strings.h>

//Length of string
int
len(char string[])
{   
    int string_length = 0;
    while (string[string_length] != '\0')
    {   
        string_length++;
    }
    return string_length;
}

int
str_compare(char s1[150], char s2[150])
{   
    int str_itr = 0;
    if (len(s1) != len(s2))
        return 1;
    
    while (s1[str_itr] != '\0')
    {
        if (s1[str_itr] != s2[str_itr])
        {
            return 1;
        }
        str_itr++;
    }
    return 0;
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

//Split Input String: returns count of given delimiter
//FIXME : can only split by single delimiter character
int
split_and_count(char *string_to_split, char delimiter, char final_array[][150])
{

    int string_itr = 0;
    int fin_arr_itr = 0;
    int block = 0;
    int found_flag = 0;
    int letter_flag = 0;
    if (!delimiter)
    {
        // FIXME: y do this?
        final_array = NULL;
    }
    // Parse till end of input
    while (string_to_split[string_itr] != '\0')
    {
        if (string_to_split[string_itr] != ' ')
        {
            // flag for marking chars 
            letter_flag = 1;
        }
        if (string_to_split[string_itr] == delimiter && letter_flag)
        {
            // flag for marking delimiter
            found_flag = 1;
        }
        if (found_flag && string_to_split[string_itr] != delimiter)
        {
            // deimiter found in prev iteration
            final_array[block][fin_arr_itr] = '\0';
            fin_arr_itr = 0;
            block++;
            found_flag = 0;
        }
        if (!found_flag && letter_flag && string_to_split[string_itr] != '\n')
        {
            // no delimiter found while parsing through chars. FIXME: y check for new line?
            final_array[block][fin_arr_itr] = string_to_split[string_itr];
            fin_arr_itr++;
        }
        string_itr++;

    }
    final_array[block][fin_arr_itr] = '\0';

    return block;
}

