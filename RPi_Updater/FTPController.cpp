#include <chrono>
#include <cstring>
#include <fstream>
#include <thread>

#include "FTPController.h"
#include "FTPExceptions.h"

FTPController::FTPController():
    _nextSessionFTPControlID(0),
    _networkControllerPtr(nullptr)
{ 
    _networkControllerPtr = new NetworkController();
}

FTPController::~FTPController()
{  
    std::unordered_map<unsigned long int, SessionFTPControlInfo*>::const_iterator it;
    
    for (it = _sessionsFTPControl.begin(); it != _sessionsFTPControl.end(); ++it)
        if (it->second != nullptr)
            delete it->second;
    
    if (_networkControllerPtr != nullptr)
        delete _networkControllerPtr;
}

unsigned long int FTPController::startFTPControlSession(const std::string servername, const std::string username, const std::string password, const unsigned long int responseWaitInMs)
{
    unsigned long int sessionNetworkID;
    
    std::string message;
    unsigned short int statusCode;
    
    unsigned char buffer[FTP_MESSAGE_LENGTH_MAX];
    unsigned long int bufferLength;
    
    unsigned char responseBuffer[FTP_MESSAGE_LENGTH_MAX];
    long int responseBufferLength = -1;
    
    // Send username info, with USER command.
    
    message = "USER " + username + "\n";
    bufferLength = message.length();
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);

    sessionNetworkID = _networkControllerPtr->connectToServer(servername, "21");
    
    _networkControllerPtr->sendBufferWithSession(sessionNetworkID, buffer, bufferLength);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
    
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
    
    // Send password info, with PASS command.
    
    std::memset(static_cast<void*>(buffer), 0, bufferLength);
    std::memset(static_cast<void*>(responseBuffer), 0, responseBufferLength);
        
    message = "PASS " + password + "\n";
    bufferLength = message.length();
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);
    
    _networkControllerPtr->sendBufferWithSession(sessionNetworkID, buffer, bufferLength);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
    
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

