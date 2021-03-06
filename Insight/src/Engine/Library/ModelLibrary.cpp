
//
//#include "Engine/Library/ModelLibrary.h"
//#include "Engine/Memory/MemoryManager.h"
//#include "Engine/Threading/ThreadPool.hpp"
//#include "Engine/Threading/ThreadCollection.h"
//#include "Engine/Threading/TThreadSafe.h"
//
//#include <ppl.h>
//#include <concurrent_vector.h>
//#include <filesystem>
//
//	namespace Library
//	{
//		ModelLibrary::ModelLibrary()
//			: Library()
//			, Serializable(this, false, "ModelLibrary.json")
//		{
//		}
//
//		ModelLibrary::~ModelLibrary()
//		{
//		}
//
//		Model* ModelLibrary::GetAssetFromPath(const std::string& filePath)
//		{
//			for (auto it = m_assets.begin(); it != m_assets.end(); ++it)
//			{
//				if ((*it).second->GetFilePath() == filePath)
//				{
//					return it->second;
//				}
//			}
//
//			std::string filepathLinux = filePath;
//			std::replace(filepathLinux.begin(), filepathLinux.end(), '\\', '/');
//
//			FILE* f;
//			errno_t error = fopen_s(&f, filepathLinux.c_str(), "r");
//			if (error == 0)
//			{
//				Model* m = ::New<Model>(filePath);
//				AddAsset(m->GetUUID(), m);
//				return m;
//			}
//			
//			//IS_CORE_ERROR("[ModelLibrary::GetAssetFromPath] Was unable to load '{0}'. Make sure file exists.", filePath);
//			return nullptr;
//		}
//
//		void ModelLibrary::LoadAssetsFromFolder(const std::string& folderName, const bool& lookInChildren)
//		{
//			IS_PROFILE_FUNCTION();
//
//			using fs = std::filesystem::recursive_directory_iterator;
//			std::vector<std::string> filePaths;
//			std::vector<u32> filePathsSize;
//
//			for (const auto& entry : std::filesystem::recursive_directory_iterator::recursive_directory_iterator(folderName))
//			{
//				std::string extension = entry.path().u8string();
//				extension = extension.substr(extension.find_last_of("."));
//
//				auto exIt = std::find(m_extensions.begin(), m_extensions.end(), extension);
//				if (exIt != m_extensions.end())
//				{
//					std::string formattedFilePath = entry.path().u8string();
//					std::replace(formattedFilePath.begin(), formattedFilePath.end(), '\\', '/');
//					filePaths.push_back(formattedFilePath);
//					filePathsSize.push_back(static_cast<u32>(entry.file_size()));
//				}
//			}
//
//			u32 objsPerThread = static_cast<u32>(filePaths.size()) >= 4 ? static_cast<u32>(filePaths.size()) / 4 : 0;
//			if (objsPerThread >= 1)
//			{
//				using VecSharedModels = std::vector<Model*>;
//
//				auto addMeshToContainer = [](ModelLoadThread threadData)
//				{
//					IS_PROFILE_THREAD("Worker");
//
//					VecSharedModels loadedModels;
//					for (const auto& file : threadData.FilesToLoad)
//					{
//						Model* m = ::New<Model>(file);
//						loadedModels.push_back(m);
//					}
//					return loadedModels;
//				};
//				auto assignDataToThread = [](std::array<ModelLoadThread, 4>& threadsData, std::string fileName, u32 fileSize)
//				{
//					IS_PROFILE_FUNCTION();
//
//					u32 lowsetSize = std::numeric_limits<u32>::max();
//					u32 index = 0;
//					for (u32 i = 0; i < 4; ++i)
//					{
//						if (threadsData[i].FileOverallSize < lowsetSize)
//						{
//							index = i;
//							lowsetSize = threadsData[i].FileOverallSize;
//						}
//					}
//
//					threadsData[index].FilesToLoad.push_back(fileName);
//					threadsData[index].FileOverallSize += fileSize;
//				};
//
//				Threading::ThreadCollection<4, VecSharedModels, ModelLoadThread, std::string, u32> threadCollection(addMeshToContainer, assignDataToThread);
//
//				int index = 0;
//				for (auto& file : filePaths)
//				{
//					threadCollection.AddData(file, filePathsSize[index++]);
//				}
//				auto results = threadCollection.Execute();
//				threadCollection.Wait();
//
//				for (auto& vec : results)
//				{
//					for (auto& model : vec.get())
//					{
//						AddAsset(model->GetUUID(), model);
//					}
//				}
//			}
//			else
//			{
//				for (auto& file : filePaths)
//				{
//					Model* m = ::New<Model>(file);
//					AddAsset(m->GetUUID(), m);
//				}
//			}
//		}
//
//		void ModelLibrary::Serialize(Serialization::SerializableElement* element, bool force)
//		{
//			//for (auto it = m_assets.begin(); it != m_assets.end(); ++it)
//			//{
//			//	tinyxml2::XMLNode* model = doc->NewElement("Model");
//			//	
//			//	tinyxml2::XMLElement* uuid = doc->NewElement("UUID");
//			//	uuid->SetText(it->second->GetUUID().c_str());
//			//	model->InsertEndChild(uuid);
//			//
//			//	tinyxml2::XMLElement* filePath = doc->NewElement("FilePath");
//			//	filePath->SetText(it->second->GetFilePath().c_str());
//			//	model->InsertEndChild(filePath);
//			//
//			//	out->InsertEndChild(model);
//			//}
//		}
//
//		void ModelLibrary::Deserialize(Serialization::SerializableElement* element, bool force)
//		{
//			IS_PROFILE_FUNCTION();
//
//			// TODO remove/clean. The thread code is for testing.
//
//			//auto loadModelFunc = [&](const std::string& filePath, const std::string& uuid)
//			//{
//			//	Model* m = NEW_ON_HEAP(Model, filePath, uuid);
//			//
//			//	AddAsset(m->GetUUID(), m);
//			//	IS_CORE_INFO("THREAD SPAWNED!!!");
//			//};
//			//
//			//std::vector<std::pair<std::string, std::string>> modelsInFile;
//
//			//tinyxml2::XMLNode* model = nullptr;//data->FirstChild();
//			//do
//			//{
//			//	std::string uuid = model->FirstChildElement("UUID")->GetText();
//			//	std::string filePath = model->FirstChildElement("FilePath")->GetText();
//			//	//modelsInFile.push_back(std::pair<std::string, std::string>(filePath, uuid));
//			//	SharedPtr<Model> m = CreateSharedPtr<Model>(filePath, uuid);
//			//	AddAsset(m->GetUUID(), m);
//
//			//	model = model->NextSibling();
//			//} while (model != nullptr);
//
//			//concurrency::concurrent_vector<Model*> loadedMeshes;
//			//
//			//concurrency::parallel_for(0, (int)modelsInFile.size(), [&](int i)
//			//	{
//			//		Model* m = NEW_ON_HEAP(Model, modelsInFile[i].first, modelsInFile[i].second);
//			//		loadedMeshes.push_back(m);
//			//		IS_CORE_INFO("NEW MDOEL LOADED: '{0}'", m->GetFilePath());
//			//	});
//			//
//			//for (size_t i = 0; i < loadedMeshes.size(); ++i)
//			//{
//			//	AddAsset(loadedMeshes[i]->GetUUID(), loadedMeshes[i]);
//			//}
//		}
//	}