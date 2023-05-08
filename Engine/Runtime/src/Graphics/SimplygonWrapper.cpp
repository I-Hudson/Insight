#include "Graphics/SimplygonWrapper.h"

#include "Core/Logger.h"
#include "FileSystem/FileSystem.h"

#include <SimplygonLoader.h>

namespace Insight
{
    ::Simplygon::ISimplygon* SimplygonWrapper::s_instance = nullptr;

    bool SimplygonWrapper::Initialise()
    {
        if (s_instance)
        {
            return true;
        }

        ::Simplygon::EErrorCodes initValue = ::Simplygon::Initialize(&s_instance);
        if (initValue != Simplygon::EErrorCodes::NoError)
        {
            s_instance = nullptr;
            IS_CORE_ERROR("[SimplygonWrapper::Initialise] Simplygon error '{}'.", Simplygon::GetError(initValue));
            return false;
        }
        return true;
    }

    void SimplygonWrapper::Shutdown()
    {
        if (s_instance)
        {
            Simplygon::Deinitialize(s_instance);
            s_instance = nullptr;
        }
    }

    std::string SimplygonWrapper::Run(std::string_view path, SimplygonStages stagesToRun)
    {
        if (!s_instance)
        {
            IS_CORE_ERROR("[SimplygonWrapper::Run] Instance is no initialised. 'SimplygonWrapper::Initialise' must be called before use.");
            return "";
        }

        if (CheckForOptimisedFile(path))
        {
            std::string optimisedFilePath = GetOptimisedFilePath(path);
            IS_CORE_INFO("[SimplygonWrapper::Run] Optimised version of '{}' already exists '{}.", path, optimisedFilePath);
            return optimisedFilePath;
        }

        Simplygon::spScene scene = LoadScene(path);
        if (scene.IsNull())
        {
            return "";
        }

        if (stagesToRun & SimplygonStages::Reduction)
        {
            RunReduction(scene);
        }

        std::string optimisedScene = SaveScene(scene, std::string(path));

        // Check log for any warnings or errors. 	
        printf("%s\n", "Check log for any warnings or errors.");
        CheckLog(s_instance);

        return optimisedScene;
    }

    void SimplygonWrapper::RunReduction(Simplygon::spScene& scene)
    {
        // Create the reduction processor. 
        Simplygon::spReductionProcessor sgReductionProcessor = s_instance->CreateReductionProcessor();
        sgReductionProcessor->SetScene(scene);
        Simplygon::spReductionSettings sgReductionSettings = sgReductionProcessor->GetReductionSettings();

        // Set reduction target to triangle ratio with a ratio of 50%. 
        sgReductionSettings->SetReductionTargets(Simplygon::EStopCondition::All, true, false, false, false);
        sgReductionSettings->SetReductionTargetTriangleRatio(0.5f);

        // Start the reduction process. 	
        printf("%s\n", "Start the reduction process.");
        sgReductionProcessor->RunProcessing();

        // Check log for any warnings or errors. 	
        printf("%s\n", "Check log for any warnings or errors.");
        CheckLog(s_instance);
    }

    Simplygon::spScene SimplygonWrapper::LoadScene(std::string_view path)
    {
        // Create scene importer 
        Simplygon::spSceneImporter sgSceneImporter = s_instance->CreateSceneImporter();
        sgSceneImporter->SetImportFilePath(path.data());

        // Run scene importer. 
        auto importResult = sgSceneImporter->Run();
        if (Simplygon::Failed(importResult))
        {
            IS_CORE_ERROR("[SimplygonWrapper::LoadScene] Failed to load scene '{}'.", path);
            return {};
        }
        Simplygon::spScene sgScene = sgSceneImporter->GetScene();
        return sgScene;
    }

    std::string SimplygonWrapper::SaveScene(Simplygon::spScene& scene, std::string path)
    {
        std::string optimisedPath = GetOptimisedFilePath(path);
        // Create scene exporter. 
        Simplygon::spSceneExporter sgSceneExporter = s_instance->CreateSceneExporter();
        sgSceneExporter->SetExportFilePath(optimisedPath.c_str());
        sgSceneExporter->SetScene(scene);

        // Run scene exporter. 
        auto exportResult = sgSceneExporter->Run();
        if (Simplygon::Failed(exportResult))
        {
            IS_CORE_ERROR("[SimplygonWrapper::LoadScene] Failed to save scene to '{}'.", path);
            return "";
        }
        return optimisedPath;
    }

    std::string SimplygonWrapper::GetOptimisedFilePath(std::string_view path)
    {
        std::string extension = std::string(FileSystem::FileSystem::GetFileExtension(path));
        std::string optimisedPath = std::string(path.substr(0, path.find_last_of('.')));
        optimisedPath += "_optimised";
        optimisedPath += extension;
        return optimisedPath;
    }

    bool SimplygonWrapper::CheckForOptimisedFile(std::string_view path)
    {
        return FileSystem::FileSystem::Exists(GetOptimisedFilePath(path));
    }

    void SimplygonWrapper::CheckLog(Simplygon::ISimplygon* sg)
    {
        // Check if any errors occurred. 
        bool hasErrors = sg->ErrorOccurred();
        if (hasErrors)
        {
            Simplygon::spStringArray errors = sg->CreateStringArray();
            sg->GetErrorMessages(errors);
            auto errorCount = errors->GetItemCount();
            if (errorCount > 0)
            {
                printf("%s\n", "Errors:");
                for (auto errorIndex = 0U; errorIndex < errorCount; ++errorIndex)
                {
                    Simplygon::spString errorString = errors->GetItem((int)errorIndex);
                    printf("%s\n", errorString.c_str());
                }
                sg->ClearErrorMessages();
            }
        }
        else
        {
            printf("%s\n", "No errors.");
        }

        // Check if any warnings occurred. 
        bool hasWarnings = sg->WarningOccurred();
        if (hasWarnings)
        {
            Simplygon::spStringArray warnings = sg->CreateStringArray();
            sg->GetWarningMessages(warnings);
            auto warningCount = warnings->GetItemCount();
            if (warningCount > 0)
            {
                printf("%s\n", "Warnings:");
                for (auto warningIndex = 0U; warningIndex < warningCount; ++warningIndex)
                {
                    Simplygon::spString warningString = warnings->GetItem((int)warningIndex);
                    printf("%s\n", warningString.c_str());
                }
                sg->ClearWarningMessages();
            }
        }
        else
        {
            printf("%s\n", "No warnings.");
        }

        // Error out if Simplygon has errors. 
        if (hasErrors)
        {
            throw std::exception("Processing failed with an error");
        }
    }


}