void FTPController::getFileWithFTPControlSession(const unsigned long int sessionFTPControlID, const std::string fileRemotePath, const std::string filePath, const unsigned long int responseWaitInMs, const unsigned long int packetWaitInMs) const
{
    unsigned char* fileBuffer = nullptr;
    unsigned long int fileLength;
    
    unsigned long int fileRemoteLength;
    
    SessionFTPControlInfo sessionFTPControlInfo = *_sessionsFTPControl.at(sessionFTPControlID);
    unsigned long int sessionNetworkID;
    
    std::string message;
    unsigned short int statusCode;
    
    unsigned char buffer[FTP_MESSAGE_LENGTH_MAX];
    unsigned long int bufferLength;
    
    unsigned char responseBuffer[FTP_MESSAGE_LENGTH_MAX];
    long int responseBufferLength = -1;
    
    std::stringstream responseStream;
    
    // Set the file-transfer mode to binary, with TYPE I command.
        
    message = "TYPE I\n";
    bufferLength = message.length();
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);
        
    _networkControllerPtr->sendBufferWithSession(sessionFTPControlInfo.SessionNetworkID, buffer, bufferLength);
        
    std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
    
    responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionFTPControlInfo.SessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
    
    if (responseBufferLength <= 0)
    {
        FTPExceptions::BinaryModeResponseException e(sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username);
        throw e;
    }
    
    statusCode = getStatusCode(std::string(responseBuffer, responseBuffer + responseBufferLength));
    
    if (statusCode != 200)
    {
        FTPExceptions::BinaryModeStatusException e(sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username, statusCode);
        throw e;
    }
    
    // Get the size of the remote file, with SIZE command.
    
    std::memset(static_cast<void*>(buffer), 0, bufferLength);
    std::memset(static_cast<void*>(responseBuffer), 0, responseBufferLength);
    
    message = "SIZE " + fileRemotePath + "\n";
    bufferLength = message.length();
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);
    
    _networkControllerPtr->sendBufferWithSession(sessionFTPControlInfo.SessionNetworkID, buffer, bufferLength);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
    
    responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionFTPControlInfo.SessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
    
    if (responseBufferLength <= 0)
    {
        FTPExceptions::GetSizeResponseException e(fileRemotePath, sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username);
        throw e;
    }
    
    responseStream.str(std::string(responseBuffer, responseBuffer + responseBufferLength));
    
    responseStream >> statusCode;
    
    if (statusCode != 213)
    {
        FTPExceptions::GetSizeStatusException e(fileRemotePath, sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username, statusCode);
        throw e;
    }
    
    responseStream >> fileRemoteLength;
    
    // Connect to a server-side passive-mode port.
    
    std::stringstream portStream;
    portStream << getFTPDataPort(sessionFTPControlID, responseWaitInMs);
    sessionNetworkID = _networkControllerPtr->connectToServer(sessionFTPControlInfo.Servername, portStream.str());
    
    // Retrieve the specified file, with RETR command.
    
    std::memset(static_cast<void*>(buffer), 0, bufferLength);
    std::memset(static_cast<void*>(responseBuffer), 0, responseBufferLength);
    
    message = "RETR " + fileRemotePath + "\n";
    bufferLength = message.length();
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);
    
    _networkControllerPtr->sendBufferWithSession(sessionFTPControlInfo.SessionNetworkID, buffer, bufferLength);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
    
    responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionFTPControlInfo.SessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
    
    if (responseBufferLength <= 0)
    {
        _networkControllerPtr->disconnectFromServer(sessionNetworkID);
        
        FTPExceptions::GetFileResponseException e(fileRemotePath, sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username);
        throw e;
    }
    
    responseStream.clear();
    responseStream.str(std::string(responseBuffer, responseBuffer + responseBufferLength));
    
    std::string lineCurrent;
    
    std::getline(responseStream, lineCurrent);
    
    statusCode = getStatusCode(lineCurrent);
    
    if (statusCode != 150)
    {
        _networkControllerPtr->disconnectFromServer(sessionNetworkID);
        
        FTPExceptions::GetFileStatusException e(fileRemotePath, sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username, statusCode);
        throw e;
    }
    
    bool isDone = false;
    
    while (std::getline(responseStream, lineCurrent))
    {
        statusCode = getStatusCode(lineCurrent);
        
        if (statusCode == 226)
            isDone = true;
        
        else
        {
            FTPExceptions::GetFileStatusException e(fileRemotePath, sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username, statusCode);
            throw e;
        }
    }
    
    std::memset(static_cast<void*>(responseBuffer), 0, responseBufferLength);
    
    // File-transfer loop.
    
    fileBuffer = new unsigned char[FTP_FILE_LENGTH_MAX];
    fileLength = 0;
    
    try
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(packetWaitInMs));
            
            responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
            
            if (responseBufferLength > 0)
            {
                std::memcpy(static_cast<void*>(fileBuffer + fileLength), static_cast<void*>(responseBuffer), responseBufferLength);
                
                fileLength += responseBufferLength;
                
                std::memset(static_cast<void*>(responseBuffer), 0, responseBufferLength);
                
                continue;
            }
            
            try
            {
                unsigned char ping[1]{0};
                
                _networkControllerPtr->sendBufferWithSession(sessionNetworkID, ping, 1);
            }
            
            catch (const std::exception&)
            {
                if (!isDone)
                {
                    responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionFTPControlInfo.SessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
                
                    if (responseBufferLength > 0)
                    {
                        statusCode = getStatusCode(std::string(responseBuffer, responseBuffer + responseBufferLength));
            
                        if (statusCode != 226)
                        {
                            FTPExceptions::GetFileStatusException e(fileRemotePath, sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username, statusCode);
                            throw e;
                        }
                    }
                }
                
                if (fileLength != fileRemoteLength)
                {           
                    FTPExceptions::GetFileSizeException e(fileRemotePath, sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username);
                    throw e;
                }
                
                break;
            }
        }
    }
    
    catch (const std::exception& e)
    {
        if (fileBuffer != nullptr)
            delete[] fileBuffer;
        
        _networkControllerPtr->disconnectFromServer(sessionNetworkID);
        
        throw;
    }
    
    _networkControllerPtr->disconnectFromServer(sessionNetworkID);
    
    // Write the retrieved file to disk.
    
    std::ofstream file(filePath, std::ofstream::binary);
    
    // REINTERPRET_CAST!
    
    file.write(reinterpret_cast<char*>(fileBuffer), fileLength);
    
    file.close();
    
    if (fileBuffer != nullptr)
        delete[] fileBuffer;
}

unsigned short int FTPController::getFTPDataPort(const unsigned long int sessionFTPControlID, const unsigned long int responseWaitInMs) const
{
    unsigned short int port;
    
    SessionFTPControlInfo sessionFTPControlInfo = *_sessionsFTPControl.at(sessionFTPControlID);
    
    std::string message;
    unsigned short int statusCode;
    
    unsigned char buffer[FTP_MESSAGE_LENGTH_MAX];   
    unsigned long int bufferLength;
    
    unsigned char responseBuffer[FTP_MESSAGE_LENGTH_MAX];    
    long int responseBufferLength = -1;
    
    // Request server-side passive-mode ports, with PASV command.
    
    message = "PASV\n"; 
    bufferLength = message.length();
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(message.c_str()), bufferLength);
                
    _networkControllerPtr->sendBufferWithSession(sessionFTPControlInfo.SessionNetworkID, buffer, bufferLength);
                
    std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
        
    responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionFTPControlInfo.SessionNetworkID, responseBuffer, FTP_MESSAGE_LENGTH_MAX);
    
    if (responseBufferLength <= 0)
    {
        FTPExceptions::PassiveModeResponseException e(sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username);
        throw e;
    }
     
    statusCode = getStatusCode(std::string(responseBuffer, responseBuffer + responseBufferLength));
        
    if (statusCode != 227)
    {
        FTPExceptions::PassiveModeStatusException e(sessionFTPControlInfo.Servername, sessionFTPControlInfo.Username, statusCode);
        throw e;
    }
    
    // Get server-side passive-mode port from response.
        
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
    
    port = (byte1 * 256) + byte2;
    
    return port;
}

unsigned short int FTPController::getStatusCode(const std::string response)
{
    return static_cast<unsigned short int>(std::stoul(response.substr(0, 3)));
}