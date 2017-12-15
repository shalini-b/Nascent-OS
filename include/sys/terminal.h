#ifndef _TERM_H
#define _TERM_H


#define BUFF_SIZE  2048
void  terminal_handler(char c);
int
read_buffer(char *buffer, int size);
int
schedule_terminal_task(char *buffer,int size);
#endif