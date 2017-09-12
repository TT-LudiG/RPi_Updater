#include <cstdio>
#include <iostream>
#include <sstream>

#include "BaseController.h"

BaseController::BaseController(void): _ftpControllerPtr(nullptr)
{   
    _ftpControllerPtr = new FTPController();
    
    unsigned long int sessionID = _ftpControllerPtr->startFTPControlSession("41.185.23.172", "RPi-Dev", "jacoistehbawsW00T!", 500);
    
    _ftpControllerPtr->getFileWithFTPControlSession(sessionID, "RPi_BLE_Scanner/RPi_BLE_Scanner.tar.gz", "//home/pi/RPi_BLE_Scanner.tar.gz", 500, 10);
    
    runBashCommandWithOutput("sudo tar -xzvf //home/pi/RPi_BLE_Scanner.tar.gz -C //home/pi/");
}

BaseController::~BaseController(void)
{
    if (_ftpControllerPtr != nullptr)
        delete _ftpControllerPtr;
}

std::string BaseController::runBashCommandWithOutput(const std::string command)
{
    std::stringstream outputStream;
    
    char output[100];
    
    FILE* filePtrTerminal = nullptr;
    
    filePtrTerminal = popen(command.c_str(), "r");
    
    while (std::fgets(output, sizeof(output), filePtrTerminal) != 0)
        outputStream << output;
    
    pclose(filePtrTerminal);
    
    return outputStream.str();
}