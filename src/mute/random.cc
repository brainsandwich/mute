#include "mute/random.h"

namespace mute
{
    RandomNumberGenerator RNG = RandomNumberGenerator::init(0b1 << 10, 0.001);
}