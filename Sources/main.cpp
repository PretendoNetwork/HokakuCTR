#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include "PatternManager.hpp"
#include "OSDManager.hpp"
#include "RMCLogger.hpp"
#include "rt.h"

#include <vector>
#include <string.h>

namespace CTRPluginFramework
{
    enum NexBufferVersion {
        NOTINIT = -1,
        INVALID = -2,
        V0 = 0,
        V1 = 1
    };

    RT_HOOK sendPacketHook = { 0 };
    RT_HOOK recvPacketHook = { 0 };
    u32 sendFuncAddr = 0;
    u32 recvFuncAddr = 0;
    NexBufferVersion bufferVersion = NexBufferVersion::NOTINIT;
    RMCLogger* mainLogger;

    static inline u32* findNearestSTMFDptr(u32* newaddr) {
		u32 i;
		for (i = 0; i < 1024; i++) {
			newaddr--;
			i++;
			if (*((u16*)newaddr + 1) == 0xE92D) {
				return newaddr;
			}
		}
		return 0;
	}

    struct NexBufferv0 {
        u32 vtable;
        u32 unk;
        const u8* data;
        u32 size;
    };

    struct NexBufferv1 {
        u32 vtable;
        u32 refCount;
        u32 unk;
        const u8* data;
        u32 size;
    };

    void AnaliseNexBufferVersion(void* buffer) {
        u32* bufPtr = (u32*)buffer;
        u8 tempRead;
        if (Process::Read8(bufPtr[2], tempRead) && Process::Read8(bufPtr[2] + bufPtr[3] -1, tempRead) && bufPtr[3] < RMCLogger::maxPacketSize) {
            bufferVersion = NexBufferVersion::V0;
            OSD::Notify("Detected NEX buffer type: V0");
        } else if (Process::Read8(bufPtr[3], tempRead) && Process::Read8(bufPtr[3] + bufPtr[4] -1, tempRead) && bufPtr[4] < RMCLogger::maxPacketSize) {
            bufferVersion = NexBufferVersion::V1;
            OSD::Notify("Detected NEX buffer type: V1");
        } else {
            bufferVersion = NexBufferVersion::INVALID;
            OSD::Notify(Utils::Format("Unk buff type: %08X %08X %08X %08X %08X", bufPtr[0], bufPtr[1], bufPtr[2], bufPtr[3], bufPtr[4]));
        }            
    }

    u32 nexSendPacket(u32 arg0, u32 arg1, void* buffer, u32 arg2) {
        if (bufferVersion == NexBufferVersion::NOTINIT)
            AnaliseNexBufferVersion(buffer);
        
        if (bufferVersion == NexBufferVersion::V0)
        {
            NexBufferv0* buf = (NexBufferv0*)buffer;
            mainLogger->LogRMCPacket(buf->data, buf->size, false);
        } else if (bufferVersion == NexBufferVersion::V1)
        {
            NexBufferv1* buf = (NexBufferv1*)buffer;
            mainLogger->LogRMCPacket(buf->data, buf->size, false);
        }
            
        return ((u32(*)(u32, u32, void*, u32))sendPacketHook.callCode)(arg0, arg1, buffer, arg2);
    }

    u32 nexRecvPacket(u32 arg0, void* buffer, u32 arg1) {
        if (bufferVersion == NexBufferVersion::NOTINIT)
            AnaliseNexBufferVersion(buffer);
        
        if (bufferVersion == NexBufferVersion::V0)
        {
            NexBufferv0* buf = (NexBufferv0*)buffer;
            mainLogger->LogRMCPacket(buf->data, buf->size, true);
        } else if (bufferVersion == NexBufferVersion::V1)
        {
            NexBufferv1* buf = (NexBufferv1*)buffer;
            mainLogger->LogRMCPacket(buf->data, buf->size, true);
        }

        return ((u32(*)(u32, void*, u32))recvPacketHook.callCode)(arg0, buffer, arg1);
    }
    
    Mutex sendPatternMutex;
    Mutex recvPatternMutex;

    bool installSendRMC(u32 addr) {
        Lock lock(sendPatternMutex);
        if (sendFuncAddr) return true;
        u32 funcStart = (u32)findNearestSTMFDptr((u32*)addr);
        if (!funcStart) return false;
        rtInitHook(&sendPacketHook, funcStart, (u32)nexSendPacket);
        sendFuncAddr = funcStart;
        return true;
    }

    bool installRecvRMC(u32 addr) {
        Lock lock(recvPatternMutex);
        if (recvFuncAddr) return true;
        u32 funcStart = (u32)findNearestSTMFDptr((u32*)addr);
        if (!funcStart) return false;
        rtInitHook(&recvPacketHook, funcStart, (u32)nexRecvPacket);
        recvFuncAddr = funcStart;
        return true;
    }

