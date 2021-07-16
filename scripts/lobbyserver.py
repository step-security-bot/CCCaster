#!/usr/bin/env python38
import asyncio
import time
import websockets
from enum import Enum
import json

WAIT = 1
INGAME = 2
VERSUS = 1
TRAINING = 2

SERVER = "ws://fierce-lowlands-57630.herokuapp.com/"

regions = ["NorthAmericaWest", "NorthAmericaEast","SouthAmerica","Europe","MiddleEast","Australia","Japan" ]
regionMap = { "NA West":"NorthAmericaWest", "NA East":"NorthAmericaEast",
              "SA":"SouthAmerica", "EU":"Europe",
              "Middle East":"MiddleEast", "Asia":"Japan", "OCE":"Australia" };

def formatForServer( string ):
    fmt = string.replace( '"', '\\"' )
    return f'"{fmt}"'

class HostEntry:

    def __init__( self, name, ip ):
        self.namelen = 43
        self.name = name[:self.namelen]
        self.opponent = ""
        self.ip = ip
        self.status = WAIT
        self.numtimeouts = 0
        self.mode = VERSUS

    def formatWaiting( self ):
        modestring = "UNK"
        if self.mode == VERSUS:
            modestring = "VRS"
        elif self.mode == TRAINING:
            modestring = "TRN"
        return f"{self.name:<43}|{modestring}|Waiting\x1e{self.ip}"

    def formatInGame( self ):
        modestring = "UNK"
        if self.mode == VERSUS:
            modestring = "VS "
        elif self.mode == TRAINING:
            modestring = "TRN"
        return f"{self.name:<self.namelen}|{modestring}|In Game\x1e{self.ip}"

    def __repr__( self ):
        if self.status == WAIT:
            return self.formatWaiting()
        elif self.status == INGAME:
            return self.formatInGame()
        else:
            return "Invalid Entry"

class Lobby:

    def __init__( self ):
        self.numEntries = 0
        self.entries = {}
        self.maxEntries = 20

    def format( self ):
        formatEntries = [ e.__repr__() for e in self.entries.values() ]
        lines= "\x1f".join( formatEntries )
        return f"{self.numEntries}\x1f{lines}"

    def addHost( self, uid, name, ipaddrport ):
        if self.numEntries < self.maxEntries:
            self.entries[ uid ] = HostEntry( name, ipaddrport )
            self.numEntries += 1
            return True
        return False

    def removeHost( self, uid ):
        if uid in self.entries:
            del self.entries[ uid ]
            self.numEntries -= 1
            return True
        return False

lobby = Lobby()

lobbylock = asyncio.Lock()
idlock = asyncio.Lock()

"""
lobby.addHost( 1, *dummyEntries[1] )
lobby.addHost( 2, *dummyEntries[2] )
lobby.addHost( 3, *dummyEntries[3] )
"""
uuid = 4
print(lobby.format())

def lformat( entries ):
    dlines = []
    for v in entries.values():
        dlines.append("\x1e".join(v) )
    lines = "\x1f".join(dlines)
    return f"{len(entries)}\x1f{lines}"

def formatConf( addr, region ):
    return formatForServer( json.dumps( {"eventType":"initialConfig", "ipAddress":addr, "region":region } ) )

async def checkHostStatus( key, name, hostAddr ):
    host, port = hostAddr.split( ":" )
    try:
        reader, writer = await asyncio.wait_for( asyncio.open_connection( host, port ), timeout=1 )
        data = await reader.read( 100 )
        clientFlags = data[9]
        print( clientFlags )
        gameStarted = ( clientFlags & 0x2 ) == 0x2
        print( gameStarted )
        if gameStarted:
            async with lobbylock:
                if key in lobby.entries:
                    lobby.entries[key].state = INGAME
        return True
    except asyncio.TimeoutError:
        print("timed out!")
        async with lobbylock:
            if key in lobby.entries:
                lobby.entries[key].numtimeouts += 1
                if lobby.entries[key].numtimeouts > 5:
                    lobby.removeHost( key )
        return False
    except ConnectionRefusedError:
        print("connection refused")
        async with lobbylock:
            if key in lobby.entries:
                lobby.entries[key].numtimeouts += 1
                if lobby.entries[key].numtimeouts > 5:
                    lobby.removeHost( key )
        return False

async def checkAllStatus():
    while True:
        aw = []
        for k, v in lobby.entries.items():
            aw.append( checkHostStatus( k, v.name, v.ip ) )
        rv = await asyncio.gather( *aw )
        print (rv)
        await asyncio.sleep(5)

