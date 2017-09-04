#include <chrono>
#include <iostream>
#include <thread>

#include "FTPController.h"

int main(int argc, char** argv)
{
    FTPController* ftpControllerPtr = new FTPController();
    
    try
    {
        unsigned long int sessionID = ftpControllerPtr->startFTPControlSession("41.185.23.172", "RPi-Dev", "jacoistehbawsW00T!");
        
        ftpControllerPtr->startFTPDataSession(sessionID);
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        ftpControllerPtr->getFileWithFTPControlSession(sessionID);
    }
    
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    if (ftpControllerPtr != nullptr)
        delete ftpControllerPtr;
}