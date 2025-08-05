#include "window.h"

#include <bitset>
#include <iostream>
#include <bits/ostream.tcc>

using namespace Voxel;

int main() {
    Window window(1280, 720, "");
    window.run();
    return 0;
}