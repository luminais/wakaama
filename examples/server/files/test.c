#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define COAP_SERVER_FILES_PATH "examples/server/coap_files"

#define MAX_SIZE (512)

#define MAX_TIMES (7)
int g_cnt = 0;

char g_buffer[MAX_SIZE] = {0};

#if 1
int read_file_block(char *file, off_t offset, int read_block, void *buff, char *block_more)
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
#if 0
    g_cnt++;
    if(g_cnt>MAX_TIMES)
        *block_more = 0;
#endif

    return read_total;
}
#else
int read_file_block(char *file, off_t offset, int read_block, void *buff, char *block_more)
{
    struct stat s;
    FILE *fp = NULL;
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
        printf("[%s][%d] stat %s failed\\n", __FUNCTION__, __LINE__, file);
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

    fp = fopen(file, "r");
    if(NULL == fp)
    {
        printf("[%s][%d] open %s failed(%m)\n", __FUNCTION__, __LINE__, file);
        return -1;
    }

    if(fseek(fp, offset, SEEK_SET) < 0)
    {
        fclose(fp);
        return -1;
    }

    do
    {
        read_size = fread(buff, need_read, 1, fp);
        if(read_size < 0)
        {
            printf("[%s][%d] fread failed\n", __FUNCTION__, __LINE__);
            return -1;
        }
        read_total += read_size;
        need_read -= read_size;
    }while(read_total<read_block && !(feof(fp)));

    fclose(fp);
    if(feof(fp))
        printf("[%s][%d] end of file\n", __FUNCTION__, __LINE__);

    len = offset + read_total;
    if(len <= total_size)
        *block_more = 1;
    else
        *block_more = 0;

    return read_total;
}
#endif
#if 1
int main(int argc, char const *argv[])
{
    char *char_p;
    int ret;
    char block_more = 0;
    off_t off = 0;

#if 0
    char_p = getcwd(NULL, 0);
    if(NULL == char_p)
    {
        printf("getcwd failed\n");
        return -1;
    }
    else
    {
        printf("path : %s\n", char_p);
        printf("len : %d\n", strlen(char_p));
        free(char_p);
    }
#endif

    do
    {
#if 1
        ret = read_file_block(COAP_SERVER_FILES_PATH"/agenttiny.bin", off, MAX_SIZE, g_buffer, &block_more);
#else
        ret = read_file_block(COAP_SERVER_FILES_PATH"/excel_sort_0.gif", off, MAX_SIZE, g_buffer, &block_more);
#endif
        printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
        if(ret < 0)
            break;

        off += ret;
        
    }while(1 == block_more);

    return 0;
}
#else
#define MAXBUFSIZE 1024 
int main ( int argc, char * argv[] ) { 
    char buf[ MAXBUFSIZE ]; 
    int count; 
    count = readlink( "/proc/self/exe", buf, MAXBUFSIZE ); 
    if ( count < 0 || count >= MAXBUFSIZE ) { 
    printf( "Failed\n" ); 
    return( EXIT_FAILURE ); 
    } 
 
    buf[ count ] = '\0'; 
    printf( "/proc/self/exe -> [%s]\n", buf ); 
    return( EXIT_SUCCESS ); 
}
#endif
