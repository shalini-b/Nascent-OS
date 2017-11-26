#ifndef _STRINGS_H
#define _STRINGS_H

int len(char string[]);
int str_compare1(char s1[150], char s2[150]);
int str_compare(char s1[150], char s2[150]);
void str_copy(char *source_string, char *destination_string);
int split_and_count(char *string_to_split, char delimiter, char final_array[][150]);

#endif
