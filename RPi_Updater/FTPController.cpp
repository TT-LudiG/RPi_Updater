#include <chrono>
#include <cstring>
#include <thread>

#include "FTPController.h"
#include "FTPExceptions.h"

#include <iostream>

FTPController::FTPController() : _nextSessionFTPControlID(0), _nextSessionFTPDataID(0)
{
    _networkControllerPtr = new NetworkController();
}

FTPController::~FTPController()
{
    {
        std::unordered_map<unsigned long int, unsigned long int>::const_iterator it;
    
        for (it = _sessionsFTPData.begin(); it != _sessionsFTPData.end(); ++it)
            _networkControllerPtr->disconnectFromServer(it->second);
    }
    
    {
        std::unordered_map<unsigned long int, SessionFTPControlInfo*>::const_iterator it;
    
        for (it = _sessionsFTPControl.begin(); it != _sessionsFTPControl.end(); ++it)
        {
            if (it->second == nullptr)
                continue;
            
            _networkControllerPtr->disconnectFromServer(it->second->SessionNetworkID);
            
            delete it->second;
        }
    }
    
    if (_networkControllerPtr != nullptr)
        delete _networkControllerPtr;
}

unsigned long int FTPController::startFTPControlSession(const std::string servername, const std::string username, const std::string password)
{
    unsigned long int sessionNetworkID;
    
    unsigned short int statusCode;
    
    unsigned char responseBuffer[FTP_MESSAGE_LENGTH_MAX];
    
    long int responseBufferLength = -1;
    
    std::string message = "USER " + username + "\n";
    
    unsigned char buffer[FTP_MESSAGE_LENGTH_MAX];
    
    unsigned long int bufferLength = message.length();
    
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);

    sessionNetworkID = _networkControllerPtr->connectToServer(servername, "21");
                
    _networkControllerPtr->sendBufferWithSession(sessionNetworkID, buffer, bufferLength);
                
    std::this_thread::sleep_for(std::chrono::seconds(1));
        
    responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
    
    if (responseBufferLength <= 0)
    {
        FTPExceptions::LoginResponseException e(servername, username);
        throw e;
    }
    
    std::stringstream responseStream(std::string(responseBuffer, responseBuffer + responseBufferLength));
        
    std::string lineCurrent;
        
    for (unsigned short int i = 0; i < 4; ++i)
        std::getline(responseStream, lineCurrent);
        
    statusCode = getStatusCode(lineCurrent);
        
    if (statusCode != 331)
    {
        FTPExceptions::LoginStatusException e(servername, username, statusCode);
        throw e;
    }
        
    std::memset(static_cast<void*>(buffer), 0, bufferLength);
    std::memset(static_cast<void*>(responseBuffer), 0, responseBufferLength);
        
    message = "PASS " + password + "\n";
        
    bufferLength = message.length();
        
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);
        
    _networkControllerPtr->sendBufferWithSession(sessionNetworkID, buffer, bufferLength);
        
    std::this_thread::sleep_for(std::chrono::seconds(1));
        
    responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
    
    if (responseBufferLength <= 0)
    {
        FTPExceptions::LoginResponseException e(servername, username);
        throw e;
    }
        
    statusCode = getStatusCode(std::string(responseBuffer, responseBuffer + responseBufferLength));
        
    if (statusCode != 230)
    {
        FTPExceptions::LoginStatusException e(servername, username, statusCode);
        throw e;
    }
    
    _sessionsFTPControl.insert(std::pair<unsigned long int, SessionFTPControlInfo*>(_nextSessionFTPControlID, new SessionFTPControlInfo(sessionNetworkID, servername, username)));
    
    return _nextSessionFTPControlID++;
}

void FTPController::startFTPDataSession(const unsigned long int sessionFTPControlID)
{
    SessionFTPControlInfo sessionFTPControlInfo = *_sessionsFTPControl.at(sessionFTPControlID);
    
    unsigned char responseBuffer[FTP_MESSAGE_LENGTH_MAX];
    
    long int responseBufferLength = -1;
    
    std::string message = "PASV\n";
    
    unsigned char buffer[FTP_MESSAGE_LENGTH_MAX];
    
    unsigned long int bufferLength = message.length();
    
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);
                
    _networkControllerPtr->sendBufferWithSession(sessionFTPControlInfo.SessionNetworkID, buffer, bufferLength);
                
    std::this_thread::sleep_for(std::chrono::seconds(1));
        
    responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionFTPControlInfo.SessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
    
    if (responseBufferLength <= 0)
    {
        FTPExceptions::PassiveModeResponseException e(sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username);
        throw e;
    }
     
    unsigned short int statusCode = getStatusCode(std::string(responseBuffer, responseBuffer + responseBufferLength));
        
    if (statusCode != 227)
    {            
        FTPExceptions::PassiveModeStatusException e(sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username, statusCode);
        throw e;
    }
        
    std::string responseString = std::string(responseBuffer, responseBuffer + responseBufferLength);
        
    unsigned long int tokensStringLength = responseString.length() - responseString.find_first_of('(') - (responseString.length() - responseString.find_first_of(')'));
        
    std::string tokensString = responseString.substr(responseString.find_first_of('(') + 1, tokensStringLength);
        
    std::stringstream tokensStream(tokensString);
        
    std::string token;
    
    unsigned char byte1;
    unsigned char byte2;
        
    for (unsigned short int i = 0; i < 6; ++i)
    {
        std::getline(tokensStream, token, ',');
        
        if (i == 4)
            byte1 = static_cast<unsigned char>(std::stoul(token));
        
        else if (i == 5)
            byte2 = static_cast<unsigned char>(std::stoul(token));
    }
    
    unsigned short int port = (byte1 * 256) + byte2;
    
    std::stringstream portStream;
    
    portStream << port;
        
    unsigned long int sessionNetworkID = _networkControllerPtr->connectToServer(sessionFTPControlInfo.Servername, portStream.str());
    
    // Register new FTP data session.
        
    _sessionsFTPData.insert(std::pair<unsigned long int, unsigned long int>(_nextSessionFTPDataID, sessionNetworkID));
        
    _sessionsFTPControl.at(sessionFTPControlID)->SessionFTPDataID = _nextSessionFTPDataID++;
}

void FTPController::getFileWithFTPControlSession(unsigned long int sessionFTPControlID)
{
    unsigned long int sessionFTPDataID = _sessionsFTPControl.at(sessionFTPControlID)->SessionFTPDataID;    
    unsigned long int sessionNetworkID = _sessionsFTPData.at(sessionFTPDataID);
    
    unsigned char responseBuffer[FTP_MESSAGE_LENGTH_MAX];
    
    long int responseBufferLength = -1;
    
    std::string message = "LIST\n";
    
    unsigned char buffer[FTP_MESSAGE_LENGTH_MAX];
    
    unsigned long int bufferLength = message.length();
    
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);
                
    _networkControllerPtr->sendBufferWithSession(sessionNetworkID, buffer, bufferLength);
                
    std::this_thread::sleep_for(std::chrono::seconds(1));
        
    responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
    
    std::string responseString = std::string(responseBuffer, responseBuffer + responseBufferLength);
    
    std::cout << responseString << std::endl;
}

unsigned short int FTPController::getStatusCode(const std::string response) const
{
    return static_cast<unsigned short int>(std::stoul(response.substr(0, 3)));
}