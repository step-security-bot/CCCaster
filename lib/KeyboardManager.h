#pragma once

#include "Protocol.h"
#include "Socket.h"

#include <unordered_set>


struct KeyboardEvent : public SerializableMessage
{
    int vkCode = 0;
    uint8_t isDown = 0;

    KeyboardEvent ( int vkCode, uint8_t isDown ) : vkCode ( vkCode ), isDown ( isDown ) {}

    PROTOCOL_MESSAGE_BOILERPLATE ( KeyboardEvent, vkCode, isDown )
};


class KeyboardManager : public Socket::Owner
{
public:

    struct Owner
    {
        virtual void keyboardEvent ( int vkCode, bool isDown ) = 0;
    };

    Owner *owner = 0;

private:

    // Socket to receive KeyboardEvent messages
    SocketPtr recvSocket;

    // Socket callback
    void readEvent ( Socket *socket, const MsgPtr& msg, const IpAddrPort& address ) override;

public:

    // Hook keyboard events
    void hook ( Owner *owner,
                const void *window = 0,
                const std::unordered_set<int>& keys = {},
                uint8_t options = 0 );

    // Unhook keyboard events
    void unhook();

    // Get the singleton instance
    static KeyboardManager& get();
};