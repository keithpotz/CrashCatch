#include "CrashCatch.hpp"

int main(){
    CrashCatch::enable();

    int* ptr = nullptr;
    *ptr = 42; //Simulated crash
    return 0;
}