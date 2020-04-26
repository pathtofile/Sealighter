// This is just a "healper" header to deal with the warning in Krabs' header
#pragma once
// Disable Warning for KrabsETW
// and it pollutes our build script with a warning
// we won't deal with because its not our code
#pragma warning( push )
#pragma warning( disable : 4244 )
#include "krabs.hpp"
#pragma warning( pop )

using namespace krabs;
