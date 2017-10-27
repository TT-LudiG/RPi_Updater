#include <exception>
#include <iostream>
#include <thread>

#include "BaseController.h"

#define SERVERNAME "vpn-wa-a.thermotrack.co.za"
#define PORT_GENERAL "49996"
#define PORT_TEMPERATURE "49995"
#define DELAY_UPDATER_LOOP_IN_SEC 300;

int main(int argc, char* argv[])
{
    std::string servername = SERVERNAME;
    std::string port_general = PORT_GENERAL;
    std::string port_temperature = PORT_TEMPERATURE;
    unsigned long int delay_updater_loop_in_sec = DELAY_UPDATER_LOOP_IN_SEC;
    
    std::string currentParam;
    
    if (argc > 1)
    {
        for (long int i = 1; i < argc; ++i)
        {
            try
            {
                currentParam = argv[i];
            }
            
            catch (...)
            {
                continue;
            }
            
            std::size_t indexEqualsChar = currentParam.find('=');
            
            if (indexEqualsChar != std::string::npos)
            {
                std::string token = currentParam.substr(0, indexEqualsChar);
                std::string value = currentParam.substr(indexEqualsChar + 1);
                
                if (token == "-servername")
                    servername = value;
                
                else if (token == "-port_general")
                    port_general = value;
                
                else if (token == "-port_temperature")
                    port_temperature = value;
                
                else if (token == "-delay_updater_loop_in_sec")
                    delay_updater_loop_in_sec = std::stoul(value);
            }
        }
    }
    
    BaseController* baseControllerPtr = nullptr;
    
    try
    {      
        baseControllerPtr = new BaseController(servername, port_general, port_temperature, delay_updater_loop_in_sec);
    }
    
    catch (const std::exception& e)
    {   
        if (baseControllerPtr != nullptr)
            delete baseControllerPtr;
        
        return 1;
    }

    std::thread updaterThread(&BaseController::checkUpdatesPeriodically, baseControllerPtr);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::thread monitorThread(&BaseController::monitorUpdaterThread, baseControllerPtr);
    
    std::cout << "Enter 'q' (quit), 'e' (exit) or 'c' (close) to end the program..." << std::endl;
    
    std::string inputLine;
    
    std::getline(std::cin, inputLine);
    
    while ((!baseControllerPtr->getFinalised()) && (inputLine != "q") && (inputLine != "e") && (inputLine != "c"))
        std::getline(std::cin, inputLine);
    
    baseControllerPtr->setFinalised();

    monitorThread.join();
    updaterThread.join();
    
    if (baseControllerPtr != nullptr)
        delete baseControllerPtr;
    
    return 0;
}