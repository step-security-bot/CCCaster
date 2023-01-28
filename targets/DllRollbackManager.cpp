#include "DllRollbackManager.hpp"
#include "MemDump.hpp"
#include "DllAsmHacks.hpp"
#include "ErrorStringsExt.hpp"

#include <utility>
#include <algorithm>

using namespace std;


// Linked rollback memory data (binary message format)
extern const unsigned char binary_res_rollback_bin_start;
extern const unsigned char binary_res_rollback_bin_end;

// Deserialized rollback memory data
static MemDumpList allAddrs;

template<typename T>
static inline void deleteArray ( T *ptr ) { delete[] ptr; }


void DllRollbackManager::GameState::save()
{
    ASSERT ( rawBytes != 0 );

    char *dump = rawBytes;

    for ( const MemDump& mem : allAddrs.addrs )
        mem.saveDump ( dump );

    ASSERT ( dump == rawBytes + allAddrs.totalSize );
}

void DllRollbackManager::GameState::load()
{
    fesetenv(&fp_env);

    ASSERT ( rawBytes != 0 );

    const char *dump = rawBytes;

    for ( const MemDump& mem : allAddrs.addrs )
        mem.loadDump ( dump );

    ASSERT ( dump == rawBytes + allAddrs.totalSize );
}

void DllRollbackManager::allocateStates()
{
    if ( allAddrs.empty() )
    {
        const size_t size = ( ( char * ) &binary_res_rollback_bin_end ) - ( char * ) &binary_res_rollback_bin_start;
        allAddrs.load ( ( char * ) &binary_res_rollback_bin_start, size );
    }

    if ( allAddrs.empty() )
        THROW_EXCEPTION ( "Failed to load rollback data!", ERROR_BAD_ROLLBACK_DATA );

    if ( ! _memoryPool )
        _memoryPool.reset ( new char[NUM_ROLLBACK_STATES * allAddrs.totalSize], deleteArray<char> );

    for ( size_t i = 0; i < NUM_ROLLBACK_STATES; ++i )
        _freeStack.push ( i * allAddrs.totalSize );

    _statesList.clear();

    for ( auto& sfxArray : _sfxHistory )
        memset ( &sfxArray[0], 0, CC_SFX_ARRAY_LEN );
}

void DllRollbackManager::deallocateStates()
{
    _memoryPool.reset();

    while ( ! _freeStack.empty() )
        _freeStack.pop();

    _statesList.clear();
}

void DllRollbackManager::saveState ( const NetplayManager& netMan )
{
    if ( _freeStack.empty() )
    {
        ASSERT ( _statesList.empty() == false );

        if ( _statesList.front().indexedFrame.parts.frame <= netMan.getRemoteFrame() )
        {
            auto it = _statesList.begin();
            ++it;
            _freeStack.push ( it->rawBytes - _memoryPool.get() );
            _statesList.erase ( it );
        }
        else
        {
            _freeStack.push ( _statesList.front().rawBytes - _memoryPool.get() );
            _statesList.pop_front();
        }
    }

    std::fenv_t fp_env;

    fegetenv(&fp_env);

    GameState state =
    {
        netMan._state,
        netMan._startWorldTime,
        netMan._indexedFrame,
        fp_env,
        _memoryPool.get() + _freeStack.top()
    };

    _freeStack.pop();
    state.save();
    _statesList.push_back ( state );

    uint8_t *currentSfxArray = &_sfxHistory [ netMan.getFrame() % NUM_ROLLBACK_STATES ][0];
    memcpy ( currentSfxArray, AsmHacks::sfxFilterArray, CC_SFX_ARRAY_LEN );
}

