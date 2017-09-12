#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include "FTPController.h"

class BaseController
{
private:
    FTPController* _ftpControllerPtr;
    
    static std::string runBashCommandWithOutput(const std::string command);
    
public:
    BaseController(void);
    ~BaseController(void);
};
#endif