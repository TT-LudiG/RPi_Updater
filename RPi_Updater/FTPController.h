#ifndef FTPCONTROLLER_H
#define FTPCONTROLLER_H

#include <unordered_map>

#include "NetworkController.h"
#include "SessionFTPControlInfo.h"

#define FTP_FILE_LENGTH_MAX 1000000
#define FTP_MESSAGE_LENGTH_MAX 1400

class FTPController
{
private:
    NetworkController* _networkControllerPtr;
    
    std::unordered_map<unsigned long int, SessionFTPControlInfo*> _sessionsFTPControl;
    
    unsigned long int _nextSessionFTPControlID;
    
    unsigned short int getFTPDataPort(const unsigned long int sessionFTPControlID) const;
    
    static unsigned short int getStatusCode(const std::string response);
    
public:
    FTPController(void);
    ~FTPController(void);
    
    unsigned long int startFTPControlSession(const std::string servername, const std::string username, const std::string password);
    
    void getFileWithFTPControlSession(unsigned long int sessionFTPControlID, const std::string filePathRemote, const std::string filePathLocal) const;
};

#endif