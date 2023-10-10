#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
    class RMCLogger
    {
    private:
        struct PcapHeader {
            u32 magic;
            u16 versionMajor;
            u16 versionMinor;
            u32 thisZone;
            u32 sigfigs;
            u32 snaplen;
            u32 network;
            PcapHeader() {
                magic = 0xa1b2c3d4;
                versionMajor = 2;
                versionMinor = 4;
                thisZone = 0;
                sigfigs = 0;
                snaplen = maxPacketSize;
                network = 147; // Value for user reserved 0
            }
        };
        struct PcapPacketHeader
        {
            u32 timestamp;
            u32 microsecondoffset;
            u32 savedBytes;
            u32 packetBytes;
        };
        struct PacketMetadata
        {
            struct {
                u8 isRecievedPacked : 1;
                u8 userPacketNote : 1;
            } flags;
        };
        u64 titleID;
        File* pcapFile;
        Clock currentElapsed;
        time_t startTime;

        u8* writeBuffer;

        void Initialize();
        void Terminate();
    public:
        static constexpr u32 maxPacketSize = 65535;

        RMCLogger() {Initialize();};
        ~RMCLogger() {Terminate();};

        
        void LogRMCPacket(const u8* data, u32 packetSize, bool isRecieved);
    };
}