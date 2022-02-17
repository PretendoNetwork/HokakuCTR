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
    LightEvent exitEvent;
    RT_HOOK sendPacketHook = { 0 };
    RT_HOOK recvPacketHook = { 0 };
    bool sendFuncV1 = false;
    u32 sendFuncAddr = 0;
    bool recvFuncV1 = false;
    u32 recvFuncAddr = 0;
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

    u32 nexSendPacketV0(u32 arg0, u32 arg1, NexBufferv0* buffer, u32 arg2) {
        mainLogger->LogRMCPacket(buffer->data, buffer->size);
        return ((u32(*)(u32, u32, NexBufferv0*, u32))sendPacketHook.callCode)(arg0, arg1, buffer, arg2);
    }

    u32 nexSendPacketV1(u32 arg0, u32 arg1, NexBufferv1* buffer, u32 arg2) {
        mainLogger->LogRMCPacket(buffer->data, buffer->size);
        return ((u32(*)(u32, u32, NexBufferv1*, u32))sendPacketHook.callCode)(arg0, arg1, buffer, arg2);
    }

    u32 nexRecvPacketV0(u32 arg0, NexBufferv0* buffer, u32 arg1) {
        mainLogger->LogRMCPacket(buffer->data, buffer->size);
        return ((u32(*)(u32, NexBufferv0*, u32))recvPacketHook.callCode)(arg0, buffer, arg1);
    }

    u32 nexRecvPacketV1(u32 arg0, NexBufferv1* buffer, u32 arg1) {
        mainLogger->LogRMCPacket(buffer->data, buffer->size);
        return ((u32(*)(u32, NexBufferv1*, u32))recvPacketHook.callCode)(arg0, buffer, arg1);
    }

    // This function executes before the game runs.
    void    PatchProcess(FwkSettings &settings)
    {
        PatternManager pm;

        const u8 sendRMCV0Pat[] = {0x60, 0x40, 0xA0, 0x13, 0x02, 0x00, 0x15, 0xE3, 0x01, 0x4C, 0x84, 0x13, 0x10, 0x00, 0x15, 0xE3};
        pm.Add(sendRMCV0Pat, sizeof(sendRMCV0Pat), [](u32 addr) {
            u32 funcStart = (u32)findNearestSTMFDptr((u32*)addr);
            if (!funcStart) return false;
            rtInitHook(&sendPacketHook, funcStart, (u32)nexSendPacketV0);
            sendFuncAddr = funcStart;
            sendFuncV1 = false;
            return true;
        });

        const u8 sendRMCV1Pat[] = {0x60, 0x40, 0xA0, 0x13, 0x02, 0x00, 0x17, 0xE3, 0x01, 0x4C, 0x84, 0x13, 0x10, 0x00, 0x17, 0xE3};
        pm.Add(sendRMCV1Pat, sizeof(sendRMCV1Pat), [](u32 addr) {
            u32 funcStart = (u32)findNearestSTMFDptr((u32*)addr);
            if (!funcStart) return false;
            rtInitHook(&sendPacketHook, funcStart, (u32)nexSendPacketV1);
            sendFuncAddr = funcStart;
            sendFuncV1 = true;
            return true;
        });

        const u8 recvRMCV0Pat[] = {0x00, 0x00, 0x50, 0xE3, 0x08, 0x10, 0x90, 0x15, 0x58, 0x00, 0x81, 0xE2, 0xFF, 0x10, 0xCC, 0xE3};
        pm.Add(recvRMCV0Pat, sizeof(recvRMCV0Pat), [](u32 addr) {
            u32 funcStart = (u32)findNearestSTMFDptr((u32*)addr);
            if (!funcStart) return false;
            rtInitHook(&recvPacketHook, funcStart, (u32)nexRecvPacketV0);
            recvFuncAddr = funcStart;
            recvFuncV1 = false;
            return true;
        });

        const u8 recvRMCV1Pat[] = {0x0C, 0x50, 0x90, 0x15, 0x58, 0x00, 0x85, 0xE2, 0xFF, 0x10, 0xC6, 0xE3, 0x00, 0x50, 0xA0, 0xE1};
        pm.Add(recvRMCV1Pat, sizeof(recvRMCV1Pat), [](u32 addr) {
            u32 funcStart = (u32)findNearestSTMFDptr((u32*)addr);
            if (!funcStart) return false;
            rtInitHook(&recvPacketHook, funcStart, (u32)nexRecvPacketV1);
            recvFuncAddr = funcStart;
            recvFuncV1 = true;
            return true;
        });

        pm.Perform();

        if (sendFuncAddr && recvFuncAddr) {
            rtEnableHook(&sendPacketHook);
            rtEnableHook(&recvPacketHook);
            mainLogger = new RMCLogger();
        }
    }


    // This function is called when the process exits
    void    OnProcessExit(void)
    {
        LightEvent_Signal(&exitEvent);
    }

    // This function is called after the game starts executing and CTRPF is ready.
    int     main(void)
    {
        LightEvent_Init(&exitEvent, RESET_ONESHOT);

        OSD::Notify("HokakuCTR v1.0");

        if (sendFuncAddr) {
            OSD::Notify(Utils::Format("Send RMC Addr: 0x%08X (%s)", sendFuncAddr, (sendFuncV1) ? "V1" : "V0"));
        } else {
            OSD::Notify("Send RMC Addr: Not Found");
        }

        if (recvFuncAddr) {
            OSD::Notify(Utils::Format("Recv RMC Addr: 0x%08X (%s)", recvFuncAddr, (recvFuncV1) ? "V1" : "V0"));
        } else {
            OSD::Notify("Recv RMC Addr: Not Found");
        }

        OSD::Notify((sendFuncAddr && recvFuncAddr) ? "Ready!" : "Not Ready!");

        // Wait for process exit event.
        LightEvent_Wait(&exitEvent);

        rtDisableHook(&sendPacketHook);
        rtDisableHook(&recvPacketHook);
        delete mainLogger;

        // Exit plugin
        return (0);
    }
}