    // This function executes before the game runs.
    void    PatchProcess(FwkSettings &settings)
    {
        PatternManager pm;

        const u8 sendRMCPat[] = {0x60, 0x40, 0xA0, 0x13, 0x02, 0x00, 0x15, 0xE3, 0x01, 0x4C, 0x84, 0x13, 0x10, 0x00, 0x15, 0xE3};
        pm.Add(sendRMCPat, sizeof(sendRMCPat), installSendRMC);
        const u8 sendRMCPat1[] = {0x60, 0x40, 0xA0, 0x13, 0x02, 0x00, 0x17, 0xE3, 0x01, 0x4C, 0x84, 0x13, 0x10, 0x00, 0x17, 0xE3};
        pm.Add(sendRMCPat1, sizeof(sendRMCPat1), installSendRMC);
        const u8 sendRMCPat2[] = {0x60, 0x40, 0xA0, 0x13, 0x02, 0x00, 0x18, 0xE3, 0x01, 0x4C, 0x84, 0x13, 0x10, 0x00, 0x18, 0xE3};
        pm.Add(sendRMCPat2, sizeof(sendRMCPat2), installSendRMC);
        const u8 sendRMCPat3[] = {0x60, 0x40, 0xA0, 0x13, 0x02, 0x00, 0x16, 0xE3, 0x01, 0x4C, 0x84, 0x13, 0x10, 0x00, 0x16, 0xE3};
        pm.Add(sendRMCPat3, sizeof(sendRMCPat3), installSendRMC);

        const u8 recvRMCPat[] = {0x00, 0x00, 0x50, 0xE3, 0x08, 0x10, 0x90, 0x15, 0x58, 0x00, 0x81, 0xE2, 0xFF, 0x10, 0xCC, 0xE3};
        pm.Add(recvRMCPat, sizeof(recvRMCPat), installRecvRMC);
        const u8 recvRMCPat1[] = {0x0C, 0x50, 0x90, 0x15, 0x58, 0x00, 0x85, 0xE2, 0xFF, 0x10, 0xC6, 0xE3, 0x00, 0x50, 0xA0, 0xE1};
        pm.Add(recvRMCPat1, sizeof(recvRMCPat1), installRecvRMC);
        const u8 recvRMCPat2[] = {0xBC, 0x03, 0xD2, 0xE1, 0x01, 0x70, 0xA0, 0xE1, 0x00, 0x50, 0xA0, 0xE3, 0x02, 0x0B, 0x10, 0xE3};
        pm.Add(recvRMCPat2, sizeof(recvRMCPat2), installRecvRMC);
        const u8 recvRMCPat3[] = {0x02, 0x0B, 0x12, 0xE3, 0x01, 0x70, 0xA0, 0xE3, 0xD0, 0x20, 0xD1, 0xE1, 0x14, 0x00, 0x90, 0xE5};
        pm.Add(recvRMCPat3, sizeof(recvRMCPat3), installRecvRMC);
        const u8 recvRMCPat4[] = {0x08, 0x40, 0x90, 0x15, 0x58, 0x00, 0x84, 0xE2, 0xFF, 0x10, 0xC5, 0xE3, 0x00, 0x40, 0xA0, 0xE1};
        pm.Add(recvRMCPat4, sizeof(recvRMCPat4), installRecvRMC);

        pm.Perform();

        if (sendFuncAddr && recvFuncAddr) {
            rtEnableHook(&sendPacketHook);
            rtEnableHook(&recvPacketHook);
            mainLogger = new RMCLogger();
        }
    }

    // This function is called after the game starts executing and CTRPF is ready.
    int     main(void)
    {
        OSD::Notify("HokakuCTR v1.0.3");

        if (sendFuncAddr) {
            OSD::Notify(Utils::Format("Send RMC Addr: 0x%08X", sendFuncAddr));
        } else {
            OSD::Notify("Send RMC Addr: Not Found");
        }

        if (recvFuncAddr) {
            OSD::Notify(Utils::Format("Recv RMC Addr: 0x%08X", recvFuncAddr));
        } else {
            OSD::Notify("Recv RMC Addr: Not Found");
        }

        OSD::Notify((sendFuncAddr && recvFuncAddr) ? "Ready!" : "Not Ready!");

        // Wait for process exit.
        Process::WaitForExit();

        rtDisableHook(&sendPacketHook);
        rtDisableHook(&recvPacketHook);
        delete mainLogger;

        // Exit plugin
        return (0);
    }
}
