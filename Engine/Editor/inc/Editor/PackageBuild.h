#pragma once

#include <string_view>

namespace Insight
{
    namespace Editor
    {
        struct PackageBuildOptions
        {
            bool EnableProfiling = false;
        };

        /// @brief Take the engine and client code and compile a new .exe
        // with the correct resources included.
        class PackageBuild
        {
        public:

            void Build(std::string_view outputFolder, const PackageBuildOptions& options);

        private:
            std::string GetExecuteablepath();
            void BuildSolution();
            void BuildPackageBuild(std::string_view outputFolder);

            std::string GenerateBuildFile();

            void CopyEngineResourceFiles(std::string_view outputFolder) const;
            void BuildRuntimeSettings(std::string_view outputFolder) const;
            void BuildContentFiles(std::string_view outputFolder) const;
        };
    }
}