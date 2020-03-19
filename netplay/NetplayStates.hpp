#pragma once

#include "Enum.hpp"


/* Netplay state transitions

    Unknown -> PreInitial -> Initial -> { AutoCharaSelect (spectate only), CharaSelect, ReplayMenu (offline only) }

    { AutoCharaSelect (spectate only), CharaSelect, ReplayMenu } -> Loading

    Loading -> { CharaIntro, InGame (training mode) }

    CharaIntro -> { InGame (versus mode) }

    Skippable -> { InGame (versus mode), RetryMenu }

    InGame -> { Skippable, CharaSelect (not on netplay), ReplayMenu }

    RetryMenu -> { Loading, CharaSelect }

*/


// PreInitial: The period while we are preparing communication channels
// Initial: The game starting phase
// AutoCharaSelect: Automatic character select (spectate only)
// CharaSelect: Character select
// Loading: Loading screen, distinct from skippable, so we can transition properly
// CharaIntro: Character Introductions, distinct so pregame data can be captured properly
// Skippable: Skippable states (round transitions, post-game, pre-retry)
// InGame: In-game state
// RetryMenu: Post-game retry menu
// ReplayMenu: Replay select menu
ENUM ( NetplayState, PreInitial, Initial, AutoCharaSelect, CharaSelect, Loading, CharaIntro, Skippable, InGame, RetryMenu, ReplayMenu );
