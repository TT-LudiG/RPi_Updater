#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "BaseController.h"
#include "HTTPRequest_GET.h"
#include "HTTPRequest_POST.h"

BaseController::BaseController(const std::string servername, const std::string port_general, const std::string port_temperature, const unsigned long int delay_updater_loop_in_sec):
    _servername(servername),
    _port_general(port_general),
    _port_temperature(port_temperature),
    _delay_updater_loop_in_sec(delay_updater_loop_in_sec),
    _networkControllerPtr(nullptr),
    _ftpControllerPtr(nullptr)
{
    try
    {
        _networkControllerPtr = new NetworkController();
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "Network");
        
        throw e;
    }
    
    try
    {
        _ftpControllerPtr = new FTPController();
    }
    
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "FTP");
        
        throw e;
    }
    
    _isDone = false;
    _isScanning = false;
    
    _isReady = false;
    _isWaiting = false;
    _hasWoken = false;
}

BaseController::~BaseController(void)
{
    if (_ftpControllerPtr != nullptr)
        delete _ftpControllerPtr;
}

void BaseController::monitorUpdaterThread(void)
{
    std::unique_lock<std::mutex> lock(_mutex);

    while (!_isDone)
    {
        // Reset the conditional wait barrier for all client-listener threads.

        if ((_isReady) && (_hasWoken))
        {
            lock.lock();

            _isReady = false;
            
            _hasWoken = false;
        }

        if ((!_isReady) && (_isWaiting))
        {
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            
            // All client-listener threads wait for _delaySenderLoopInSec seconds (interruptible).
                
            while (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() < _delay_updater_loop_in_sec)
                if (_isDone)
                    break;
            
            _isReady = true;

            lock.unlock();

            _cv.notify_all();

            _isWaiting = false;
        }
    }
    
    _isReady = true;
}

