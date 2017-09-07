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
    
    // EXCEPTION_NET_FTP_4

    class BinaryModeResponseException: public std::exception
    {
    private:
        const std::string _servername;
        const std::string _username;
	
    public:
        BinaryModeResponseException(const std::string servername, const std::string username):
            _servername(servername),
            _username(username) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_4: Failed to set binary file-transfer mode for server: " << _servername << ", with username: " << _username << " (NO RESPONSE)";
		
            return messageStream.str().c_str();
        }
    };
    
    // EXCEPTION_NET_FTP_5

    class BinaryModeStatusException: public std::exception
    {
    private:
        const std::string _servername;
        const std::string _username;
        
        const unsigned short int _statusCode;
	
    public:
        BinaryModeStatusException(const std::string servername, const std::string username, const unsigned short int statusCode):
            _servername(servername),
            _username(username),
            _statusCode(statusCode) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_5: Failed to set binary file-transfer mode for server: " << _servername << ", with username: " << _username << ". Status code: " << _statusCode;
		
            return messageStream.str().c_str();
        }
    };
    
    // EXCEPTION_NET_FTP_6

    class GetSizeResponseException: public std::exception
    {
    private:
        const std::string _filePath;
        const std::string _servername;
        const std::string _username;
	
    public:
        GetSizeResponseException(const std::string filePath, const std::string servername, const std::string username):
            _filePath(filePath),
            _servername(servername),
            _username(username) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_6: Failed to retrieve filesize for file: " << _filePath << ", at server: " << _servername << ", with username: " << _username << " (NO RESPONSE)";
		
            return messageStream.str().c_str();
        }
    };
    
    // EXCEPTION_NET_FTP_7

    class GetSizeStatusException: public std::exception
    {
    private:
        const std::string _filePath;
        const std::string _servername;
        const std::string _username;
        
        const unsigned short int _statusCode;
	
    public:
        GetSizeStatusException(const std::string filePath, const std::string servername, const std::string username, const unsigned short int statusCode):
            _filePath(filePath),
            _servername(servername),
            _username(username),
            _statusCode(statusCode) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_7: Failed to retrieve filesize for file: " << _filePath << ", at server: " << _servername << ", with username: " << _username << ". Status code: " << _statusCode;
		
            return messageStream.str().c_str();
        }
    };
    
    // EXCEPTION_NET_FTP_8

    class GetFileResponseException: public std::exception
    {
    private:
        const std::string _filePath;
        const std::string _servername;
        const std::string _username;
	
    public:
        GetFileResponseException(const std::string filePath, const std::string servername, const std::string username):
            _filePath(filePath),
            _servername(servername),
            _username(username) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_8: Failed to retrieve file: " << _filePath << ", at server: " << _servername << ", with username: " << _username << " (NO RESPONSE)";
		
            return messageStream.str().c_str();
        }
    };
    
    // EXCEPTION_NET_FTP_9

    class GetFileStatusException: public std::exception
    {
    private:
        const std::string _filePath;
        const std::string _servername;
        const std::string _username;
        
        const unsigned short int _statusCode;
	
    public:
        GetFileStatusException(const std::string filePath, const std::string servername, const std::string username, const unsigned short int statusCode):
            _filePath(filePath),
            _servername(servername),
            _username(username),
            _statusCode(statusCode) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_9: Failed to retrieve file: " << _filePath << ", at server: " << _servername << ", with username: " << _username << ". Status code: " << _statusCode;
		
            return messageStream.str().c_str();
        }
    };
    
    // EXCEPTION_NET_FTP_10

    class GetFileSizeException: public std::exception
    {
    private:
        const std::string _filePath;
        const std::string _servername;
        const std::string _username;
	
    public:
        GetFileSizeException(const std::string filePath, const std::string servername, const std::string username):
            _filePath(filePath),
            _servername(servername),
            _username(username) {}
	
        virtual const char* what() const throw()
        {
            std::stringstream messageStream;
            
            messageStream << "EXCEPTION_NET_FTP_10: Inconsistency between remote and local filesize for file: " << _filePath << ", at server: " << _servername << ", with username: ";
		
            return messageStream.str().c_str();
        }
    };
}

#endif