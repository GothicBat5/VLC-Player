#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <windows.h>
#include "client/windows/handler/exception_handler.h"
#include "common/windows/http_upload.h"
#include <memory>
#include <map>
#include <string>
#include <sstream>

using google_breakpad::ExceptionHandler;

static bool FilterCallback(void*, EXCEPTION_POINTERS*, MDRawAssertionInfo*)
{
    return !IsDebuggerPresent();
}

extern "C"
{

#define WIDEN_(x) L ## x
#define WIDEN(x) WIDEN_(x)

void CheckCrashDump( const wchar_t* path )
{
    std::wstringstream pattern;
    WIN32_FIND_DATAW data;
    pattern << path << L"/*.dmp";
    HANDLE h = FindFirstFileW( pattern.str().c_str(), &data );
    if (h == INVALID_HANDLE_VALUE)
        return;
    int answer = MessageBoxW( NULL, L"Ooops: VLC media player just crashed.\n" \
        "Would you like to send a bug report to the developers team?",
        L"VLC crash reporting", MB_YESNO);
    std::map<std::wstring, std::wstring> params;
    params[L"prod"] = L"VLC";
    params[L"ver"] = WIDEN(PACKAGE_VERSION);
  
    do
    {
        std::wstringstream fullPath;
        fullPath << path << L'/' << data.cFileName;
        if( answer == IDYES )
        {
            std::map<std::wstring, std::wstring> files;
            files[L"upload_file_minidump"] = fullPath.str();
            google_breakpad::HTTPUpload::SendRequest(WIDEN( BREAKPAD_URL "/reports" ), params, files, NULL, NULL, NULL );
        }
        DeleteFileW( fullPath.str().c_str() );
    }
     while ( FindNextFileW( h, &data ) );
    FindClose(h);
}

void* InstallCrashHandler( const wchar_t* crashdump_path )
{
    // Breakpad needs the folder to exist to generate the crashdump
    CreateDirectoryW( crashdump_path, NULL );
    return new(std::nothrow) ExceptionHandler( crashdump_path, FilterCallback, NULL, NULL, ExceptionHandler::HANDLER_ALL);
}

void ReleaseCrashHandler( void* handler )
{
    ExceptionHandler* eh = reinterpret_cast<ExceptionHandler*>( handler );
    delete eh;
}

}
