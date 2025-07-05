#include <iostream>
#include "ShaderTypeCodeGen.h"

int main()
{
    std::cout << "\nCode Gen ...\n";

    ShaderTypeCodeGen::GenerateCode();

    std::cout << "Code Gen Done!\n";
    std::cout << "************************\n";
    std::cout << "************************\n\n\n";
}

