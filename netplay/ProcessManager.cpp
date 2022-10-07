#include "ProcessManager.hpp"
#include "TcpSocket.hpp"
#include "Messages.hpp"
#include "Constants.hpp"
#include "EventManager.hpp"
#include "Exceptions.hpp"
#include "ErrorStringsExt.hpp"

#include <windows.h>
#include <direct.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory>

using namespace std;

#include <defines.hpp>

#define GAME_START_INTERVAL     ( 2000 )

#define GAME_START_ATTEMPTS     ( 30 )

#define PIPE_CONNECT_TIMEOUT    ( 60000 )

#define CC_KEY_CONFIG           "System\\_App.ini"


string ProcessManager::gameDir;

string ProcessManager::appDir;


ProcessManager::ProcessManager ( Owner *owner ) : owner ( owner ) {}

ProcessManager::~ProcessManager()
{
    disconnectPipe();
}

void ProcessManager::socketAccepted ( Socket *serverSocket )
{
    ASSERT ( serverSocket == _ipcSocket.get() );
    ASSERT ( serverSocket->isServer() == true );

    _ipcSocket = serverSocket->accept ( this );

    LOG ( "ipcSocket=%08x", _ipcSocket.get() );

    ASSERT ( _ipcSocket->address.addr == "127.0.0.1" );

    _ipcSocket->send ( new IpcConnected() );
}

void ProcessManager::socketConnected ( Socket *socket )
{
    ASSERT ( socket == _ipcSocket.get() );
    ASSERT ( _ipcSocket->address.addr == "127.0.0.1" );

    _ipcSocket->send ( new IpcConnected() );
}

void ProcessManager::socketDisconnected ( Socket *socket )
{
    ASSERT ( socket == _ipcSocket.get() );

    disconnectPipe();

    LOG ( "IPC disconnected" );

    if ( owner )
        owner->ipcDisconnected();
}

void ProcessManager::socketRead ( Socket *socket, const MsgPtr& msg, const IpAddrPort& address )
{
    ASSERT ( socket == _ipcSocket.get() );
    ASSERT ( address.addr == "127.0.0.1" );

    if ( msg && msg->getMsgType() == MsgType::IpcConnected )
    {
        ASSERT ( _connected == false );

        _connected = true;
        _gameStartTimer.reset();

        if ( owner )
            owner->ipcConnected();
        return;
    }

    owner->ipcRead ( msg );
}

void ProcessManager::timerExpired ( Timer *timer )
{
    ASSERT ( timer == _gameStartTimer.get() );

    if ( _gameStartCount >= GAME_START_ATTEMPTS && !isConnected() )
    {
        disconnectPipe();

        LOG ( "Failed to start game" );

        if ( owner )
            owner->ipcDisconnected();
        return;
    }

    _gameStartTimer->start ( GAME_START_INTERVAL );
    ++_gameStartCount;

    LOG ( "Trying to start game (%d)", _gameStartCount );

    void *hwnd = 0;
    if ( ! ( hwnd = findWindow ( CC_STARTUP_TITLE, false ) ) )
        return;

    if ( ! ( hwnd = FindWindowEx ( ( HWND ) hwnd, 0, 0, CC_STARTUP_BUTTON ) ) )
        return;

    if ( ! PostMessage ( ( HWND ) hwnd, BM_CLICK, 0, 0 ) )
        return;
}

