#include "RMCLogger.hpp"
#include "time.h"

namespace CTRPluginFramework {
    void RMCLogger::Initialize() {
                std::map<std::string, std::string> gameNames = {
            {"CENTER", "badge-arcade"},
            {"ctrapp", "super-smash-bros3ds"},
            {"kujira-1", "PokemonX"},
            {"MarioKar", "mario-kart7"},
            {"TAKUMI", "animal-crossing-happyHD"},
            {"Kirby3DS", "Team-kirby-clash-deluxe"},
            {"GARDEN", "animal-crossing-new-leaf"},
            {"momiji", "pokemon-ultra-soleil"},
            {"Rabbids", "rabbids-rumble"},
            {"uniform", "mario-superstars"}
        };

        std::string finalFolder = "/HokakuCTR";
        if (!Directory::IsExists(finalFolder))
            Directory::Create(finalFolder);

        std::string tid;
        Process::GetTitleID(tid);
        std::string procName;
        Process::GetName(procName);

        titleID = Process::GetTitleID();

        // Rechercher si le nom de processus commence par l'un des termes de la liste
        for (const auto& gameName : gameNames) {
            if (procName.find(gameName.first) == 0) {
                procName.replace(0, gameName.first.length(), gameName.second);
                break;
            }
        }

        finalFolder += "/" + procName + " - (" + tid + ")";
        if (!Directory::IsExists(finalFolder))
            Directory::Create(finalFolder);

        
        startTime = time(NULL);
        currentElapsed.Restart();
        
        struct tm* timeStruct = gmtime((const time_t *)&startTime);
        std::string session = Utils::Format("%02d%02d%02d_%02d%02d%02d.pcap", timeStruct->tm_year % 100, timeStruct->tm_mon + 1, timeStruct->tm_mday, timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec);
        
        pcapFile = new File(finalFolder + "/" + session, File::RWC);

        PcapHeader hdr;
        pcapFile->Write(&hdr, sizeof(PcapHeader));

        writeBuffer = (u8*)operator new(maxPacketSize + sizeof(PcapPacketHeader));
    }

    void RMCLogger::Terminate() {
        pcapFile->Flush();
        pcapFile->Close();
        delete pcapFile;
        pcapFile = nullptr;
        operator delete(writeBuffer);
        writeBuffer = nullptr;
    }

    void RMCLogger::LogRMCPacket(const u8* data, u32 packetSize, bool isRecieved) {
        if (packetSize < 4 || ((u32*)data)[0] != packetSize - 4)
            return;
        if (packetSize > maxPacketSize - sizeof(PacketMetadata)) {
            OSD::Notify(Utils::Format("Packet too big! 0x%08X, 0x%08X", (u32)data, packetSize));
        }
        PcapPacketHeader* pHdr = reinterpret_cast<PcapPacketHeader*>(writeBuffer);
        pHdr->savedBytes = pHdr->packetBytes = packetSize + sizeof(PacketMetadata);

        s64 elapsedMsecTot = currentElapsed.GetElapsedTime().AsMicroseconds();
        u32 elapsedSec = elapsedMsecTot / 1000000;
        u32 elapsedMsec = elapsedMsecTot - elapsedSec * 1000000;

        pHdr->timestamp = startTime + elapsedSec;
        pHdr->microsecondoffset = elapsedMsec;

        PacketMetadata metadata;
        metadata.titleID = titleID;
        metadata.flags.isRecievedPacket = isRecieved;
        memcpy(writeBuffer + sizeof(PcapPacketHeader), &metadata, sizeof(PacketMetadata));
        
        memcpy(writeBuffer + sizeof(PcapPacketHeader) + sizeof(PacketMetadata), data, packetSize);
        pcapFile->Write(writeBuffer, sizeof(PcapPacketHeader) + sizeof(PacketMetadata) + packetSize);
    }
}
