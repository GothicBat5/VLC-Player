#ifndef VLC_ACCESS_H
#define VLC_ACCESS_H 1

#include <vlc_stream.h>
#define VLC_ACCESS_REDIRECT VLC_ETIMEOUT

VLC_API stream_t *vlc_access_NewMRL(vlc_object_t *obj, const char *mrl);
VLC_API int access_vaDirectoryControlHelper( stream_t *p_access, int i_query, va_list args );

#define ACCESS_SET_CALLBACKS( read, block, control, seek )
    do { 
        p_access->pf_read = (read); 
        p_access->pf_block = (block); 
        p_access->pf_control = (control); 
        p_access->pf_seek = (seek); 
        p_access->pf_demux = NULL; 
    } while(0)

#endif
