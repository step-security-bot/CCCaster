#pragma once

#include "ConsoleUi.hpp"
#include "Socket.hpp"
#include "Timer.hpp"

#define DEFAULT_GET_TIMEOUT ( 5000 )

enum GameType
{
    FFA,
    WinnerStaysOn,
};

class Lobby
    : Socket::Owner
    , Timer::Owner
    , public Thread
{
public:
    struct Owner
    {
        virtual void connectionFailed( Lobby* lobby ) = 0;
        virtual void unlock( Lobby* lobby ) = 0;
    };

    Owner *owner = 0;

    std::vector<std::string> getMenu();
    std::vector<std::string> getIps();

    Lobby ( Owner* owner );

    bool connect( std::string url );
    void disconnect();
    void host( std::string name, IpAddrPort port );
    void unhost();

    void init( Owner* owner );

    void stop();

    uint64_t timeout;
    IpAddrPort _address;
    bool connectionSuccess;

    int numEntries;
    bool hostSuccess;
    Mutex entryMutex;

private:
    SocketPtr _socket;

    TimerPtr _timer;

    std::string blankEntry;

    std::vector<std::string> entries;
    std::vector<std::string> ips;

    bool hostResponse;

    // Socket callbacks
    void socketAccepted ( Socket *socket ) override {}
    void socketConnected ( Socket *socket ) override;
    void socketDisconnected ( Socket *socket ) override;
    void socketRead ( Socket *socket, const MsgPtr& msg, const IpAddrPort& address ) override {}
    void socketRead ( Socket *socket, const char *bytes, size_t len, const IpAddrPort& address ) override;

    // Timer callback
    void timerExpired ( Timer *timer ) override;

    // Thread
    void run() override;

};
