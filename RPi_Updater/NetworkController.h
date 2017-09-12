#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <unordered_map>

#include "SessionInfo.h"

class NetworkController
{
private:
    std::unordered_map<unsigned long int, SessionInfo*> _sessions;
    
    unsigned long int _nextSessionID;
	
public:
    NetworkController(void);
    ~NetworkController(void);
    
    unsigned long int connectToServer(const std::string servername, const std::string port);
    void disconnectFromServer(const unsigned long int sessionID);
    void disconnectFromServerAll(void);
	
    long int sendBufferWithSession(const unsigned long int sessionID, const unsigned char* inputBuffer, const unsigned long int bufferLength) const;
    long int receiveBufferWithSession(const unsigned long int sessionID, unsigned char* outputBuffer, const unsigned long int bufferLength) const;
};

#endif