void BaseController::checkUpdatesPeriodically(void)
{    
    std::unique_lock<std::mutex> lock(_mutex);
    
    lock.unlock();
    
    // First attempt to get the ID.
    
    _id = getID();
    
    while (!_isDone)
    {
        // Subsequent attempts to get the ID, if not yet found.
        
        if (_id == 0)
            _id = getID();
        
        else
        {
            // Send a VPN security certificate and key request to the ThermoTrack_API_BLE_General Web API.
            
            std::stringstream uriStream;
            uriStream << "/api/blereaders/vpn" << _id;
            
            HTTPResponse response = sendGETToServerURI(_servername, _port_general, uriStream.str(), 2000);
            
            if ((response.statusCode == 200) && (response.contentLength > 0))
            {
                std::string isReadyVPNFiles = std::string(response.content, response.content + response.contentLength);
                
                if (isReadyVPNFiles == "false")
                    continue;
                
                std::stringstream nameCommonStream;
                nameCommonStream << "client-rpi-ble-" << _id;

                unsigned long int sessionID = _ftpControllerPtr->startFTPControlSession(_servername, "RPi-Dev", "jacoistehbawsW00T!", 500);
    
                _ftpControllerPtr->getFileWithFTPControlSession(sessionID, "TT_BLE/VPN/" + nameCommonStream.str() + ".crt", "//home/pi/" + nameCommonStream.str() + ".crt", 500, 10);
    
//                runBashCommandWithOutput("sudo tar -xzvf //home/pi/RPi_BLE_Scanner.tar.gz -C //home/pi/");
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        _isWaiting = true;
            
        lock.lock();

        while (!_isReady)
            _cv.wait(lock);
            
        _hasWoken = true;
        
        lock.unlock();
    }
}

void BaseController::setFinalised(void)
{
    _isDone = true;
}

bool BaseController::getFinalised(void) const
{
    return _isDone;
}

std::string BaseController::runBashCommandWithOutput(const std::string command)
{
    std::stringstream outputStream;
    
    char output[100];
    
    FILE* filePtrTerminal = nullptr;
    
    filePtrTerminal = popen(command.c_str(), "r");
    
    while (std::fgets(output, sizeof(output), filePtrTerminal) != 0)
        outputStream << output;
    
    pclose(filePtrTerminal);
    
    return outputStream.str();
}

unsigned long int BaseController::getID(void) const
{
    unsigned long int id = 0;
    
    // Check the config file for an existing ReaderID.
    
    if (fileExists("//home/pi/CONFIG", "RPi_BLE_Scanner.config"))
    {
        std::ifstream fileConfig("//home/pi/CONFIG/RPi_BLE_Scanner.config");
        
        if (fileConfig.is_open())
        {
            std::stringstream jsonStream;
                
            jsonStream << fileConfig.rdbuf();
                
            std::unordered_map<std::string, std::string> jsonPairs = getJSONPairs(jsonStream.str());
            
            id = std::stoul(jsonPairs.at("ReaderID"));
            
            fileConfig.close();
            
            return id;
        }
    }
    
    // Send a ReaderID request to the ThermoTrack_API_BLE_General Web API.
    
    std::string uri = "/api/blereaders/" + getMACAddress();
    
    HTTPResponse response = sendGETToServerURI(_servername, _port_general, uri, 2000);
    
    // Write the ReaderID response to the config file.
    
    if ((response.statusCode == 200) && (response.contentLength > 0))
    {
        id = std::stoul(std::string(response.content, response.content + response.contentLength));
            
        std::ofstream fileConfig("//home/pi/CONFIG/RPi_BLE_Scanner.config");
            
        fileConfig << "{ ReaderID: " << id << " }";
    }
    
    return id;
}

HTTPResponse BaseController::sendGETToServerURI(const std::string servername, const std::string port, const std::string uri, const unsigned long int responseWaitInMs) const
{
    HTTPResponse response;
    
    unsigned char responseBuffer[HTTP_REQUEST_LENGTH_MAX];
    
    long int responseBufferLength = -1;
    
    HTTPRequest_GET message(uri, servername + ":" + port, "close", "application/json");
            
    unsigned char buffer[HTTP_REQUEST_LENGTH_MAX];
            
    unsigned long int bufferLength = message.serialise(buffer, sizeof(buffer));
    
    try
    {
        unsigned long int sessionID = _networkControllerPtr->connectToServer(servername, port);
                
        _networkControllerPtr->sendBufferWithSession(sessionID, buffer, bufferLength);
                
        std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
        
        responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionID, responseBuffer, HTTP_REQUEST_LENGTH_MAX);
                
        _networkControllerPtr->disconnectFromServer(sessionID);
    }
            
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "Network");
    }
    
    if (responseBufferLength > 0)
        response.deserialise(responseBuffer, responseBufferLength);
    
    return response;
}

HTTPResponse BaseController::sendPOSTToServerURI(const std::string servername, const std::string port, const std::string uri, const std::string body, const unsigned long int responseWaitInMs) const
{
    HTTPResponse response;
    
    unsigned char responseBuffer[HTTP_REQUEST_LENGTH_MAX];
    
    long int responseBufferLength = -1;
    
    HTTPRequest_POST message(uri, servername + ":" + port, "close", "application/json");
    
    unsigned char buffer[HTTP_REQUEST_LENGTH_MAX];
    
    unsigned long int bufferLength = body.length();
    
    std::memcpy(static_cast<void*>(buffer), static_cast<const void*>(body.c_str()), bufferLength);
    
    message.setContent(buffer, bufferLength);
    
    std::memset(static_cast<void*>(buffer), 0, bufferLength);
    
    bufferLength = message.serialise(buffer, sizeof(buffer));
    
    try
    {
        unsigned long int sessionID = _networkControllerPtr->connectToServer(servername, port);
                
        _networkControllerPtr->sendBufferWithSession(sessionID, buffer, bufferLength);
                
        std::this_thread::sleep_for(std::chrono::milliseconds(responseWaitInMs));
        
        responseBufferLength = _networkControllerPtr->receiveBufferWithSession(sessionID, responseBuffer, HTTP_REQUEST_LENGTH_MAX);
                
        _networkControllerPtr->disconnectFromServer(sessionID);
    }
            
    catch (const std::exception& e)
    {
        logToFileWithSubdirectory(e, "Network");
    }
    
    if (responseBufferLength > 0)
        response.deserialise(responseBuffer, responseBufferLength);
    
    return response;
}

