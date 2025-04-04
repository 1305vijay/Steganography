#include <string.h>
#include "common.h"
#include "types.h"
#include "decode.h"

//funct call to read and validate function
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decodeInfo)
{
    printf("started Arguement checking\n");
    if(strcmp(strstr(argv[2], "."), ".bmp") == 0) //checking argv has .bmp file
    {
        decodeInfo->stego_fname = argv[2]; //store the file name to src
    }
    else
    {
        printf("Error! .bmp file is not there\n");
        return e_failure;
    }

    //create a o/p file if user did not given any file
    if(argv[3] != NULL)
    {
        decodeInfo->decode_fname = argv[3];
    }
    else
    {
        strcpy(decodeInfo->decode_fname, "decode");
    }
    printf("Arguements verification has done successfully\n");
    return e_success;
}

//funct call to open file funct
Status open_decode_files(DecodeInfo *decodeInfo)
{
    printf("Opening Source File\n");
    decodeInfo->fptr_stego = fopen(decodeInfo->stego_fname, "r"); //src file in read only mode
    if(decodeInfo->fptr_stego == NULL)
    {
        printf("Error: image.bmp file is not found\n");
        return e_failure;
    }
    printf("Source File Opened Successfully\n");
    return e_success;
}

//funct call to Decode lsb to char 
char decode_lsb_to_char(DecodeInfo *decodeInfo)
{
    char buffer[8];
    //reading 8 bytes from src file to buffer file
    fread(buffer, 8, 1, decodeInfo->fptr_stego);
    char ch=0;
    //logic to get lsb from buffer
    //and then logic to decode to char
    for(int i=0; i<8; i++)
    {
        ch = ch | (buffer[i] & 1); //extracting each lsb 
        if(i != 7)
            ch = ch << 1; //left shifting char to store
    }
    return ch;
}

//funct call to decode Magic str
Status decode_magic_string(DecodeInfo *decodeInfo)
{
    int magic_strlen = strlen(MAGIC_STRING );
    fseek(decodeInfo->fptr_stego, 54, SEEK_SET);
    char magic_str[5];
    int i;
    for(i=0; i<magic_strlen; i++)
    {
        magic_str[i] = decode_lsb_to_char(decodeInfo);
    }
    magic_str[i] = '\0';

    if(strcmp(magic_str, MAGIC_STRING) == 0)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

//func to decode Lsb to int
int decode_lsb_to_int(DecodeInfo *decodeInfo)
{
    char buffer[32];
    int size = 0;
    fread(buffer, 32, 1, decodeInfo->fptr_stego);

    for(int i=0; i<32; i++)
    {
        size =size | (buffer[i] & 1);
        if(i !=31)
        {
            size = size << 1;
        }
    }
    return size;
}

//funct to decode secret file extension size 
Status decode_secret_file_extn_size(DecodeInfo *decodeInfo)
{
    decodeInfo->extn_size = decode_lsb_to_int(decodeInfo);
    return e_success;
}

//funct call to decode secret file ext 
Status decode_secret_extn(DecodeInfo *decodeInfo)
{
    char extension[decodeInfo->extn_size + 1]; //includes null
    for(int i=0; i<decodeInfo->extn_size; i++)
    {
        extension[i] = decode_lsb_to_char(decodeInfo);
    }
    extension[decodeInfo->extn_size] = '\0';

    //extracting base name file from file
    char *temp =strstr(decodeInfo->decode_fname, ".");
    if(temp != NULL)
    {
        int i=0;
        char str[100]; //to store the base file name
        while (decodeInfo->decode_fname[i] != '.')
        {
            str[i] = decodeInfo->decode_fname[i];
            i++;
        }
        str[i] = '\0';

        //copying extension to o/p file
        strcpy(decodeInfo->decode_fname, str);
    }
    strcat(decodeInfo->decode_fname, extension);

    decodeInfo->fptr_decode = fopen(decodeInfo->decode_fname, "w");
    if(decodeInfo->fptr_decode == NULL)
    {
        return e_failure;
    }
    return e_success;
}

//funct to decode secret file size
Status decode_secret_file_size(DecodeInfo *decodeInfo)
{
    decodeInfo->size_secretfile = decode_lsb_to_int(decodeInfo);
    return e_success;
}

//funct to decode secret file data
Status decode_secret_file_data(DecodeInfo *decodeInfo)
{
    char ch;
    for(int i=0; i<decodeInfo->size_secretfile; i++)
    {
        ch = decode_lsb_to_char(decodeInfo);
        fwrite(&ch, 1, 1, decodeInfo->fptr_decode);
    }
    return e_success;
}

//Main funct to decode file
Status do_decoding(DecodeInfo *decodeInfo)
{
    printf("Opening file process started.\n");
    if(open_decode_files(decodeInfo) == e_success)
    {
        printf("Files opened successfullyu\n");
        printf("Decoding Magic string now\n");
        if(decode_magic_string(decodeInfo) == e_success)
        {
            printf("Magic string decoded successfully.\n");
            printf("Decoding secret file extension size now\n");
            if(decode_secret_file_extn_size(decodeInfo) == e_success)
            {
                printf("Secret file extension size decoded successfully\n");
                printf("Decoding secret file extension now\n");
                if(decode_secret_extn(decodeInfo) == e_success)
                {
                    printf("Secret message file is %s\n", decodeInfo->decode_fname);
                    printf("Secret File opened successfully\n");
                    printf("Decoding secret file size now\n");
                    if(decode_secret_file_size(decodeInfo) == e_success)
                    {
                        printf("Secret file size decoded successfully.\n");
                        printf("Decoding secret data\n");
                        if(decode_secret_file_data(decodeInfo) == e_success)
                        {
                            printf("Secret data decoded successfully\n");
                            return e_success;
                        }
                        else
                        {
                            printf("Error: failed to decode secret file data\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Error: failed to decode secret file size\n");
                        return e_failure;    
                    }
                }
                else
                {
                    printf("Error: failed to decode secret extension\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to decode secret extension size\n");
                return e_failure;
            }
        }
        else
        {
            printf("Error: failed to decode Magic string\n");
            return e_failure;   
        }
    }
    else
    {
        printf("Error: Failed to open decoding files\n");
        return e_failure;
    }

}
