#ifndef _TERM_H
#define _TERM_H
#define BUFF_SIZE  2048
char term_buff[BUFF_SIZE];
int terminal_line_count;
int r_buff_ptr;
int w_buff_ptr ;

void  terminal_handler(char c);
int
read_buffer(char *buffer, int size);
int
schedule_terminal_task(char *buffer,int size);
void dec_term();
#endif