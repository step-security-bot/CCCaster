#pragma once

#include "Enum.hpp"
#include "Protocol.hpp"

#include <string>
#include <vector>
#include <unordered_map>


// Set of command line options
ENUM ( Options,
       // Regular options
       Help,
       GameDir,
       Tunnel,
       Training,
       Broadcast,
       Spectate,
       Offline,
       NoUi,
       Tournament,
       MaxDelay,
       DefaultRollback,
       Fullscreen,
       AutoReplaySave,
       // Debug options
       FrameLimiter,
       Tests,
       Stdout,
       FakeUi,
       Dummy,
       StrictVersion,
       PidLog,
       SyncTest,
       Replay,
       // Special options
       NoFork,
       AppDir,
       SessionId,
       HeldStartDuration );


// Forward declaration
namespace option { class Option; }


struct OptionsMessage : public SerializableSequence
{
    size_t operator[] ( const Options& opt ) const
    {
        const auto it = _options.find ( ( size_t ) opt.value );

        if ( it == _options.end() )
            return 0;
        else
            return it->second.count;
    }

    void set ( const Options& opt, size_t count, const std::string& arg = "" )
    {
        if ( count == 0 )
            _options.erase ( opt.value );
        else
            _options[opt.value] = Opt ( count, arg );
    }

    const std::string& arg ( const Options& opt ) const
    {
        static const std::string EmptyString = "";

        const auto it = _options.find ( ( size_t ) opt.value );

        if ( it == _options.end() )
            return EmptyString;
        else
            return it->second.arg;
    }

    OptionsMessage ( const std::vector<option::Option>& opt );

    PROTOCOL_MESSAGE_BOILERPLATE ( OptionsMessage, _options )

private:

    struct Opt
    {
        size_t count;
        std::string arg;

        Opt() {}
        Opt ( size_t count, const std::string& arg = "" ) : count ( count ), arg ( arg ) {}

        CEREAL_CLASS_BOILERPLATE ( count, arg )
    };

    std::unordered_map<size_t, Opt> _options;
};
