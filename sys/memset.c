//void
//memset(void *initial_mem, int length,int value_memset)
//{
//    for(int i=0;i<length;i++)
//    {
//        initial_mem[i] = character;
//    }
//}


void  *memset(void *string_to_memset, int char_to_memset_with, int length_to_memset)
{
    unsigned char *temp_typecasted_string = string_to_memset;
    for(int str_itr = 0;str_itr<length_to_memset;str_itr++)
    {
        temp_typecasted_string[str_itr] = char_to_memset_with;
    }
    return(string_to_memset);
}