void BaseController::logToFileWithSubdirectory(const std::exception& e, const std::string subdirectoryName)
{
    std::stringstream fileLogNameStream;
        
    fileLogNameStream << "//home/pi/LOGS/RPi_Updater/" << subdirectoryName;
    
    umask(0);
    mkdir("//home/pi/LOGS", 0755);
    mkdir("//home/pi/LOGS/RPi_Updater", 0755);
    mkdir(fileLogNameStream.str().c_str(), 0755);
        
    fileLogNameStream << "/" << getTimeString_Now("%F_%T") << ".log";
        
    std::ofstream fileLog(fileLogNameStream.str());
    
    if (fileLog.is_open())
        fileLog << e.what() << std::endl;
        
    fileLog.close();
}

std::unordered_map<std::string, std::string> BaseController::getJSONPairs(const std::string jsonString)
{
    std::unordered_map<std::string, std::string> jsonPairs;
    
    std::size_t indexBraceFirst, indexBraceLast;
    
    indexBraceFirst = jsonString.find_first_of('{');
    indexBraceLast = jsonString.find_last_of('}');
    
    std::string bodyString = jsonString.substr(indexBraceFirst + 1, indexBraceLast - (indexBraceFirst + 1));
    
    std::stringstream bodyStream(bodyString);
    
    std::string jsonPairCurrent;
    
    while (std::getline(bodyStream, jsonPairCurrent, ','))
    {
        std::size_t indexColon = jsonPairCurrent.find(':');
        
        std::string key = jsonPairCurrent.substr(0, indexColon);
        
        if (key.find('"') != std::string::npos)
        {
            std::size_t indexQuotationFirst, indexQuotationLast;
            
            indexQuotationFirst = key.find_first_of('"');
            indexQuotationLast = key.find_last_of('"');
            
            key = key.substr(indexQuotationFirst + 1, indexQuotationLast - (indexQuotationFirst + 1));
        }
        
        else
        {
            std::stringstream keyStream(key);
            
            keyStream >> key;
        }
        
        std::string value = jsonPairCurrent.substr(indexColon + 1);
        
        if (value.find('"') != std::string::npos)
        {
            std::size_t indexQuotationFirst, indexQuotationLast;
            
            indexQuotationFirst = value.find_first_of('"');
            indexQuotationLast = value.find_last_of('"');
            
            value = value.substr(indexQuotationFirst + 1, indexQuotationLast - (indexQuotationFirst + 1));
        }
        
        else
        {
            std::stringstream valueStream(value);
            
            valueStream >> value;
        }
        
        jsonPairs.emplace(key, value);
    }
    
    return jsonPairs;
}

std::string BaseController::getMACAddress(void)
{
    std::string addressMAC = "";
    
    std::ifstream fileMACAddress("//sys/class/net/eth0/address");
    
    if (fileMACAddress.is_open())
        fileMACAddress >> addressMAC;
    
    fileMACAddress.close();
    
    std::transform(addressMAC.begin(), addressMAC.end(), addressMAC.begin(), ::tolower);
    
    addressMAC.erase(std::remove(addressMAC.begin(), addressMAC.end(), ':'), addressMAC.end());
    
    return addressMAC;
}

std::string BaseController::getTimeString_Now(const std::string format)
{
    std::string timeString = "";
    
    time_t timeRaw;
            
    std::time(&timeRaw);
            
    struct tm timeInfo = *std::localtime(&timeRaw);
            
    char timeBuffer[20];
            
    std::strftime(timeBuffer, 20, format.c_str(), &timeInfo);
    
    timeString = std::string(timeBuffer);
    
    return timeString;
}