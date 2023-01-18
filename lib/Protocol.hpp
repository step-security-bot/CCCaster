#pragma once

#include "Enum.hpp"

#include <cereal/archives/binary.hpp>

#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <array>


#define EMPTY_MESSAGE_BOILERPLATE(NAME)                                                                     \
    NAME() {}                                                                                               \
    MsgPtr clone() const override;                                                                          \
    MsgType getMsgType() const override;

#define DECLARE_MESSAGE_BOILERPLATE(NAME)                                                                   \
    EMPTY_MESSAGE_BOILERPLATE(NAME)                                                                         \
    void save ( cereal::BinaryOutputArchive& ar ) const override;                                           \
    void load ( cereal::BinaryInputArchive& ar ) override;

#define PROTOCOL_MESSAGE_BOILERPLATE(NAME, ...)                                                             \
    EMPTY_MESSAGE_BOILERPLATE(NAME)                                                                         \
    void save ( cereal::BinaryOutputArchive& ar ) const override { ar ( __VA_ARGS__ ); }                    \
    void load ( cereal::BinaryInputArchive& ar ) override { ar ( __VA_ARGS__ ); }

#define CEREAL_CLASS_BOILERPLATE(...)                                                                       \
    void save ( cereal::BinaryOutputArchive& ar ) const { ar ( __VA_ARGS__ ); }                             \
    void load ( cereal::BinaryInputArchive& ar ) { ar ( __VA_ARGS__ ); }


// Message types, auto-generated from scanning all the headers
enum class MsgType : uint8_t
{
    FirstType = 0,

#include "ProtocolEnums.hpp"

    LastType
};

// Base message type
ENUM ( BaseType, SerializableMessage, SerializableSequence );

// Common declarations
struct Serializable;
typedef std::shared_ptr<Serializable> MsgPtr;
std::ostream& operator<< ( std::ostream& os, MsgType type );
std::ostream& operator<< ( std::ostream& os, const MsgPtr& msg );
std::ostream& operator<< ( std::ostream& os, const Serializable& msg );


// Function that does nothing to a message pointer
inline void ignoreMsgPtr ( Serializable * ) {}

// Null message pointer
const MsgPtr NullMsg;


// Contains protocol methods
class Protocol
{
public:

    // Encode a message to a series of bytes
    static std::string encode ( const Serializable& message );
    static std::string encode ( Serializable *message );
    static std::string encode ( const MsgPtr& msg );

    // Decode a series of bytes into a message, consumed indicates the number of bytes read.
    // This returns null if the message failed to decode, NOTE consumed will still be updated.
    static MsgPtr decode ( const char *bytes, size_t len, size_t& consumed );

    static bool checkMsgType ( MsgType type )
    {
        return ( type > MsgType::FirstType && type < MsgType::LastType );
    }
};


// Abstract base class for all serializable messages
class Serializable
{
public:

    // Basic constructor and destructor
    Serializable();
    virtual ~Serializable() {}

    // Return a clone
    virtual MsgPtr clone() const = 0;

    // Get message and base types
    virtual MsgType getMsgType() const = 0;
    virtual BaseType getBaseType() const = 0;

    // Serialize to and deserialize from a binary archive
    virtual void save ( cereal::BinaryOutputArchive& ar ) const {}
    virtual void load ( cereal::BinaryInputArchive& ar ) {}

    // Cast this to another another type
    template<typename T> T& getAs() { return *static_cast<T *> ( this ); }
    template<typename T> const T& getAs() const { return *static_cast<const T *> ( this ); }

    // Invalidate any cached data
    virtual void invalidate() const;

    // Return a string representation of this message, defaults to the message type
    virtual std::string str() const { std::stringstream ss; ss << getMsgType(); return ss.str(); }

    // Flag to indicate compression level
    mutable uint8_t compressionLevel;

private:

    typedef std::array<char, 16> HashType;

    // Cached hash data
    mutable HashType _hash;
    mutable bool _hashValid = true;

    // Serialize and deserialize the base type
    virtual void saveBase ( cereal::BinaryOutputArchive& ar ) const {}
    virtual void loadBase ( cereal::BinaryInputArchive& ar ) {}

    friend struct Protocol;
    friend struct SerializableMessage;
    friend struct SerializableSequence;

#ifndef RELEASE
    // Allow UdpSocket access to munge the hash for testing purposes
    friend class UdpSocket;
#endif
};


// Represents a regular message, should only be used when size constrained AND reliability is not required
class SerializableMessage : public Serializable
{
public:

    BaseType getBaseType() const override
    {
        static const BaseType baseType = BaseType::SerializableMessage;
        return baseType;
    }
};


// Represents a sequential message, should be used for any message that's not size constrained
class SerializableSequence : public Serializable
{
public:

    // Constructor
    SerializableSequence();
    SerializableSequence ( uint32_t sequence );

    BaseType getBaseType() const override
    {
        static const BaseType baseType = BaseType::SerializableSequence;
        return baseType;
    }

    // Get and set the message sequence
    uint32_t getSequence() const { return _sequence; }
    void setSequence ( uint32_t sequence ) const;

private:

    // Message sequence number
    mutable uint32_t _sequence = 0;

    void saveBase ( cereal::BinaryOutputArchive& ar ) const override { ar ( _sequence ); };
    void loadBase ( cereal::BinaryInputArchive& ar ) override { ar ( _sequence ); };
};
