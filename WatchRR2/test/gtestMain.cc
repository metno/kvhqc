
#include "HqcLogging.hh"
#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    Log4CppConfig log4cpp("-.!!=-:");
    return RUN_ALL_TESTS();
}
