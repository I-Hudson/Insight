#include "Asset/AssetPackage.h"

int main(int argc, char** agc)
{
    using namespace Insight;
    Runtime::AssetPackage package("./", "TestPackage");

    package.BuildPackage("./TestPackage.ispackage");

    return 0;
}