bool DllRollbackManager::loadState ( IndexedFrame indexedFrame, NetplayManager& netMan )
{
    if ( _statesList.empty() )
    {
        LOG ( "Failed to load state: indexedFrame=%s", indexedFrame );
        return false;
    }

    LOG ( "Trying to load state: indexedFrame=%s; _statesList={ %s ... %s }",
          indexedFrame, _statesList.front().indexedFrame, _statesList.back().indexedFrame );

    const uint32_t origFrame = netMan.getFrame();

    for ( auto it = _statesList.rbegin(); it != _statesList.rend(); ++it )
    {
#ifdef RELEASE
        if ( ( it->indexedFrame.value <= indexedFrame.value ) || ( & ( *it ) == &_statesList.front() ) )
#else
        if ( it->indexedFrame.value <= indexedFrame.value )
#endif
        {
            LOG ( "Loaded state: indexedFrame=%s", it->indexedFrame );

            // Overwrite the current game state
            netMan._state = it->netplayState;
            netMan._startWorldTime = it->startWorldTime;
            netMan._indexedFrame = it->indexedFrame;
            it->load();

            // Count the number of frames rolled back
            int rbFrames;
            if ( !netMan.config.mode.isTraining() ) {
                rbFrames = _statesList.back().indexedFrame.value - it->indexedFrame.value;
                LOG("Rolled back %i frames", rbFrames);
            }
            // Erase all other states after the current one.
            // Note: it.base() returns 1 after the position of it, but moving forward.
            for ( auto jt = it.base(); jt != _statesList.end(); ++jt )
            {
                _freeStack.push ( jt->rawBytes - _memoryPool.get() );
            }

            // Disable rollback for input history if in training mode
            if ( !netMan.config.mode.isTraining() ) {
                LOG( "Fixing input history for rbFrames %d", rbFrames );
                // Erase one frame of inputs from the game's replay structs for each frame rolled back.
                for (; rbFrames > 0; rbFrames--) {
                    if (!*(RepRound**)CC_REPROUND_TBL_ENDPTR_ADDR) {
                        LOG( "Missing replay table" );
                        break;
                    }
                    RepRound* curRound = (*(RepRound**)CC_REPROUND_TBL_ENDPTR_ADDR - 1);
                    LOG( "%d", curRound );
                    if (!curRound->inputs) {
                        LOG( "Missing inputs" );
                        break;
                    }
                    // Assumes there are always containers for 4 players in input container table; may not be true
                    for (int i=0; i<4; i++) {
                        RepInputContainer* inputs = &(curRound->inputs[i]);
                        if (!inputs->states) {
                            LOG( "player %d no states", i );
                            continue;
                        }
                        RepInputState* state = &(inputs->states[inputs->activeIndex]);
                        if (!state->frameCount) {
                            LOG( "player %d no framecount", i+1 );
                            continue;
                        }
                        if (state->frameCount == 1) {
                            memset(state, 0, sizeof(RepInputState));
                            inputs->statesEnd -= sizeof(RepInputState);
                            LOG("Replay state %i for p%i has frame count 1; decrementing index", inputs->activeIndex, i+1);
                            inputs->activeIndex--;
                        } else {
                            LOG("Replay state %i for p%i has frame count %i; decrementing count", inputs->activeIndex, i+1, state->frameCount);
                            state->frameCount--;
                        }
                    }
                }
            }

            _statesList.erase ( it.base(), _statesList.end() );

            // Initialize the SFX filter by flagging all played SFX flags in the range (R,S),
            // where R is the actual reset frame, and S is the original starting frame.
            // Note: we can skip frame S, because the current SFX filter array is already initialized by frame S.
            for ( uint32_t i = netMan.getFrame() + 1; i < origFrame; ++i )
            {
                for ( uint32_t j = 0; j < CC_SFX_ARRAY_LEN; ++j )
                    AsmHacks::sfxFilterArray[j] |= _sfxHistory [ i % NUM_ROLLBACK_STATES ][j];
            }

            // We set the SFX filter flag to 0x80. Since played (but filtered) SFX are incremented,
            // unplayed sound effects in the filter will stay as 0 or 0x80.
            for ( uint32_t j = 0; j < CC_SFX_ARRAY_LEN; ++j )
            {
                if ( AsmHacks::sfxFilterArray[j] )
                    AsmHacks::sfxFilterArray[j] = 0x80;
            }

            return true;
        }
    }

    LOG ( "Failed to load state: indexedFrame=%s", indexedFrame );
    return false;
}

void DllRollbackManager::saveRerunSounds ( uint32_t frame )
{
    uint8_t *currentSfxArray = &_sfxHistory [ frame % NUM_ROLLBACK_STATES ][0];

    // Rewrite the sound effects history during re-run
    for ( uint32_t j = 0; j < CC_SFX_ARRAY_LEN; ++j )
    {
        if ( AsmHacks::sfxFilterArray[j] & ~0x80 )
            currentSfxArray[j] = 1;
        else
            currentSfxArray[j] = 0;
    }
}

void DllRollbackManager::finishedRerunSounds()
{
    // Cancel unplayed sound effects after rollback
    for ( uint32_t j = 0; j < CC_SFX_ARRAY_LEN; ++j )
    {
        // Filter flag 0x80 means the SFX didn't play after rollback since the filter didn't get incremented
        if ( AsmHacks::sfxFilterArray[j] == 0x80 )
        {
            // Play the SFX muted to cancel it
            CC_SFX_ARRAY_ADDR[j] = 1;
            AsmHacks::sfxMuteArray[j] = 1;
        }
    }

    // Cleared last played sound effects
    memset ( AsmHacks::sfxFilterArray, 0, CC_SFX_ARRAY_LEN );
}
