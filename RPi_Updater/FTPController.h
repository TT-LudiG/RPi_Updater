#ifndef FTPCONTROLLER_H
#define FTPCONTROLLER_H

#include <unordered_map>

#include "NetworkController.h"
#include "SessionFTPControlInfo.h"

#define FTP_MESSAGE_LENGTH_MAX 1400

class FTPController
{
private:
    NetworkController* _networkControllerPtr;
    
    std::unordered_map<unsigned long int, SessionFTPControlInfo*> _sessionsFTPControl;
    std::unordered_map<unsigned long int, unsigned long int> _sessionsFTPData;
    
    unsigned long int _nextSessionFTPControlID;
    unsigned long int _nextSessionFTPDataID;
    
    unsigned short int getStatusCode(const std::string response) const;
    
public:
    FTPController(void);
    ~FTPController(void);
    
    unsigned long int startFTPControlSession(const std::string servername, const std::string username, const std::string password);
    
    void startFTPDataSession(const unsigned long int sessionFTPControlID);
    
    void getFileWithFTPControlSession(unsigned long int sessionFTPControlID);
};

#endif