#pragma once

#include <codeanalysis\warnings.h>
#define SILENCE_CODE_ANALYSIS                              \
__pragma(warning( push ))                                  \
__pragma(warning( disable : ALL_CODE_ANALYSIS_WARNINGS ))  \
__pragma(warning( disable : ALL_CPPCORECHECK_WARNINGS ))   \
__pragma(warning( disable : 26812 4244 4267 ))

#define RESTORE_CODE_ANALYSIS \
__pragma(warning( pop ))