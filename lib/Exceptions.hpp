#pragma once

#include "Logger.hpp"
#include "StringUtils.hpp"

#define LAUNCHER "cccaster\\launcher.exe"
#define HOOK_DLL "cccaster\\hook.release.dll"


struct Exception
{
    std::string debug, user;

    Exception() {}
    Exception ( const std::string& debug, const std::string& user )
        : debug ( debug ), user ( user.empty() ? debug : user ) {}

    virtual std::string str() const;
};


struct WinException : public Exception
{
    int code = 0;
    std::string desc;

    WinException() {}
    WinException ( int code, const std::string& debug, const std::string& user )
        : Exception ( debug, user ), code ( code ), desc ( getAsString ( code ) ) {}

    std::string str() const override;

    static std::string getAsString ( int windowsErrorCode );

    static std::string getLastError();

    static std::string getLastSocketError();
};


inline std::ostream& operator<< ( std::ostream& os, const Exception& exception )
{
    return ( os << exception.str() );
}


#define THROW_EXCEPTION(DEBUG, USER, ...)                                                       \
    do {                                                                                        \
        Exception exc ( format ( DEBUG, ## __VA_ARGS__ ), USER );                               \
        LOG ( "%s", exc );                                                                      \
        throw exc;                                                                              \
    } while ( 0 )


#define THROW_WIN_EXCEPTION(CODE, DEBUG, USER, ...)                                             \
    do {                                                                                        \
        WinException exc ( CODE, format ( DEBUG, ## __VA_ARGS__ ), USER );                      \
        LOG ( "%s", exc );                                                                      \
        throw exc;                                                                              \
    } while ( 0 )
