#pragma once

//#include <codeanalysis\warnings.h>
#define SILENCE_CODE_ANALYSIS                              \
__pragma(warning( push ))                                  \
__pragma(warning( disable : ALL_CODE_ANALYSIS_WARNINGS ))  \
__pragma(warning( disable : ALL_CPPCORECHECK_WARNINGS ))   \
__pragma(warning( disable : 26812 4244 4267 ))

#define RESTORE_CODE_ANALYSIS \
__pragma(warning( pop ))

// Some of these are pretty sus, ngl
// 4324 structure was padded due to alignment specifier
// 4061 enumerator in switch not explicitly handled by case label
// 4200 nonstandard extension used: zero-sized array in struct/union
// 4265 class has virtual functions, but its non-trivial destructor is not virtual
// 4266 no override available for virtual member function from base, function is hidden
// 4365 conversion, signed/unsigned mismatch
// 4371 layout of class may have changed from a previous version of the compiler due to better packing of member
// 4388 signed/unsigned mismatch
// 4514 unreferenced inline function has been removed
// 4582 constructor is not implicitly called
// 4583 destructor is not implicitly called
// 4619 #pragma warning: there is no warning number 'n'
// 4623 default constructor was implicitly defined as deleted
// 4625 copy constructor was implicitly defined as deleted
// 4626 assignment operator was implicitly defined as deleted
// 4668 'x' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
// 4710 function not inlined
// 4820 'x' bytes padding added after data member 'y'
// 5026 move constructor was implicitly defined as deleted
// 5027 move assignment operator was implicitly defined as deleted
// 5031 #pragma warning(pop); likely mismatch, popping warning state pushed in different file
// 5032 detected #pragma warning(push) with no corresponding #pragma warning(pop)
// 5039 pointer or reference to potentially throwing function passed to 'extern "C"' under -EHc
// 5105 macro expansion producing 'defined' has undefined behavior
// 5204 class has virtual functions, but its trivial destructor is not virtual
// 5220 a non-static data memeber with a volatile qualified type no longer implies
#define SILENCE_EXTERNAL
__pragma(warning( push )) \
__pragma(warning( disable : 4201 4324 4061 4200 4265 4266 4365 4371 4388 4514 4582 4583 4619 4623 4625 4626 4668 4710 4820 5026 5027 5031 5032 5039 5105 5204 5220 ))

#define RESTORE_EXTERNAL \
__pragma(warning( pop ))