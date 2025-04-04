#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"
#include "common.h"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct DecodeInfo
{
    //source img info
    char *stego_fname;
    FILE *fptr_stego;
    char image_data[MAX_IMAGE_BUF_SIZE];

    //decoding file info
    char *decode_fname;
    FILE *fptr_decode;
    int extn_size;
    uint size_secretfile;
} DecodeInfo;

//Decoding function prototypes

//to check operation type
OperationType check_operation(char *argv[]);

//Read and validate Decode args from argv
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decodeInfo);

//perform the decoding
Status do_decoding(DecodeInfo *decodeInfo);

//Get file pointers for i/p and o/p
Status open_decode_files(DecodeInfo *decodeInfo);

//decode Magic str
Status decode_magic_string(DecodeInfo *decodeInfo);

//decode secret file extension size
Status decode_secret_file_extn_size(DecodeInfo *decodeInfo);

//Decode secret file extension
Status decode_secret_extn(DecodeInfo *decodeInfo);

//decode secret file Size
Status decode_secret_file_size(DecodeInfo *decodeInfo);

//decode secret file data
Status decode_secret_file_data(DecodeInfo *decodeInfo);

#endif //DECODE_H