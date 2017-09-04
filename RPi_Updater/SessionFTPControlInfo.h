#ifndef SESSIONFTPCONTROLINFO_H
#define SESSIONFTPCONTROLINFO_H

#include <string>

struct SessionFTPControlInfo
{	
    const unsigned long int SessionNetworkID;
    
    const std::string Servername;
    const std::string Username;
    
    unsigned long int SessionFTPDataID;
    
    // Constructor.
    
    SessionFTPControlInfo(const unsigned long int sessionNetworkID, const std::string servername, const std::string username):
        SessionNetworkID(sessionNetworkID),
        Servername(servername),
        Username(username) {}
    
    // Copy constructor.
    
    SessionFTPControlInfo(const SessionFTPControlInfo& lref):
        SessionNetworkID(lref.SessionNetworkID),
        Servername(lref.Servername),
        Username(lref.Username),
        SessionFTPDataID(lref.SessionFTPDataID) {}
};

#endif