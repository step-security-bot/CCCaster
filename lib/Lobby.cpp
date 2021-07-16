#include "Lobby.hpp"
#include "ConsoleUi.hpp"
#include "TcpSocket.hpp"
#include "Exceptions.hpp"
#include "EventManager.hpp"
#include "StringUtils.hpp"

using namespace std;

vector<string> Lobby::getMenu()
{
    return entries;
}

vector<string> Lobby::getIps(){
    return ips;
}

Lobby::Lobby( Owner* owner )
    : owner( owner )
{
    timeout = 5000;
    numEntries = 0;
    blankEntry = "";
    for( int i = 0; i < 43; ++i ) {
        blankEntry += " ";
    }
    blankEntry += "|   |Open   ";
    vector<string> menu;
    for( int i = 0; i < 20; ++i ) {
        menu.push_back( blankEntry );
        ips.push_back( blankEntry );
    }
    entries = menu;
    connectionSuccess = false;
}

bool Lobby::connect( std::string url )
{
    LOG ( "Connecting to: '%s'", url );

    url = _address.str();
    try
    {
        _socket = TcpSocket::connect ( this, url, true, timeout ); // Raw socket
    }
    catch ( ... )
    {
        _socket.reset();

        LOG ( "Failed to create socket!" );

        if ( owner )
            owner->connectionFailed ( this );
        return false;
    }
    return true;

}

void Lobby::disconnect()
{
    if ( owner )
        owner->connectionFailed(this);
}
void Lobby::socketConnected ( Socket *socket )
{
    ASSERT ( _socket.get() == socket );
    const string request = format ( "LIST,none" );
    LOG ( "Sending request:\n%s", request );

    _timer.reset ( new Timer ( this ) );
    _timer->start ( 3000 );

    if ( ! _socket->send ( &request[0], request.size() ) ) {
        LOG ( "Failed to send request!" );

        if ( owner )
            owner->connectionFailed ( this );
    }

}

void Lobby::socketDisconnected ( Socket *socket )
{
    LOG ( "Socket DC" );
    ASSERT ( _socket.get() == socket );

    _socket.reset();
    _timer.reset();

    if ( owner )
        owner->connectionFailed ( this );
}

void Lobby::socketRead ( Socket *socket, const char *bytes, size_t len, const IpAddrPort& address )
{
    LOG ( "Socket Read" );
    ASSERT ( _socket.get() == socket );
    const string data ( bytes, len );

    //_timer.reset();

    LOG( data );

    vector<string> rawdata = split( data, "\x1f" );
    string reqType = rawdata[0];
    if ( reqType == "LIST" ) {
        int entryCount = stoi( rawdata[1] );
        if ( entryCount > 0 ) {
            vector<string> lines = { rawdata.begin()+2, rawdata.end() };
            vector<string> names;
            vector<string> ip;
            for ( string line : lines ) {
                vector<string> nameipaddr = split( line, "\x1e" );
                names.push_back(nameipaddr[0]);
                ip.push_back(nameipaddr[1]);
            }
            entryMutex.lock();
            numEntries = stoi( rawdata[1] );
            LOG( lines[0] );
            LOG( numEntries );
            for ( int i = 0; i < numEntries; ++i ){
                entries[i] = names[i];
                ips[i] = ip[i];
            }
            for ( int i = numEntries; i < 20; ++i ) {
                entries[i] = blankEntry;
            }
            entryMutex.unlock();
        } else {
            entryMutex.lock();
            numEntries = 0;
            for ( int i = 0; i < 20; ++i ) {
                entries[i] = blankEntry;
            }
            entryMutex.unlock();
        }

    } else if  ( reqType == "HOST" ) {
        hostSuccess = rawdata[1] == "True";
    }

    if ( owner && !connectionSuccess ) {
        connectionSuccess = true;
        owner->unlock( this );
    }
    if ( owner && !hostResponse ) {
        hostResponse = true;
        owner->unlock( this );
    }
    LOG("endscoketre");
}

// Timer callback
void Lobby::timerExpired ( Timer *timer )
{
    LOG( "Timer Expired Callback" );
    ASSERT ( _timer.get() == timer );

    const string request = format ( "LIST,none" );
    LOG ( "Sending request:\n%s", request );

    _timer.reset ( new Timer ( this ) );
    _timer->start ( 3000 );

    if ( ! _socket->send ( &request[0], request.size() ) ) {
        LOG ( "Failed to send request!" );

        if ( owner )
            owner->connectionFailed ( this );
    }
}

void Lobby::run()
{
    LOG( "Lobby run" );
    connect( "ASDF" );
    EventManager::get().start();
    LOG( "Lobby stop" );
    stop();
}

void Lobby::stop()
{
    LOG( "stop" );
    _socket.reset();
    _timer.reset();
    owner->unlock( this );
    entryMutex.unlock();
}

void Lobby::host( string name, IpAddrPort port ) {
    hostSuccess = false;
    string portStr = port.str();
    hostResponse = false;
    const string request = format ( "HOST," + name + "|" + portStr );
    _timer.reset ( new Timer ( this ) );
    _timer->start ( 3000 );
    LOG( request );
    if ( ! _socket->send ( &request[0], request.size() ) ) {
        LOG ( "Failed to send request!" );

        if ( owner )
            owner->connectionFailed ( this );
    }
}

void Lobby::unhost() {
    const string request = format ( "UNHOST,none" );
    if ( ! _socket->send ( &request[0], request.size() ) ) {
        LOG ( "Failed to send request!" );

        if ( owner )
            owner->connectionFailed ( this );
    }
}
