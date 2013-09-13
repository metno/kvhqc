
#include "Functors.hh"

namespace Helpers {

bool float_eq::operator()(float a, float b) const
{
    return std::fabs(a - b) < 0.01f;
}

} // namespace Helpers
