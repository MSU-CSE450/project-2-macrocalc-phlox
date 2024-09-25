#include <iostream>
#include <exception>
#include "../lexer.hpp"

using emplex::Lexer;

bool lexerTest1 () { 
    try {
        Lexer lexer;

    } catch(std::exception& e) {
        std::cout << "Lexer construction failed" << std::endl;
    }

    return true;
}

int main() {

    std::cout << "Lexer test 1: " << lexerTest1() << std::endl;
    return 0;
}