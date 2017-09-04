#ifndef FTPEXCEPTIONS_H
#define FTPEXCEPTIONS_H

#include <exception>
#include <sstream>
#include <string>

namespace FTPExceptions
{
    // EXCEPTION_NET_FTP_0

    class LoginResponseException: public std::exception
    {
    private:
        const std::string _servername;
        const std::string _username;
	
    public:
        LoginResponseException(const std::string servername, const std::string username):
            _servername(servername),
            _username(username) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_0: Failed to login to server: " << _servername << ", with username: " << _username << " (NO RESPONSE)";
		
            return messageStream.str().c_str();
        }
    };
    
    // EXCEPTION_NET_FTP_1

    class LoginStatusException: public std::exception
    {
    private:
        const std::string _servername;
        const std::string _username;
        
        const unsigned short int _statusCode;
	
    public:
        LoginStatusException(const std::string servername, const std::string username, const unsigned short int statusCode):
            _servername(servername),
            _username(username),
            _statusCode(statusCode) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_1: Failed to login to server: " << _servername << ", with username: " << _username << ". Status code: " << _statusCode;
		
            return messageStream.str().c_str();
        }
    };
    
    // EXCEPTION_NET_FTP_2

    class PassiveModeResponseException: public std::exception
    {
    private:
        const std::string _servername;
        const std::string _username;
	
    public:
        PassiveModeResponseException(const std::string servername, const std::string username):
            _servername(servername),
            _username(username) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_2: Failed to retrieve a passive FTP port from server: " << _servername << ", with username: " << _username << " (NO RESPONSE)";
		
            return messageStream.str().c_str();
        }
    };
    
    // EXCEPTION_NET_FTP_3

    class PassiveModeStatusException: public std::exception
    {
    private:
        const std::string _servername;
        const std::string _username;
        
        const unsigned short int _statusCode;
	
    public:
        PassiveModeStatusException(const std::string servername, const std::string username, const unsigned short int statusCode):
            _servername(servername),
            _username(username),
            _statusCode(statusCode) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_3: Failed to retrieve a passive FTP port from server: " << _servername << ", with username: " << _username << ". Status code: " << _statusCode;
		
            return messageStream.str().c_str();
        }
    };
}

#endif