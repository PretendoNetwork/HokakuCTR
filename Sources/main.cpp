#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include "PatternManager.hpp"
#include "OSDManager.hpp"
#include "rt.h"

#include <vector>
#include <string.h>


namespace CTRPluginFramework
{
    LightEvent exitEvent;

    static inline u32   decodeARMBranch(const u32 *src)
	{
		s32 off = (*src & 0xFFFFFF) << 2;
		off = (off << 6) >> 6;
		return (u32)src + 8 + off;
	}

    // This function executes before the game runs.
    void    PatchProcess(FwkSettings &settings)
    {
        PatternManager pm;

        pm.Perform();
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

        // Wait for process exit event.
        LightEvent_Wait(&exitEvent);

        // Exit plugin
        return (0);
    }
}
