#include <iostream>
#include <exception>
#include <string>
#include <fstream>
#include "../lexer.hpp"

using emplex::Lexer;


// Test basic construction
bool lexerTest1 () { 
    try {
        Lexer lexer;

    } catch(std::exception& e) {
        std::cout << "Lexer construction failed" << std::endl;
    }

    return true;
}

// Test the first sample given in the project description
//
// Incomplete
//
bool lexerTest2 () {

    Lexer lexer;
    std::ifstream inputFile("sample_text/test2.txt");


    lexer.Tokenize(inputFile);


    return true;
}

int main() {

    std::cout << "Lexer test 1: " << lexerTest1() << std::endl;
    std::cout << "Lexer test 2: " << lexerTest2() << std::endl;
    return 0;
}