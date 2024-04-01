#include "Asset/Importers/IAssetImporter.h"

#include "Algorithm/Vector.h"

namespace Insight
{
    namespace Runtime
    {
        IAssetImporter::IAssetImporter(std::vector<const char*> validFileExtensions)
            : m_validFileExtensions(std::move(validFileExtensions))
        { }

        IAssetImporter::~IAssetImporter()
        { }

        bool IAssetImporter::IsValidImporterForFileExtension(const char* fileExtension) const
        {
            for (size_t i = 0; i < m_validFileExtensions.size(); ++i)
            {
                if (strcmp(m_validFileExtensions[i], fileExtension) == 0) 
                {
                    return true;
                }
            }
            return false;
        }
    }
}