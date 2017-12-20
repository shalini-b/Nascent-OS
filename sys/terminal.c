#include <sys/terminal.h>
#include <sys/kprintf.h>
#include <sys/process.h>
#include <sys/proc_mngr.h>
#define MIN(a, b)  (a<b)? a : b
#define MAX(a, b)  (a>b)? a : b

int r_buff_ptr = 0;
int w_buff_ptr = 0;
void
terminal_handler(char c)
{
    if (c == '\n')
    {
        terminal_line_count++;
        wake_up_task();
    }
    kprintf("%c", c);
    if (w_buff_ptr > BUFF_SIZE - 2)
    {
        w_buff_ptr = w_buff_ptr % BUFF_SIZE;
    }

    if(c!='\r')
    {
        term_buff[w_buff_ptr] = c;
        w_buff_ptr++;
//        kprintf("writ buff value %d",w_buff_ptr);

    }

}

void dec_term()
{
    term_buff[w_buff_ptr]='\0';
    w_buff_ptr--;
    term_buff[w_buff_ptr]='\0';
//    term_buff[w_buff_ptr]='\0';
//    w_buff_ptr--;
//    kprintf("writ buff value %d",w_buff_ptr);
}

int
read_buffer(char *buffer, int size)
{
    if (terminal_line_count != 0)
    {
        int i = 0;
        while (term_buff[r_buff_ptr] != '\n' && i < size - 1)
        {
            buffer[i] = term_buff[r_buff_ptr];
            i++;
            r_buff_ptr++;
            r_buff_ptr = r_buff_ptr % BUFF_SIZE;
        }
        r_buff_ptr++;//read after new line next time
        buffer[i + 1] = '\0';
        terminal_line_count--;
        return 0;
    }
    else
    {
        return 1;
    }

}

int
schedule_terminal_task(char *buffer,int size)
{
    while (1)
    {
        if (terminal_line_count != 0)
        {
            return read_buffer(buffer, size);
        }
        else
        {
            RunningTask->task_state = WAIT;
            schedule();
        }

    }
}

