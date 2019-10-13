// gradient.cpp : Defines the entry point for the console application.
//
#include <iostream>

struct student {
    char *name;
    uint32_t age;
    char *place;
};
using namespace std;


int main() {
    auto off = (uint64_t) (&((student *)0)->age);
    cout << off << endl;
    return 0;
}
