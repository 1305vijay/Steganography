#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width storing into width var (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height and store into height var (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3; //3 is color combinations from 3 colours RGB
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
//calling func to validate the user sent files are mandatory files or not
//for encoding beautiful.bmp(or anyname.bmp) and secret.txt are mandatory files to sent
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //checking the argv having ".bmp" file after "."
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        encInfo ->src_image_fname = argv[2];
    }
    else 
    {
        return e_failure; //returns error if argv contains no ".bmp" file
    }
    //checking the argv having "secretfile.txt"
     if (argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
    {
        encInfo ->secret_fname = argv[3];
    }
    else 
    {
        return e_failure; //returns error if argv contains no ".txt" file
    }
    //o/p image file creation if user gives other .bmp files
    if(argv[4] != NULL)
    {
        encInfo ->stego_image_fname = argv[4]; //creating the user given file as argv
    }
    else //if no file was given then create a .bmp file 
    {
        encInfo ->stego_image_fname = "stego.bmp";
    }
    return e_success; 
}
//funct to get .txt file size
uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret, 0, SEEK_END);
    return ftell(fptr_secret);
}

Status check_capacity(EncodeInfo *encInfo)
{
    //size of img
    encInfo ->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    //size of txt
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    if(encInfo->image_capacity > (54 + (2 + 4 + 4 + 4 + encInfo->size_secret_file)* 8) )
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
Status copy_bmp_header(FILE *fptr_src, FILE *fptr_stego)  //funct to copy header files to stego.bmp
{
    char header[54];
    fseek(fptr_src, 0, SEEK_SET); //can use rewind(); to reset the pos of fptr
    fread(header, sizeof(char), 54, fptr_src);  //copying 54 bytes from src file
    fwrite(header, sizeof(char), 54, fptr_stego); //writing those 54 bytes to stego_file
    return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned mask = 1 << 7;
    for(int i=0; i<8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7-i));
        mask = mask >> 1;
    }
    return e_success;
}
Status encode_data_to_image(const char *data, int size, FILE *fptr_src, FILE *fptr_stego, EncodeInfo *encInfo)
{
    //encode_byte to lsb to encode the char
    for(int i=0; i<size; i++)
    {
        //read 8 bytes from src file
        fread(encInfo->image_data, 8, sizeof(char), fptr_src);
        //call encode_byte_to_lsb to encode the data
        encode_byte_to_lsb(data[i], encInfo->image_data);
        //write the encoded data to stego
        fwrite(encInfo->image_data, 8, sizeof(char), fptr_stego);
    }
    return e_success;
}
Status encode_magic_string(const char *magic_str, EncodeInfo *encInfo )
{
    //call encode_data_to_image funct to encode every char into img
    encode_data_to_image(magic_str, strlen(magic_str), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}
Status encode_size_to_lsb(char *image_buffer, int size)
{
    unsigned int mask = 1 << 31;
    for(int i=0; i<32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31-i));
        mask = mask >> 1;
    }
    return e_success;
}
Status encode_size(int size, FILE *fptr_src, FILE *fptr_stego)
{
    char rgb[32]; //encode 4 byte
    fread(rgb, 32, sizeof(char), fptr_src);
    encode_size_to_lsb(rgb, size);
    fwrite(rgb, 32, sizeof(char), fptr_stego);
    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    file_extn = ".txt";
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}
Status encode_secret_file_size(long int file_size, EncodeInfo *encInfo)
{
    char rgb[32]; //encode 4 byte
    fread(rgb, 32, sizeof(char), encInfo->fptr_src_image);
    encode_size_to_lsb(rgb, file_size);
    fwrite(rgb, 32, sizeof(char), encInfo->fptr_stego_image);
    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    for(int i=0; i<encInfo->size_secret_file; i++)
    {
        fread(encInfo->image_data, 8, sizeof(char), encInfo->fptr_src_image);
        fread(&ch, sizeof(char), 1, encInfo->fptr_secret);
        encode_byte_to_lsb(ch, encInfo->image_data);
        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image);
    }
    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_stego)
{
    char ch;
    while(fread(&ch, 1, 1 , fptr_src) > 0)
    {
        fwrite(&ch, 1, 1, fptr_stego);
    }
    return e_success;
}
//calling encoding funct
Status do_encoding(EncodeInfo *encInfo)
{
    //all the remaining encode funct need to be done here
    if(open_files(encInfo) == e_success)
    {
        printf("Opened all the required files successfully\n");
        if(check_capacity(encInfo) == e_success)
        {
            printf("secret data can be encoded\n");
            if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("Header copied successfully\n");
                if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("MAGIC STRING encoded successfully\n");
                    if(encode_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("Encoded secret file extension size successfully\n");
                        if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("Encoded secret file extension successfully\n");
                            if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Encoded secret file size successfully\n");
                                if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Encoded secret data successfully\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("Copied remaining bytes successfully\n");
                                        //hello brother

                                    }
                                    else
                                    {
                                        printf("Failed to copy the remaining bytes\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failed to encode the secret data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed to encode the file size\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("Failed to encode the secret file extension\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to encode secret file extension size\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed to encode magic string\n");
                    return e_failure;
                }
            }
            else 
            {
                printf("Header is failed to copy");
                return e_failure;
            }
        }
        else
        {
            printf("Encoding is not possible with the image\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open the files\n");
        return e_failure;
    }
    return e_success;
}
