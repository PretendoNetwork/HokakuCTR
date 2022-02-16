#include "RMCLogger.hpp"
#include "time.h"

namespace CTRPluginFramework {
    void RMCLogger::Initialize() {
        std::string finalFolder = "/RMCDump";
        if (!Directory::IsExists(finalFolder))
            Directory::Create(finalFolder);

        std::string tid;
        Process::GetTitleID(tid);
        std::string procName;
        Process::GetName(procName);

        finalFolder += "/" + procName + " - (" + tid + ")";
        if (!Directory::IsExists(finalFolder))
            Directory::Create(finalFolder);
        
        time_t unixTime = time(NULL);
        struct tm* timeStruct = gmtime((const time_t *)&unixTime);
        std::string session = Utils::Format("%04d%02d%02d_%02d%02d", timeStruct->tm_year, timeStruct->tm_mon + 1, timeStruct->tm_mday, timeStruct->tm_hour, timeStruct->tm_min);
    }
}