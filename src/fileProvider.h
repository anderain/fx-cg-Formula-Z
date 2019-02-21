#ifndef _FILE_PROVIDER_H_
#   define _FILE_PROVIDER_H_
#   include "platform.h"
#   if defined(APP_MSVC)
#   elif defined(APP_FXCG)

typedef struct tag_FILE_INFO
{
    unsigned short  id;
    unsigned short  type;
    unsigned long   fsize;                  // File size
    unsigned long   dsize;                  // Data size
    unsigned int    property;               // The file has not been completed, except when property is 0.
    unsigned long   address;
} FILE_INFO;

#       define FONTCHARACTER                unsigned short
#       define _OPENMODE_READ               0x01
#       define _OPENMODE_READ_SHARE         0x80
#       define _OPENMODE_WRITE              0x02
#       define _OPENMODE_READWRITE          0x03
#       define _OPENMODE_READWRITE_SHARE    0x83
#       include <BFILE_syscalls.h>
#   endif
#endif

void * loadFile(const char *fileName);