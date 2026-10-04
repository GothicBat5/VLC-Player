#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>

#define DECLARE_VLC_VERSION( func, var )  
\
const char * VLC_##func ( void )                                            \
{                                                                           \
    return VLC_##var ;                                                      \
}

DECLARE_VLC_VERSION( CompileBy, COMPILE_BY )
DECLARE_VLC_VERSION( CompileHost, COMPILE_HOST )
DECLARE_VLC_VERSION( Compiler, COMPILER )
