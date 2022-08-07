/***
* Bare Metal Engine
* Written by Tomasz Jonarski (RexDex)
* Basic middleware testing project
***/

#include "build.h"

#include <vector>
#include <filesystem>
#include <fstream>

#ifdef __APPLE__
#import <sys/proc_info.h>
#import <libproc.h>
#elif defined _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

//--

#define FREEIMAGE_LIB
#define OPJ_STATIC
#include <FreeImage.h>

#ifdef PLATFORM_WINAPI
void __cdecl InitJXR(struct Plugin*, int) {}
#else
void InitJXR(struct Plugin*, int) {}
#endif

//--

namespace memory
{
	struct MemoryState
	{
		const uint8_t* m_data = nullptr;
		uint32_t m_size = 0;
		uint32_t m_pos = 0;
	};

	static unsigned DLL_CALLCONV ReadProc(void* buffer, unsigned size, unsigned count, fi_handle handle)
	{
		auto reader = (MemoryState*)handle;

		auto totalRead = size * count;
		auto left = reader->m_size - reader->m_pos;
		if (totalRead > left)
			totalRead = (uint32_t)(left);

		if (totalRead)
		{
			memcpy(buffer, reader->m_data + reader->m_pos, totalRead);
			reader->m_pos += totalRead;
		}

		return totalRead / size;
	}

	static unsigned DLL_CALLCONV WriteProc(void* buffer, unsigned size, unsigned count, fi_handle handle)
	{
		return 0;
	}

	static int DLL_CALLCONV SeekProc(fi_handle handle, long offset, int origin)
	{
		auto reader = (MemoryState*)handle;

		if (origin == SEEK_END)
		{
			if (offset < 0)
				offset = 0;
			else if (offset > reader->m_size)
				offset = reader->m_size;

			reader->m_pos = reader->m_size - offset;
		}
		else if (origin == SEEK_CUR)
		{
			int64_t newPos = (int64_t)reader->m_pos + offset;

			if (newPos < 0)
				newPos = 0;
			else if (newPos > reader->m_size)
				newPos = reader->m_size;

			reader->m_pos = newPos;
		}
		else if (origin == SEEK_SET)
		{
			if (offset < 0)
				offset = 0;
			else if (offset > reader->m_size)
				offset = reader->m_size;

			reader->m_pos = offset;
		}
		return 0;
	}

	static long DLL_CALLCONV TellProc(fi_handle handle)
	{
		auto reader = (MemoryState*)handle;
		return reader->m_pos;
	}

} // memory

//--

static FREE_IMAGE_FORMAT GetFormat(const char* typeHint)
{
	if (0 == strcmp(typeHint, ".bmp"))
		return FIF_BMP;
	else if (0 == strcmp(typeHint, ".dds"))
		return FIF_DDS;
	else if (0 == strcmp(typeHint, ".jpg") || 0 == strcmp(typeHint, ".jpeg"))
		return FIF_JPEG;
	else if (0 == strcmp(typeHint, ".jp2") || 0 == strcmp(typeHint, ".jpx"))
		return FIF_JP2;
	else if (0 == strcmp(typeHint, ".tga"))
		return FIF_TARGA;
	else if (0 == strcmp(typeHint, ".tiff") || 0 == strcmp(typeHint, ".tif"))
		return FIF_TIFF;
	else if (0 == strcmp(typeHint, ".hdr"))
		return FIF_HDR;
	else if (0 == strcmp(typeHint, ".exr"))
		return FIF_EXR;
	else if (0 == strcmp(typeHint, ".ppm"))
		return FIF_PPM;
	else if (0 == strcmp(typeHint, ".pgm"))
		return FIF_PGM;
	else if (0 == strcmp(typeHint, ".pbm"))
		return FIF_PBM;
	else if (0 == strcmp(typeHint, ".png"))
		return FIF_PNG;
	else if (0 == strcmp(typeHint, ".psd"))
		return FIF_PSD;
	else if (0 == strcmp(typeHint, ".xbm"))
		return FIF_XBM;
	else if (0 == strcmp(typeHint, ".xpm"))
		return FIF_XPM;
	else if (0 == strcmp(typeHint, ".ico"))
		return FIF_ICO;
	else if (0 == strcmp(typeHint, ".gif"))
		return FIF_GIF;
	else if (0 == strcmp(typeHint, ".webp"))
		return FIF_WEBP;

	return FIF_UNKNOWN;
}

