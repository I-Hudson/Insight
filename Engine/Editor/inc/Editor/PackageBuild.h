#pragma once

#include <string_view>

namespace Insight
{
    namespace Editor
    {
        /// @brief Take the engine and client code and compile a new .exe
        // with the correct resources included.
        class PackageBuild
        {
        public:

            void Build(std::string_view outputFolder);

        private:
            std::string GetExecuteablepath();
            void BuildSolution();
            void BuildPackageBuild(std::string_view outputFolder);

            std::string GenerateBuildFile();
        };
    }
}