async def checkConnectionClosed( reader ):
    if reader.at_eof():
        print("connection closed")
        async with lobbylock:
            lobby.removeHost( connectionid )
        writer.close()
        return True
    return False

async def handle_connect(reader, writer):
    addr = writer.get_extra_info('peername')
    print(f"Connection from {addr}")
    global lobbylock, idlock, lobby, uuid
    async with idlock:
        connectionid = uuid
        uuid += 1
        if uuid > 2147483647 - 1:
            uuid = 0
    while True:
        data = await reader.read(100)
        if await checkConnectionClosed( reader ):
            return
        message = data.decode()
        try:
            req, info = message.split(",")
        except:
            print( "invalid request" )
            print( message )
            writer.close()
            return
        print( "Req", req )
        if req == "LIST":
            retString = "LIST\x1f"+lobby.format()
            print(f"< {retString}")
            writer.write(bytes( retString, 'utf-8'))
            await writer.drain()
        elif req == "HOST":
            name, port = info.split("|")
            if name == "":
                name = "Anonymous"
            print("n", name)
            print("p", port)
            print("a", addr)
            async with lobbylock:
                success = lobby.addHost( connectionid, name, addr[0] + port );
            retString = f"HOST\x1f{success!r}"
            writer.write(bytes( retString, 'utf-8'))
            print("s", retString)
            await writer.drain()
        elif req == "UNHOST":
            async with lobbylock:
                lobby.removeHost( connectionid )
        elif req == "MMSTART":
            region, port = info.split("|")
            async with websockets.connect(SERVER) as websocket:
                ipaddr = addr[0]
                addrFormat = formatConf( f"{ipaddr}:{port}", regionMap[region] )
                print("Af", addrFormat)
                await websocket.send( addrFormat )
                """
                pingtest temporarily ignored
                resp = await websocket.recv()
                print(f"< {resp}")
                # ignore pingtest for now, send back dummy
                matchers = [ {"matcherID":rid,
                              "ping":str( 100 - 50 * int( rid == regionMap[ region ] ) ) } for rid in regions ]
                dummyPingTest = formatForServer( json.dumps({"eventType":'pingTestResponse',
                                                             "matchers": matchers}) )
                await websocket.send(dummyPingTest)
                """
                resp = await websocket.recv()
                print(f"< {resp}")
                jresp = json.loads( resp )
                while jresp["eventType"] == "noOpponents":
                    print(f"NOP")
                    # todo add timeout
                    resp = await websocket.recv()
                    jresp = json.loads( resp )
                    if await checkConnectionClosed( reader ):
                        return
                while jresp["eventType"] == "pingTest":
                    addr = jresp["address"]
                    writer.write( bytes( f"PINGTEST\x1f{addr}", 'utf-8' ) )
                    await writer.drain()
                    data = await reader.read( 100 )
                    msg = data.decode()
                    ping = int( msg )
                    msg = formatForServer( json.dumps( { "eventType" :'pingTestResponse',
                                                         "matcherID" : jresp[ "matcherID" ],
                                                         "ping" : ping  } ) )
                    await websocket.send( msg )
                    resp = await websocket.recv()
                    print(f"< {resp}")
                    jresp = json.loads( resp )
                    if await checkConnectionClosed( reader ):
                        return
                if jresp["eventType"] == "joinMatch":
                    msg = f"CLIENT\x1f{jresp['address']}"
                    print( msg )
                    writer.write( bytes( msg, 'utf-8' ) )
                    await writer.drain()
                elif jresp["eventType"] == "openPort":
                    print(f"HOST")
                    writer.write( bytes( "HOST", 'utf-8' ) )
                    await writer.drain()
                    data = await reader.read( 100 )
                    msg = data.decode()
                    if msg == "SUCCESS":
                        print(msg)
                        await websocket.send( formatForServer( json.dumps ( {"eventType":'portIsOpen'} ) ) )
                    await asyncio.sleep(10)

        else:
            print("invalid request")
            print(data)
            print("connection closed")
            writer.close()
            return


        await asyncio.sleep(1)

    print("Close the connection")
    writer.close()

async def main():
    server = await asyncio.start_server(
        handle_connect, '0.0.0.0', 3502 )

    addr = server.sockets[0].getsockname()
    print(f'Serving on {addr}')

    task = asyncio.create_task( checkAllStatus() )
    await task
    async with server:
        await server.serve_forever()

asyncio.run(main())
