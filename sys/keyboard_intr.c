//reference : http://www.cs.umd.edu/~hollings/cs412/s98/project/proj1/scancode

#include<sys/idt.h>
#include <sys/kprintf.h>
#include <sys/terminal.h>
void
keyboard_intr()
{
    int keyboard_value = inb(0x60);
    static int shift_flag =0,control_flag =0;
    char c;
    char asciicodes[58][2] =
        {
            {0, 0},
            {'E', 'E'},
            {'1', '!'},
            {'2', '@'},
            {'3', '#'},
            {'4', '$'},
            {'5', '%'},
            {'6', '^'},
            {'7', '&'},
            {'8', '*'},
            {'9', '('},
            {'0', ')'},
            {'-', '_'},
            {'=', '+'},
            {8, 8},
            {9, 9},
            {'q', 'Q'},
            {'w', 'W'},
            {'e', 'E'},
            {'r', 'R'},
            {'t', 'T'},
            {'y', 'Y'},
            {'u', 'U'},
            {'i', 'I'},
            {'o', 'O'},
            {'p', 'P'},
            {'[', '{'},
            {']', '}'},
            {13, 13},
            {0, 0},
            {'a', 'A'},
            {'s', 'S'},
            {'d', 'D'},
            {'f', 'F'},
            {'g', 'G'},
            {'h', 'H'},
            {'j', 'J'},
            {'k', 'K'},
            {'l', 'L'},
            {';', ':'},
            {39, 34},
            {'`', '~'},
            {0, 0},
            {'\\', '|'},
            {'z', 'Z'},
            {'x', 'X'},
            {'c', 'C'},
            {'v', 'V'},
            {'b', 'B'},
            {'n', 'N'},
            {'m', 'M'},
            {',', '<'},
            {'.', '>'},
            {'/', '?'},
            {0, 0},
            {0, 0},
            {0, 0},
            {' ', ' '},
        };
//    kprintf("%d\n",keyboard_value);
    if(keyboard_value==28)
    {
        terminal_handler('\n');
    }
    if(keyboard_value == 29)//cntrl
    {
        control_flag = 1;
    }
    else if(keyboard_value == 42)//shift
    {
        shift_flag =1;
    }
    else
    {
        if(keyboard_value<60)
        {
            if(shift_flag==1)
            {
                c = asciicodes[keyboard_value][1];
                shift_flag = 0;
                terminal_handler(c);
                print_key(control_flag,c);
                control_flag = 0;
            }
            else
            {
                c = asciicodes[keyboard_value][0];
                terminal_handler(c);
                print_key(control_flag,c);
                control_flag = 0;
            }
        }

    }
    outb(0x20, 0x20);
}