void ProcessManager::openGame ( bool highPriority, bool isTraining )
{
    LOG ( "Opening pipe" );

    _pipe = CreateNamedPipe (
                NAMED_PIPE,                                          // name of the pipe
                PIPE_ACCESS_DUPLEX,                                  // 2-way pipe
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,     // byte stream + blocking
                1,                                                   // only allow 1 instance of this pipe
                1024,                                                // outbound buffer size
                1024,                                                // inbound buffer size
                0,                                                   // use default wait time
                0 );                                                 // use default security attributes

    if ( _pipe == INVALID_HANDLE_VALUE ){
        _pipe = CreateNamedPipe (
                NAMED_PIPE2,                                         // name of the pipe
                PIPE_ACCESS_DUPLEX,                                  // 2-way pipe
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,     // byte stream + blocking
                1,                                                   // only allow 1 instance of this pipe
                1024,                                                // outbound buffer size
                1024,                                                // inbound buffer size
                0,                                                   // use default wait time
                0 );                                                 // use default security attributes
    }
    if ( _pipe == INVALID_HANDLE_VALUE )
        THROW_WIN_EXCEPTION ( GetLastError(), "CreateNamedPipe failed", ERROR_PIPE_OPEN );

    LOG ( "appDir='%s'", appDir );
    LOG ( "gameDir='%s'", gameDir );

    string path = appDir + LAUNCHER;
    vector<string> stringArgs;
    stringArgs.push_back ( "\"" + path + "\"" );
    stringArgs.push_back ( "\"" + gameDir + MBAA_EXE + "\"" );
    stringArgs.push_back ( "\"" + appDir + HOOK_DLL + "\"" );
    stringArgs.push_back ( "\"" + appDir + "framestep.dll" + "\"" );
    if ( isTraining )
        stringArgs.push_back ( "--framestep" );
    if ( highPriority )
        stringArgs.push_back ( "--high" );

#ifndef DISABLE_LOGGING
    stringArgs.push_back ( "--popup_errors" );
#endif

    unique_ptr<const char *> argsPtr ( new const char *[stringArgs.size() + 1] );
    auto *args = argsPtr.get();
    for ( size_t i = 0; i < stringArgs.size(); i++ )
        args[i] = stringArgs[i].c_str();
    args[stringArgs.size()] = NULL;

    intptr_t returnCode = _spawnv ( _P_DETACH, path.c_str(), args );
    if ( returnCode < 0 )
        THROW_EXCEPTION ( "errno=%d", ERROR_PIPE_START, errno );

    LOG ( "Connecting pipe" );

    struct TimeoutThread : public Thread
    {
        void run() override
        {
            Sleep ( PIPE_CONNECT_TIMEOUT );

            HANDLE tmpPipe = CreateFile ( NAMED_PIPE, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                          0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

            HANDLE tmpPipe2 = CreateFile ( NAMED_PIPE2, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                          0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );


            if ( tmpPipe == INVALID_HANDLE_VALUE )
                return;

            if ( tmpPipe2 == INVALID_HANDLE_VALUE )
                return;

            CloseHandle ( tmpPipe );
            CloseHandle ( tmpPipe2 );
        }
    };

    ThreadPtr thread ( new TimeoutThread() );
    thread->start();
    EventManager::get().addThread ( thread );

    if ( ! ConnectNamedPipe ( _pipe, 0 ) )
    {
        int error = GetLastError();

        if ( error != ERROR_PIPE_CONNECTED )
            THROW_WIN_EXCEPTION ( GetLastError(), "ConnectNamedPipe failed", ERROR_PIPE_START );
    }

    LOG ( "Pipe connected" );

    DWORD bytes;
    IpAddrPort ipcHost ( "127.0.0.1", 0 );

    if ( ! ReadFile ( _pipe, &ipcHost.port, sizeof ( ipcHost.port ), &bytes, 0 ) )
        THROW_WIN_EXCEPTION ( GetLastError(), "ReadFile failed", ERROR_PIPE_RW );

    if ( bytes != sizeof ( ipcHost.port ) )
        THROW_EXCEPTION ( "read %d bytes, expected %d", ERROR_PIPE_RW, bytes, sizeof ( ipcHost.port ) );

    LOG ( "ipcHost='%s'", ipcHost );

    _ipcSocket = TcpSocket::connect ( this, ipcHost );

    LOG ( "ipcSocket=%08x", _ipcSocket.get() );

    if ( ! ReadFile ( _pipe, &_processId, sizeof ( _processId ), &bytes, 0 ) )
        THROW_WIN_EXCEPTION ( GetLastError(), "ReadFile failed", ERROR_PIPE_RW );

    if ( bytes != sizeof ( _processId ) )
        THROW_EXCEPTION ( "read %d bytes, expected %d", ERROR_PIPE_RW, bytes, sizeof ( _processId ) );

    LOG ( "processId=%08x", _processId );

    _gameStartTimer.reset ( new Timer ( this ) );
    _gameStartTimer->start ( GAME_START_INTERVAL );
    _gameStartCount = 0;
}

void ProcessManager::closeGame()
{
    if ( ! isConnected() )
        return;

    disconnectPipe();

    LOG ( "Closing game" );

    // Find and close any lingering windows
    for ( const string& window : { CC_TITLE, CC_STARTUP_TITLE } )
    {
        void *hwnd;
        if ( ( hwnd = findWindow ( window, false ) ) )
            PostMessage ( ( HWND ) hwnd, WM_CLOSE, 0, 0 );
    }
}

void ProcessManager::disconnectPipe()
{
    _gameStartTimer.reset();
    _ipcSocket.reset();

    if ( _pipe )
    {
        CloseHandle ( ( HANDLE ) _pipe );
        _pipe = 0;
    }

    _connected = false;
}

bool ProcessManager::getIsWindowed()
{
    const string file = gameDir + CC_APP_CONFIG_FILE;

    LOG ( "Reading: %s", file );

    string line, total;
    ifstream fin ( file );

    while ( getline ( fin, line ) )
    {
        vector<string> parts = split ( line, "=" );

        if ( parts.size() != 2 || parts[0] != CC_APP_WINDOW_MODE_KEY )
            continue;

        return lexical_cast<int> ( parts[1] );
    }

    return false;
}

void ProcessManager::setIsWindowed ( bool enabled )
{
    const string file = gameDir + CC_APP_CONFIG_FILE;

    LOG ( "Reading: %s", file );

    string line, total;
    ifstream fin ( file );

    while ( getline ( fin, line ) )
    {
        vector<string> parts = split ( line, "=" );

        if ( parts.size() != 2 || parts[0] != CC_APP_WINDOW_MODE_KEY )
        {
            total += line + "\n";
            continue;
        }

        total += format ( "%s= %u\n", CC_APP_WINDOW_MODE_KEY, enabled );
    }

    fin.close();

    LOG ( "Writing: %s", file );

    ofstream fout ( file );
    fout.write ( &total[0], total.size() );
    fout.close();
}

string ProcessManager::fetchGameUserName()
{
    const string file = gameDir + CC_NETWORK_CONFIG_FILE;

    LOG ( "Reading: %s", file );

    string line, buffer;
    ifstream fin ( file );

    while ( getline ( fin, line ) )
    {
        buffer.clear();

        if ( line.substr ( 0, sizeof ( CC_NETWORK_USERNAME_KEY ) - 1 ) == CC_NETWORK_USERNAME_KEY )
        {
            // Find opening quote
            size_t pos = line.find ( '"' );
            if ( pos == string::npos )
                break;

            buffer = line.substr ( pos + 1 );

            // Find closing quote
            pos = buffer.rfind ( '"' );
            if ( pos == string::npos )
                break;

            buffer.erase ( pos );
            break;
        }
    }

    fin.close();
    return buffer;
}

array<char, 10> ProcessManager::fetchKeyboardConfig()
{
    const string file = gameDir + MBAA_EXE;

    LOG ( "Reading: %s", file );

    array<char, 10> config;
    for ( char& c : config )
        c = 0;

    ifstream fin ( file, ios::binary );

    if ( fin.good() )
        fin.seekg ( CC_KEYBOARD_CONFIG_OFFSET );

    if ( fin.good() )
        fin.read ( &config[0], config.size() );

    fin.close();

    return config;
}

void *ProcessManager::findWindow ( const string& title, bool exact )
{
    static string tmpTitle;
    static HWND tmpHwnd;
    static bool tmpExact;

    tmpTitle = title;
    tmpHwnd = 0;
    tmpExact = exact;

    struct _
    {
        static BOOL CALLBACK enumWindowsProc ( HWND hwnd, LPARAM lParam )
        {
            if ( hwnd == 0 )
                return true;

            char buffer[4096];
            GetWindowText ( hwnd, buffer, sizeof ( buffer ) );

            if ( ! tmpHwnd  )
            {
                if ( tmpExact )
                {
                    if ( trimmed ( buffer ) == tmpTitle )
                        tmpHwnd = hwnd;
                }
                else
                {
                    if ( trimmed ( buffer ).find ( tmpTitle ) == 0 )
                        tmpHwnd = hwnd;
                }
            }

            return true;
        }
    };

    EnumWindows ( _::enumWindowsProc, 0 );

    return tmpHwnd;
}

bool ProcessManager::isWine()
{
    static char isWine = -1; // -1 means uninitialized

    if ( isWine >= 0 )
        return isWine;

    HMODULE ntdll = GetModuleHandle ( "ntdll.dll" );

    if ( ! ntdll )
    {
        isWine = 0;
        return isWine;
    }

    isWine = ( GetProcAddress ( ntdll, "wine_get_version" ) ? 1 : 0 );
    return isWine;
}


bool ProcessManager::isConnected() const
{
    return ( _pipe && _ipcSocket && _ipcSocket->isClient() && _connected );
}

bool ProcessManager::ipcSend ( Serializable& msg )
{
    return ipcSend ( MsgPtr ( &msg, ignoreMsgPtr ) );
}

bool ProcessManager::ipcSend ( Serializable *msg )
{
    return ipcSend ( MsgPtr ( msg ) );
}

bool ProcessManager::ipcSend ( const MsgPtr& msg )
{
    if ( ! isConnected() )
        return false;
    else
        return _ipcSocket->send ( msg );
}
