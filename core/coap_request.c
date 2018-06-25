#include "internals.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "coap_request.h"

#define COAP_SERVER_FILES_PATH "examples/server/files"

static int read_file_block(char *file, off_t offset, int read_block, void *buff, char *block_more)
{
    struct stat s;
    int fd = -1;
    off_t total_size = 0, len;
    int read_size, read_total = 0;
    int need_read = read_block;

    if(!file || offset < 0 || read_block <= 0 || !buff || !block_more)
    {
        printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
        return -1;
    }
    memset(&s, 0x0, sizeof(s));

    if(0 != stat(file, &s))
    {
        printf("[%s][%d] stat %s failed\n", __FUNCTION__, __LINE__, file);
        printf("[%s][%d] %d(%m)\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }
    printf("[%s][%d] offset : %ld\n", __FUNCTION__, __LINE__, offset);
    printf("[%s][%d] need_read : %d\n", __FUNCTION__, __LINE__, need_read);
    printf("[%s][%d] total size : %ld\n", __FUNCTION__, __LINE__, s.st_size);
    total_size = s.st_size;

    if(offset >= total_size)
    {
        return -1;
    }

    fd = open(file, O_RDONLY);
    if(-1 == fd)
    {
        printf("[%s][%d] open %s failed(%m)\n", __FUNCTION__, __LINE__, file);
        return -1;
    }

    if(lseek(fd, offset, SEEK_SET) < 0)
    {
        close(fd);
        return -1;
    }

    do
    {
        read_size = read(fd, buff, need_read);
        if(read_size < 0)
        {
            printf("[%s][%d] fread failed\n", __FUNCTION__, __LINE__);
            return -1;
        }
        if(0 == read_size)
        {
            printf("[%s][%d] read to end\n", __FUNCTION__, __LINE__);
            break;
        }
        read_total += read_size;
        need_read -= read_size;
        printf("[%s][%d] read_size = %d\n", __FUNCTION__, __LINE__, read_size);
        printf("[%s][%d] read_total = %d\n", __FUNCTION__, __LINE__, read_total);
        printf("[%s][%d] need_read = %d\n", __FUNCTION__, __LINE__, need_read);
    }while(read_total<read_block);

    close(fd);

    len = offset + read_total;
    if(len < total_size)
        *block_more = 1;
    else
        *block_more = 0;

    return read_total;
}

static int read_file_to_buff(char *file, void **buff)
{
    struct stat s;
    int fd = -1;
    off_t total_size = 0, len;
    int read_size, read_total = 0, need_read;
    void *temp_buf = NULL;

    if(!file|| *file == '\0' || !buff)
    {
        printf("[%s][%d] invalid params.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    memset(&s, 0x0, sizeof(s));

    if(0 != stat(file, &s))
    {
        printf("[%s][%d] stat %s failed\n", __FUNCTION__, __LINE__, file);
        printf("[%s][%d] %d(%m)\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }
    printf("[%s][%d] total size : %ld\n", __FUNCTION__, __LINE__, s.st_size);
    total_size = s.st_size;

    temp_buf = malloc(total_size);
    if(!temp_buf)
    {
        printf("[%s][%d] malloc failed: %d(%m)\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }

    fd = open(file, O_RDONLY);
    if(-1 == fd)
    {
        printf("[%s][%d] open %s failed(%m)\n", __FUNCTION__, __LINE__, file);
        return -1;
    }

    need_read = total_size;
    do
    {
        read_size = read(fd, temp_buf, need_read);
        if(read_size < 0)
        {
            printf("[%s][%d] fread failed\n", __FUNCTION__, __LINE__);
            close(fd);
            return -1;
        }
        if(0 == read_size)
        {
            printf("[%s][%d] read to end\n", __FUNCTION__, __LINE__);
            break;
        }
        read_total += read_size;
        need_read -= read_size;
        printf("[%s][%d] read_size = %d\n", __FUNCTION__, __LINE__, read_size);
        printf("[%s][%d] read_total = %d\n", __FUNCTION__, __LINE__, read_total);
        printf("[%s][%d] need_read = %d\n", __FUNCTION__, __LINE__, need_read);
    }while(read_total<total_size);

    close(fd);

    *buff = temp_buf;

    return read_total;
}

static int parse_uri_path_option(multi_option_t *uri_path, char **uri_buff)
{
    int uri_len = strlen(COAP_SERVER_FILES_PATH);
    char *char_p = NULL;
    multi_option_t *uri_path_P;
    if(!uri_path || !uri_buff)
    {
        printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
        return -1;
    }

    uri_path_P = uri_path;
    while(NULL != uri_path_P)
    {
        printf("[%s][%d] %.*s\n", __FUNCTION__, __LINE__, uri_path_P->len, uri_path_P->data);
        uri_len += uri_path_P->len;
        uri_path_P = uri_path_P->next;
    }

    char_p = (char *)malloc(uri_len);
    if(!char_p)
    {
        printf("[%s][%d] malloc failed.%d(%m)\n", __FUNCTION__, __LINE__, errno);
        return -1;
    }
    memset(char_p, 0x0, uri_len);

    strcpy(char_p, COAP_SERVER_FILES_PATH);
    uri_len = strlen(COAP_SERVER_FILES_PATH);

    uri_path_P = uri_path;
    while(NULL != uri_path_P)
    {
        memcpy(char_p+uri_len, "/", 1);
        uri_len += 1;
        memcpy(char_p+uri_len, uri_path_P->data, uri_path_P->len);
        uri_len += uri_path_P->len;
        uri_path_P = uri_path_P->next;
    }
    char_p[uri_len] = '\0';
    *uri_buff = char_p;

    return 0;
}

uint8_t coap_file_get(lwm2m_context_t * contextP,
                                   lwm2m_uri_t * uriP,
                                   void * fromSessionH,
                                   coap_packet_t * message,
                                   coap_packet_t * response)
{
    char *uri_buff = NULL;
    multi_option_t *uri_path = message->uri_path;
    int read_len = 0;

    if(0 != parse_uri_path_option(uri_path, &uri_buff))
    {
        printf("[%s][%d] parse_uri_path_option failed\n", __FUNCTION__, __LINE__);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    printf("[%s][%d] uri_buff : %s\n", __FUNCTION__, __LINE__, uri_buff);
    read_len = read_file_to_buff(uri_buff, (void **)(&(response->payload)));
    if(read_len < 0)
    {
        printf("[%s][%d] read_file_to_buff failed\n", __FUNCTION__, __LINE__);
        free(uri_buff);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }
    response->payload_len = (uint32_t)read_len;
    printf("[%s][%d] response->payload_len : %u\n", __FUNCTION__, __LINE__, response->payload_len);
    free(uri_buff);
    return COAP_NO_ERROR;
}

