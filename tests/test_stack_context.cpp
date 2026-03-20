#include "CrashCatch.hpp"
#include <iostream>
#include <fstream>
#include <string>

// This function is the actual crash site. Its name should appear in the stack trace.
// Give it a unique, obviously-named function so we can grep for it.
void crashSiteFunction() {
    int* ptr = nullptr;
    *ptr = 42; // crash here
}

int main() {
    CrashCatch::Config config;
    config.dumpFolder    = "./test_stack_output/";
    config.dumpFileName  = "test_stack";
    config.autoTimestamp = true;
    config.appVersion    = "test";
    config.includeStackTrace = true;

    CrashCatch::initialize(config);
    crashSiteFunction();
    return 0;
}
