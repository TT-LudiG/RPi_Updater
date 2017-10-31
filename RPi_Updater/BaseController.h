#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "FTPController.h"
#include "HTTPResponse.h"

class BaseController
{
private:
    NetworkController* _networkControllerPtr;
    FTPController* _ftpControllerPtr;
    
    const std::string _servername_general;
    const std::string _port_general;
    const unsigned long int _delay_updater_loop_in_sec;
    const std::string _name_common_prefix;
    
    unsigned long int _id;

    std::atomic<bool> _isDone;
    std::atomic<bool> _isScanning;

    std::mutex _mutex;
    std::atomic<bool> _isReady;
    std::atomic<bool> _isWaiting;
    std::atomic<bool> _hasWoken;

    std::condition_variable _cv;
    
    static std::string runBashCommandWithOutput(const std::string command);
    
    unsigned long int getID(void) const;
    
    HTTPResponse sendGETToServerURI(const std::string servername, const std::string port, const std::string uri, const unsigned long int responseWaitInMs) const;
    HTTPResponse sendPOSTToServerURI(const std::string servername, const std::string port, const std::string uri, const std::string body, const unsigned long int responseWaitInMs) const;
    
    static bool configureOpenVPN(const std::string commonName, const std::string directoryPath, const std::string fileName);
    
    static bool fileExists(const std::string directoryName, const std::string fileName);
    
    static void logToFileWithSubdirectory(const std::string message, const std::string subdirectoryName);
    
    static std::unordered_map<std::string, std::string> getJSONPairs(const std::string jsonString);    
    static std::string getMACAddress(void);
    static std::string getTimeString_Now(const std::string format);
    
public:
    BaseController(const std::string servername_general, const std::string port_general, const unsigned long int delay_updater_loop_in_sec, const std::string name_common_prefix);
    ~BaseController(void);
    
    void monitorUpdaterThread(void);
    
    void checkUpdatesPeriodically(void);
    
    void setFinalised(void);
    bool getFinalised(void) const;
};
#endif