static FREE_IMAGE_FORMAT GetFormat(memory::MemoryState& buf, const char* typeHint)
{
	// set the IO
	FreeImageIO io;
	io.read_proc = memory::ReadProc;
	io.write_proc = memory::WriteProc;
	io.tell_proc = memory::TellProc;
	io.seek_proc = memory::SeekProc;

	// read
	auto format = FreeImage_GetFileTypeFromHandle(&io, (fi_handle)&buf, (int)buf.m_size);
	if (format != FIF_UNKNOWN)
		return format;

	// try to get format with the type hint
	return GetFormat(typeHint);
}

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
	try
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
	catch (std::exception& e)
	{
		std::cout << "Error reading file " << path << ": " << e.what() << "\n";
		return false;
	}
}

static void FreeImageOutput(FREE_IMAGE_FORMAT fif, const char* msg)
{
	const char* name = FreeImage_GetFormatFromFIF(fif);
	fprintf(stdout, "FreeImage: %s: %s", name, msg);
}

struct LoadedImage
{
	FIBITMAP* bitmap = nullptr;

	LoadedImage(FIBITMAP* b)
		: bitmap(b)
	{}
	
	~LoadedImage()
	{
		if (bitmap)
		{
			FreeImage_Unload((FIBITMAP*)bitmap);
			bitmap = nullptr;
		}
	}
};

class FreeImageTest : public testing::Test
{
public:
	FreeImageTest()
	{
	}

	static void SetUpTestSuite()
	{
		FreeImage_Initialise(true);
		FreeImage_SetOutputMessage(&FreeImageOutput);
	}

	std::shared_ptr<LoadedImage> loadImage(std::string_view name)
	{
		const auto path = MakeTestDataPath(name);

		std::vector<uint8_t> data;
		if (!LoadFileToBuffer(path, data))
			return nullptr;

		memory::MemoryState memoryState;
		memoryState.m_pos = 0;
		memoryState.m_size = data.size();
		memoryState.m_data = (const uint8_t*)data.data();

		auto ext = std::filesystem::path(name).extension().u8string();
		auto format = GetFormat(memoryState, ext.c_str());
		if (format == FIF_UNKNOWN)
			return nullptr;

		FreeImageIO io;
		io.read_proc = memory::ReadProc;
		io.write_proc = memory::WriteProc;
		io.tell_proc = memory::TellProc;
		io.seek_proc = memory::SeekProc;

		memoryState.m_pos = 0;
		auto* dib = FreeImage_LoadFromHandle(format, &io, (fi_handle)&memoryState);
		if (!dib)
			return nullptr;

		return std::make_shared< LoadedImage>(dib);
	}
};

TEST_F(FreeImageTest, LoadSimplePNG)
{
	auto image = loadImage("test.png");
	ASSERT_TRUE(image);

	const auto height = FreeImage_GetHeight(image->bitmap);
	const auto width = FreeImage_GetWidth(image->bitmap);
	const auto bpp = FreeImage_GetBPP(image->bitmap);
	EXPECT_EQ(16, height);
	EXPECT_EQ(16, width);
	EXPECT_EQ(32, bpp);
}

TEST_F(FreeImageTest, LoadSimpleTGA)
{
	auto image = loadImage("test.tga");
	ASSERT_TRUE(image);

	const auto height = FreeImage_GetHeight(image->bitmap);
	const auto width = FreeImage_GetWidth(image->bitmap);
	const auto bpp = FreeImage_GetBPP(image->bitmap);
	EXPECT_EQ(16, height);
	EXPECT_EQ(16, width);
	EXPECT_EQ(24, bpp);
}

TEST_F(FreeImageTest, LoadSimpleTIFF)
{
	auto image = loadImage("test.tiff");
	ASSERT_TRUE(image);

	const auto height = FreeImage_GetHeight(image->bitmap);
	const auto width = FreeImage_GetWidth(image->bitmap);
	const auto bpp = FreeImage_GetBPP(image->bitmap);
	EXPECT_EQ(16, height);
	EXPECT_EQ(16, width);
	EXPECT_EQ(32, bpp);
}

TEST_F(FreeImageTest, LoadSimpleBMP)
{
	auto image = loadImage("test.bmp");
	ASSERT_TRUE(image);

	const auto height = FreeImage_GetHeight(image->bitmap);
	const auto width = FreeImage_GetWidth(image->bitmap);
	const auto bpp = FreeImage_GetBPP(image->bitmap);
	EXPECT_EQ(16, height);
	EXPECT_EQ(16, width);
	EXPECT_EQ(32, bpp);
}

TEST_F(FreeImageTest, LoadSimpleJPG)
{
	auto image = loadImage("test.jpg");
	ASSERT_TRUE(image);

	const auto height = FreeImage_GetHeight(image->bitmap);
	const auto width = FreeImage_GetWidth(image->bitmap);
	const auto bpp = FreeImage_GetBPP(image->bitmap);
	EXPECT_EQ(16, height);
	EXPECT_EQ(16, width);
	EXPECT_EQ(24, bpp);
}

//--
