#include <sys/terminal.h>
#include <sys/kprintf.h>

char term_buff[BUFF_SIZE];
int terminal_line_count;
void  terminal_handler(char c)
{
    if(c=='\n')
    {
        terminal_line_count++;
    }
    kprintf("%c",c);
}

void terminal_test()
{

}