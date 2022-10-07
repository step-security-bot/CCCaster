#pragma once

#include "ErrorStrings.hpp"

#include <defines.hpp>

#define ERROR_PIPE_OPEN             "Failed to start game!\nIs " MBAA_EXE " already running?"

#define ERROR_PIPE_START            "Failed to start game!\nIs " BINARY " in same folder as " MBAA_EXE "?"

#define ERROR_PIPE_RW               "Failed to communicate with " MBAA_EXE

#define ERROR_INVALID_GAME_MODE     "Unhandled game mode!"

#define ERROR_INVALID_HOST_CONFIG   "Host sent invalid configuration!"

#define ERROR_INVALID_DELAY         "Delay must be less than 255!"

#define ERROR_INVALID_ROLLBACK      "Rollback must be less than %u!"

#define ERROR_BAD_ROLLBACK_DATA     "Rollback data is corrupted!\nTry reinstalling?"
