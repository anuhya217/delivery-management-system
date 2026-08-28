#include <iostream>
#include "services/dsp_service.h"

int main() {

    std::string name;
    std::string region;

    std::cout << "Enter DSP Name: ";
    std::getline(std::cin, name);

    std::cout << "Enter Region: ";
    std::getline(std::cin, region);

    createDSP(name, region);

    return 0;
}