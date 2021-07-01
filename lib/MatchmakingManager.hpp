#pragma once

#include "ConsoleUi.hpp"
#include "Socket.hpp"
#include "Timer.hpp"
#include "Pinger.hpp"
#include "KeyboardManager.hpp"

#define DEFAULT_GET_TIMEOUT ( 5000 )

class MatchmakingManager
    : Socket::Owner
    , Timer::Owner
    , public KeyboardManager::Owner
    , public Thread
{
public:
    struct Owner
    {
        virtual void connectionFailed( MatchmakingManager* lobby ) = 0;
        virtual void setAddr( MatchmakingManager* lobby, std::string addr ) = 0;
        virtual void setMode( MatchmakingManager* lobby, std::string mode ) = 0;
        virtual void unlock( MatchmakingManager* lobby ) = 0;
    };

    Owner *owner = 0;

    MatchmakingManager ( Owner* owner, IpAddrPort _address, std::string region );

    void stop();

    void connect();
    void disconnect();

    uint64_t timeout;
    IpAddrPort _address;
    bool connectionSuccess;

    Mutex hostMutex;
    CondVar hostCondVar;

private:

    SocketPtr serversocket;
    TimerPtr _timer;

    std::string region;

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

    // Keyboard
    void keyboardEvent ( uint32_t vkCode, uint32_t scanCode, bool isExtended, bool isDown ) override;
};
