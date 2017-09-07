#include <iostream>

#include "BaseController.h"

int main(int argc, char* argv[])
{
    BaseController* baseControllerPtr;
    
    try
    {
        baseControllerPtr = new BaseController();
    }
    
    catch (const std::exception& e)
    {      
        if (baseControllerPtr != nullptr)
            delete baseControllerPtr;
        
        return 1;
    }
    
    std::cout << "Enter 'q' (quit), 'e' (exit) or 'c' (close) to end the program..." << std::endl;
    
    std::string inputLine;
    
    std::getline(std::cin, inputLine);
    
    while ((inputLine != "q") && (inputLine != "e") && (inputLine != "c"))
        std::getline(std::cin, inputLine);
    
    if (baseControllerPtr != nullptr)
        delete baseControllerPtr;
    
    return 0;
}