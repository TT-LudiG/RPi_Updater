#include <cerrno>
#include <cstring>

#include <arpa/inet.h>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "NetworkController.h"
#include "NetworkExceptions.h"

NetworkController::NetworkController(void): _nextSessionID(0) {}

NetworkController::~NetworkController(void)
{
    disconnectFromServerAll();
}

unsigned long int NetworkController::connectToServer(const std::string servername, const std::string port)
{
    long int socketHandle = -1;
    
    // Initialise the socket address.
    
    struct sockaddr_in* socketAddress;
    
    struct addrinfo* result;
    
    unsigned long int status = getaddrinfo(servername.c_str(), port.c_str(), nullptr, &result);
    
    if (status != 0)
    {
        NetworkExceptions::ServerLookupException e(servername, std::string(gai_strerror(status)));
        throw e;
    }
    
    for (struct addrinfo* it = result; it != NULL; it = it->ai_next)
        socketAddress = (struct sockaddr_in*)it->ai_addr;
    
    // Create the socket.
    
    socketHandle = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    
    if (socketHandle < 0)
    {
        NetworkExceptions::SocketCreateException e(socketHandle, std::string(std::strerror(errno)));
        throw e;
    }
    
    // Free the memory allocated for the "addrinfo" struct.
    
    freeaddrinfo(result);
    
    // Set the socket timeout period (for the server-connect operation).
    
    struct timeval timeout;
    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    // REINTERPRET_CAST!

    if (setsockopt(socketHandle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions::SocketSetOptionException e(socketHandle, "SO_SNDTIMEO", std::string(std::strerror(errno)));
        throw e;
    }
    
    // REINTERPRET_CAST!
    
    if (setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions::SocketSetOptionException e(socketHandle, "SO_RCVTIMEO", std::string(std::strerror(errno)));
        throw e;
    }
	
    // Connect to the server.
    
    // REINTERPRET_CAST!
	
    if (connect(socketHandle, reinterpret_cast<const struct sockaddr*>(socketAddress), sizeof(*socketAddress)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions::ServerConnectException e(servername, port, std::string(std::strerror(errno)));
        throw e;
    }
    
    // Set the socket timeout period (for subsequent read/write operations).
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    
    // REINTERPRET_CAST!

    if (setsockopt(socketHandle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions::SocketSetOptionException e(socketHandle, "SO_SNDTIMEO", std::string(std::strerror(errno)));
        throw e;
    }
    
    // REINTERPRET_CAST!
    
    if (setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout)) < 0)
    {
        close(socketHandle);
        
        NetworkExceptions::SocketSetOptionException e(socketHandle, "SO_RCVTIMEO", std::string(std::strerror(errno)));
        throw e;
    }
    
    _sessions.insert(std::pair<unsigned long int, SessionInfo*>(_nextSessionID, new SessionInfo(socketHandle, *socketAddress)));
    
    return _nextSessionID++;
}

void NetworkController::disconnectFromServer(const unsigned long int sessionID)
{ 
    if (_sessions.count(sessionID) > 0)
    {      
        close(_sessions.at(sessionID)->SocketHandle);
        
        if (_sessions.at(sessionID) != nullptr)
            delete _sessions.at(sessionID);
        
        _sessions.erase(sessionID);
    }
}

void NetworkController::disconnectFromServerAll(void)
{
    std::unordered_map<unsigned long int, SessionInfo*>::iterator it;
    
    for (it = _sessions.begin(); it != _sessions.end();)
    {     
        close(_sessions.at(it->first)->SocketHandle);
        
        if (_sessions.at(it->first) != nullptr)
            delete _sessions.at(it->first);
        
        it = _sessions.erase(it);
    }
}

long int NetworkController::sendBufferWithSession(const unsigned long int sessionID, const unsigned char* inputBuffer, const unsigned long int bufferLength) const
{
    int bytesCount = -1;
    
    if (_sessions.count(sessionID) > 0)
    {
        long int socketHandle = _sessions.at(sessionID)->SocketHandle;
        
        bytesCount = send(socketHandle, static_cast<const void*>(inputBuffer), bufferLength, MSG_NOSIGNAL);
        
        if (bytesCount < 0)
        {
            if (errno == EAGAIN)
                bytesCount = 0;
            
            else
            {
                NetworkExceptions::SocketWriteException e(socketHandle, std::string(std::strerror(errno)));
                throw e;
            }
        }
    }
    
    return bytesCount;
}

long int NetworkController::receiveBufferWithSession(const unsigned long int sessionID, unsigned char* outputBuffer, const unsigned long int bufferLength) const
{
    int bytesCount = -1;

    if (_sessions.count(sessionID) > 0)
    {
        long int socketHandle = _sessions.at(sessionID)->SocketHandle;
        
        bytesCount = read(socketHandle, static_cast<void*>(outputBuffer), bufferLength);
	
        if (bytesCount < 0)
        {
            if (errno == EAGAIN)
                bytesCount = 0;
            
            else
            {
                NetworkExceptions::SocketReadException e(socketHandle, std::string(std::strerror(errno)));
                throw e;
            }
        }
    }
    
    return bytesCount;
}