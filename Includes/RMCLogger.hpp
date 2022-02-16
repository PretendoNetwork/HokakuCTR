#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
    class RMCLogger
    {
    private:
        File* rawFile;
        File* infoFile;
    public:
        RMCLogger();
        ~RMCLogger();

        void Initialize();
    };
}