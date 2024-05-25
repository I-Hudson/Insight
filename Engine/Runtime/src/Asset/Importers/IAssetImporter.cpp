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
                std::string inFileExtension = fileExtension;
                ToLower(inFileExtension);
                std::string importerFileExtension = m_validFileExtensions[i];
                ToLower(importerFileExtension);

                if (inFileExtension == importerFileExtension)
                {
                    return true;
                }

                if (importerFileExtension.front() == '.')
                {
                    importerFileExtension = importerFileExtension.substr(1);
                }
                
                if (inFileExtension == importerFileExtension)
                {
                    return true;
                }
            }
            return false;
        }
    }
}