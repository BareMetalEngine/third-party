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


// FreeType integration
// NOTE: it's not public, we don't want to leak symbols outside
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
}

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

//--

struct FreeTypeFont
{
	FT_Face face = nullptr;
	std::vector<uint8_t> data;

	FreeTypeFont(FT_Face f, std::vector<uint8_t>&& d)
		: face(f)
		, data(std::move(d))
	{}

	virtual ~FreeTypeFont()
	{
		if (face)
		{
			FT_Done_Face(face);
			face = nullptr;
		}
	}
};

class FreeType : public testing::Test
{
public:
	FT_Library m_library = nullptr;

	virtual void SetUp() override
	{
		FT_Error err = FT_Init_FreeType(&m_library);
		EXPECT_EQ(0, err);
	}


	virtual void TearDown() override
	{
		if (m_library)
		{
			FT_Done_FreeType(m_library);
			m_library = nullptr;
		}
	}

	std::shared_ptr<FreeTypeFont> LoadFont(std::string_view name)
	{
		const auto path = MakeTestDataPath(name);

		std::vector<uint8_t> data;
		if (!LoadFileToBuffer(path, data))
			return nullptr;

		FT_Face face = NULL;
		FT_Error err = FT_New_Memory_Face(m_library, (const FT_Byte*)data.data(), data.size(), 0, &face);
		if (err != 0 || !face)
			return nullptr;

		return std::make_shared<FreeTypeFont>(face, std::move(data));
	}
};

//--

TEST_F(FreeType, OTFLoad)
{
	auto font = LoadFont("test.otf");
	ASSERT_TRUE(font);
}

TEST_F(FreeType, OTFLoadValidFamily)
{
	auto font = LoadFont("test.otf");
	ASSERT_TRUE(font);

	EXPECT_STREQ("Aileron", font->face->family_name);
}

TEST_F(FreeType, OTFLoadValidStyle)
{
	auto font = LoadFont("test.otf");
	ASSERT_TRUE(font);

	EXPECT_STREQ("Regular", font->face->style_name);
}

TEST_F(FreeType, OTFGetGlyph)
{
	auto font = LoadFont("test.otf");
	ASSERT_TRUE(font);

	auto adjustedSize = (FT_UInt)(16 * (float)font->face->units_per_EM / (float)(font->face->ascender - font->face->descender));
	FT_Error err = FT_Set_Pixel_Sizes(font->face, 0, adjustedSize);
	ASSERT_EQ(0, err);

	// load the char into the font
	err = FT_Load_Char(font->face, 'M', FT_LOAD_RENDER);
	ASSERT_EQ(0, err);

	// render the glyph
	FT_Render_Glyph(font->face->glyph, FT_RENDER_MODE_NORMAL);

	// check glyph size
	EXPECT_EQ(9, font->face->glyph->bitmap.rows);
	EXPECT_EQ(10, font->face->glyph->bitmap.width);
}

//--

TEST_F(FreeType, TTFLoad)
{
	auto font = LoadFont("test.ttf");
	ASSERT_TRUE(font);
}

TEST_F(FreeType, TTFLoadValidFamily)
{
	auto font = LoadFont("test.ttf");
	ASSERT_TRUE(font);

	EXPECT_STREQ("Bitstream Vera Sans Mono", font->face->family_name);
}

TEST_F(FreeType, TTFLoadValidStyle)
{
	auto font = LoadFont("test.ttf");
	ASSERT_TRUE(font);

	EXPECT_STREQ("Roman", font->face->style_name);
}

TEST_F(FreeType, TTFGetGlyph)
{
	auto font = LoadFont("test.ttf");
	ASSERT_TRUE(font);

	auto adjustedSize = (FT_UInt)(16 * (float)font->face->units_per_EM / (float)(font->face->ascender - font->face->descender));
	FT_Error err = FT_Set_Pixel_Sizes(font->face, 0, adjustedSize);
	ASSERT_EQ(0, err);

	// load the char into the font
	err = FT_Load_Char(font->face, 'M', FT_LOAD_RENDER);
	ASSERT_EQ(0, err);

	// render the glyph
	FT_Render_Glyph(font->face->glyph, FT_RENDER_MODE_NORMAL);

	// check glyph size
	EXPECT_EQ(9, font->face->glyph->bitmap.rows);
	EXPECT_EQ(8, font->face->glyph->bitmap.width);
}

//--
