#pragma once

#include "EventManager.hpp"
#include "ProcessManager.hpp"
#include "SocketManager.hpp"
#include "SmartSocket.hpp"
#include "TimerManager.hpp"
#include "Timer.hpp"
#include "KeyboardManager.hpp"
#include "IpAddrPort.hpp"
#include "Options.hpp"

#include <unordered_set>


// Log file that contains all the data needed to keep games in sync
#define SYNC_LOG_FILE FOLDER "sync.log"

// Controller mappings file extension
#define MAPPINGS_EXT ".mappings"


struct Main
        : public ProcessManager::Owner
        , public SmartSocket::Owner
        , public Timer::Owner
{
    OptionsMessage options;

    ClientMode clientMode;

    IpAddrPort address;

    ProcessManager procMan;

    SocketPtr serverCtrlSocket, ctrlSocket;

    SocketPtr serverDataSocket, dataSocket;

    TimerPtr stopTimer;

    Logger syncLog;


    Main() : procMan ( this ) {}

    Main ( const ClientMode& clientMode ) : clientMode ( clientMode ), procMan ( this ) {}
};


struct AutoManager
{
    bool doDeinit;

    AutoManager( int id = 0 )
    {
        TimerManager::get( id ).initialize();
        SocketManager::get( id ).initialize();
    }

    template<typename T>
    AutoManager ( T *main,
                  const void *window = 0,                           // Window to match, 0 to match all
                  const std::unordered_set<uint32_t>& keys = {},    // VK codes to match, empty to match all
                  const std::unordered_set<uint32_t>& ignore = {} ) // VK codes to specifically IGNORE
        : AutoManager()
    {
#ifdef RELEASE
        KeyboardManager::get().keyboardWindow = window;
        KeyboardManager::get().matchedKeys = keys;
        KeyboardManager::get().ignoredKeys = ignore;
        KeyboardManager::get().hook ( main );
#endif
    }

    ~AutoManager()
    {
        KeyboardManager::get().unhook();
        // TODO: Figure out a cleaner way of doing this
        if ( doDeinit ) {
            SocketManager::get().deinitialize();
            TimerManager::get().deinitialize();
        }
    }
};
