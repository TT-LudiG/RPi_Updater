#include <exception>
#include <iostream>
#include <thread>

#include "BaseController.h"

#define SERVERNAME_GENERAL "vpn-wa-a.thermotrack.co.za"
#define PORT_GENERAL "49996"
#define DELAY_UPDATER_LOOP_IN_SEC 300;
#define NAME_COMMON_PREFIX "client-rpi-ble-"

int main(int argc, char* argv[])
{
    std::string servername_general = SERVERNAME_GENERAL;
    std::string port_general = PORT_GENERAL;
    unsigned long int delay_updater_loop_in_sec = DELAY_UPDATER_LOOP_IN_SEC;
    std::string name_common_prefix = NAME_COMMON_PREFIX;
    
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
                
                if (token == "-servername_general")
                    servername_general = value;
                
                else if (token == "-port_general")
                    port_general = value;
                
                else if (token == "-delay_updater_loop_in_sec")
                    delay_updater_loop_in_sec = std::stoul(value);
                
                else if (token == "-name_common_prefix")
                    name_common_prefix = value;
            }
        }
    }
    
    BaseController* baseControllerPtr = nullptr;
    
    try
    {      
        baseControllerPtr = new BaseController(servername_general, port_general, delay_updater_loop_in_sec, name_common_prefix);
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