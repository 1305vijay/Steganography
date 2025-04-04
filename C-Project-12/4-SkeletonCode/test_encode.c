#include <stdio.h>
#include<string.h>
#include "encode.h"
#include"decode.h"
#include "types.h"

int main(int argc, char **argv)
{
    if(check_operation_type(argv) == e_encode){
        EncodeInfo encInfo;
        printf("Selected Encoding\n");
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Read and validate arguements is successful\n");
            if(do_encoding(&encInfo) == e_success)
            {
                printf("Completed Encoding\n");
            }
            else
            {
                printf("Failed to encode the secret data \n");
            }
        }
        else
        {
          printf("Failed to validate the arguments\n");  
        }
    }
    else if(check_operation_type(argv)== e_decode){
        puts("Selected Decoding\n");
        DecodeInfo decodeInfo;
        if(read_and_validate_decode_args(argv, &decodeInfo) == e_success)
        {
            if(do_decoding(&decodeInfo) == e_success)
            {
                printf("Decoding is completed\n");
            }
            else
            {
                printf("Decoding Failed\n");
            }
        }
    }
    else {
        printf("Invalid Options \n USAGE\n");
        printf("Encoding: ./a.out -e beautiful.bmp secret.txt\n");
        printf("Decoding: ./a.out -d stego.bmp\n");
    }
    return 0;
}
//checking operationtype using argv statement
//whether user asking to encoding or decoding (i.e -e or -d)
OperationType check_operation_type(char *argv[]){
    if(strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else{
        return e_unsupported;
    }
}