#define CRASHCATCH_AUTO_INIT
#include "CrashCatch.hpp"

int main(){
    int* ptr = nullptr;
    *ptr = 42; //Simulated crash
    return 0;
}