
#include "test_macros.h"

int main() {
    auto& r = TestRunner::getInstance();
    return r.run_all_tests();
}

