/***
* Bare Metal Engine
* Written by Tomasz Jonarski (RexDex)
* Basic middleware testing project
***/

#include "build.h"

#ifdef __APPLE__
#import <sys/proc_info.h>
#import <libproc.h>
#elif defined _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <ofbx.h>

//--

#include <vector>
#include <filesystem>
#include <fstream>

//--

#define TEST_DATA "../../../data/"

std::filesystem::path GetExecutablePath()
{
	char exepath[1024];
	memset(exepath, 0, sizeof(exepath));

#ifdef _WIN32
	GetModuleFileNameA(GetModuleHandle(NULL), exepath, sizeof(exepath));
#elif defined(__APPLE__)
	proc_pidpath(getpid(), exepath, sizeof(exepath));
	printf("path: %s\n", exepath);
#else
	char arg1[20];
	sprintf(arg1, "/proc/%d/exe", getpid());
	if (readlink(arg1, exepath, sizeof(exepath)) < 0)
		return "";
#endif

	return std::filesystem::path(exepath);
}

std::filesystem::path TestRootPath()
{
	static auto rootPath = std::filesystem::weakly_canonical(GetExecutablePath().parent_path() / TEST_DATA).make_preferred();
	return rootPath;
}

std::filesystem::path MakeTestDataPath(std::string_view name)
{
	return (TestRootPath() / name).make_preferred();
}

bool LoadFileToBuffer(const std::filesystem::path& path, std::vector<uint8_t>& outBuffer)
{
    std::ifstream file(path, std::ios::binary);
    file.unsetf(std::ios::skipws);

    if (!file.is_open())
    {
        std::cout << "Unable to open file " << path << "\n";
        return false;
    }

    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    outBuffer.resize(fileSize);
    file.read((char*)outBuffer.data(), fileSize);

    return true;
}

struct SceneWrapper
{
	ofbx::IScene* scene = nullptr;

	SceneWrapper(ofbx::IScene* s)
		: scene(s)
	{}

	~SceneWrapper()
	{
		if (scene)
		{
			scene->destroy();
			scene = nullptr;
		}
	}
};

std::shared_ptr<SceneWrapper> LoadScene(std::string_view name)
{
	const auto path = MakeTestDataPath(name);

	std::vector<uint8_t> data;
	if (!LoadFileToBuffer(path, data))
		return nullptr;

	auto scene = ofbx::load(data.data(), data.size(), 0);
	if (!scene)
		return nullptr;

	return std::make_shared<SceneWrapper>(scene);
}

//--

TEST(OpenFBX, LoadCube)
{
	auto scene = LoadScene("cube.fbx");
	ASSERT_TRUE(scene);
}

TEST(OpenFBX, CubeHasSingleGeometry)
{
	auto scene = LoadScene("cube.fbx");
	ASSERT_TRUE(scene);

	EXPECT_EQ(1, scene->scene->getGeometryCount());
	EXPECT_EQ(1, scene->scene->getMeshCount());
}

TEST(OpenFBX, CubeHasValidObjects)
{
	auto scene = LoadScene("cube.fbx");
	ASSERT_TRUE(scene);

	const auto objectCount = scene->scene->getAllObjectCount();
	const auto *objects = scene->scene->getAllObjects();
	ASSERT_EQ(3, objectCount);

	{
		auto* obj = objects[0];		
		EXPECT_EQ(ofbx::Object::Type::MATERIAL, obj->getType());
		EXPECT_STREQ("Material", obj->name);
	}

	{
		auto* obj = objects[1];
		EXPECT_EQ(ofbx::Object::Type::MESH, obj->getType());
		EXPECT_STREQ("Cube", obj->name);
	}

	{
		auto* obj = objects[2];
		EXPECT_EQ(ofbx::Object::Type::GEOMETRY, obj->getType());
		EXPECT_STREQ("CubeGeometry", obj->name);
	}
}

TEST(OpenFBX, RootElement)
{
	auto scene = LoadScene("cube.fbx");
	ASSERT_TRUE(scene);

	auto root = scene->scene->getRoot();
	ASSERT_TRUE(root);	
	ASSERT_EQ(ofbx::Object::Type::ROOT, root->getType());
	ASSERT_STREQ("RootNode", root->name);
}

TEST(OpenFBX, CubeHasGeometry)
{
	auto scene = LoadScene("cube.fbx");
	ASSERT_TRUE(scene);

	auto geometry = scene->scene->getGeometry(0);
	ASSERT_TRUE(geometry);
	ASSERT_STREQ("CubeGeometry", geometry->name);

	EXPECT_EQ(24, geometry->getIndexCount());
	EXPECT_EQ(8, geometry->getVertexCount());
}

//--
