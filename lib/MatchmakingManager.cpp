#include "MatchmakingManager.hpp"
#include "ConsoleUi.hpp"
#include "TcpSocket.hpp"
#include "Exceptions.hpp"
#include "EventManager.hpp"
#include "StringUtils.hpp"

using namespace std;

MatchmakingManager::MatchmakingManager( Owner* owner, IpAddrPort _address, string region )
    : owner( owner ), _address( _address ), region( region )
{
    timeout = 1000;
    connectionSuccess = false;
}

void MatchmakingManager::run()
{
    LOG( "mmm run" );
    connect();
    EventManager::get().start();
    LOG( "mmm stop" );
    stop();
}

void MatchmakingManager::stop()
{
    LOG( "stop" );
    serversocket.reset();
    _timer.reset();
    owner->unlock( this );
}

void MatchmakingManager::connect()
{
    string url = _address.str();
    LOG ( "Connecting to: '%s'", url );

    try
    {
        serversocket = TcpSocket::connect ( this, url, true, timeout ); // Raw socket
    }
    catch ( ... )
    {
        serversocket.reset();

        LOG ( "Failed to create socket!" );

        if ( owner )
            owner->connectionFailed ( this );
    }
}

void MatchmakingManager::socketConnected ( Socket *socket )
{
    ASSERT ( serversocket.get() == socket );
    const string request = format ( "MMSTART," + region );
    LOG ( "Sending request:\n%s", request );

    _timer.reset ( new Timer ( this ) );
    _timer->start ( 3000 );

    if ( ! serversocket->send ( &request[0], request.size() ) ) {
        LOG ( "Failed to send request!" );

        if ( owner )
            owner->connectionFailed ( this );
    }
    if ( owner && !connectionSuccess ) {
        connectionSuccess = true;
        owner->unlock( this );
    }
}

void MatchmakingManager::socketDisconnected ( Socket *socket )
{
    LOG ( "Socket DC" );
    ASSERT ( serversocket.get() == socket );

    serversocket.reset();
    _timer.reset();
}

void MatchmakingManager::socketRead ( Socket *socket, const char *bytes, size_t len, const IpAddrPort& address )
{
    LOG ( "Socket Read" );
    ASSERT ( serversocket.get() == socket );
    const string data ( bytes, len );

    //_timer.reset();

    LOG( data );

    vector<string> rawdata = split( data, "\x1f" );
    string reqType = rawdata[0];
    if ( reqType == "CLIENT" ) {
        if ( owner ) {
            _timer.reset();
            owner->setMode( this, "Client" );
            owner->setAddr( this, rawdata[1] );
            owner->unlock( this );
        }
    } else if ( reqType == "HOST" ) {
        if ( owner ) {
            owner->setMode( this, "Host" );
            owner->unlock( this );
            LOCK( hostMutex );
            hostCondVar.wait ( hostMutex );
            const string request = format ( "46318" );
            LOG ( "Sending request:\n%s", request );

            _timer.reset ( new Timer ( this ) );
            _timer->start ( 3000 );

            if ( ! serversocket->send ( &request[0], request.size() ) ) {
                LOG ( "Failed to send request!" );

                if ( owner )
                    owner->connectionFailed ( this );
            }
        }
    }
}

// Timer callback
void MatchmakingManager::timerExpired ( Timer *timer )
{
    LOG( "Timer Expired Callback" );
    ASSERT ( _timer.get() == timer );

    serversocket.reset();
    _timer.reset();

    if ( owner )
        owner->connectionFailed ( this );

    return;
}

// KeyboardManager callback
void MatchmakingManager::keyboardEvent ( uint32_t vkCode, uint32_t scanCode, bool isExtended, bool isDown )
{
    if ( vkCode == VK_ESCAPE )
        stop();
}
