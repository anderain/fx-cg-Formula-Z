#include "fileProvider.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(APP_MSVC)

void* loadFile(const char *fileName) {
    FILE *fp;
    unsigned int length;
    unsigned char * buffer;
    fopen_s(&fp, fileName, "rb");
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer = (unsigned char *)malloc(length);
    fread(buffer, length, 1, fp);
    fclose(fp);
    return buffer;
}

#elif defined(APP_FXCG)

// #include "inputProvider.h"
// #include "display.h"
// #include "CONVERT_syscalls.h"

void * loadFile(const char *fileName) {
    int handle;
    unsigned int length;
    void *buffer;
    unsigned short fontFileName[100];

    Bfile_StrToName_ncpy(fontFileName, fileName, sizeof(fontFileName));

    handle = Bfile_OpenFile_OS(fontFileName, _OPENMODE_READ);
    
    if (handle < 0) {
        return 0;
    }

    length = Bfile_GetFileSize_OS(handle, 0);
    buffer = malloc(length);
    Bfile_ReadFile_OS(handle, buffer, length, 0);
    Bfile_CloseFile_OS(handle);
    {
        char str[100];
        itoa(buffer, str);
        Bdisp_AllClr_VRAM();
        locate_OS(2,2);
        Print_OS(str, 1, 0);
        wait_key();
    }
    return buffer;
}

#endif

