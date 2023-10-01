#include "Asset/AssetRegistry.h"
#include "Asset/AssetPackage.h"
#include "FileSystem/FileSystem.h"

#include <filesystem>

int main(int argc, char** argv)
{
    const char* packagePath = argv[1];
    const char* packageName = argv[2];
    const char* packageOutputPath = argv[argc - 1];

    using namespace Insight;
    Runtime::AssetRegistry assetRegistry;
    Runtime::AssetPackage package(packagePath, packageName);

    for (size_t i = 3; i < argc - 1; ++i)
    {
        const char* path = argv[i];
        if (FileSystem::IsFile(std::string_view(path)))
        {
            assetRegistry.AddAsset(argv[i], &package);
        }
        else
        {
            for (auto pathItr : std::filesystem::recursive_directory_iterator(path))
            {
                if (FileSystem::IsFile(std::string_view(pathItr.path().string()))
                    && (FileSystem::GetFileExtension(pathItr.path().string()) != ".isassetpackage" 
                        || FileSystem::GetFileExtension(pathItr.path().string()) != ".ispackage"))
                {
                    assetRegistry.AddAsset(pathItr.path().string(), &package);
                }
            }
        }
    }

    package.BuildPackage(packageOutputPath);

    return 0;
}