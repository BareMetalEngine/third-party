/***
* Bare Metal Engine
* Written by Tomasz Jonarski (RexDex)
* Basic middleware testing project
***/

#include "build.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <string>
#include <string_view>
#include <vector>

#include <lz4.h>
#include <lz4hc.h>


//--

static bool CompressLZ4(const void* data, uint32_t size, std::vector<uint8_t>& output)
{
	auto compressedSize = LZ4_compress_default((const char*)data, (char*)output.data(), (uint32_t)size, (uint32_t)output.size());
	if (compressedSize == 0)
		return false;

	output.resize(compressedSize);

	return true;
}

static bool CompressLZ4HC(const void* data, uint32_t size, std::vector<uint8_t>& output)
{
	void* lzState = alloca(LZ4_sizeofStateHC());

	// compress the data
	auto compressedSize = LZ4_compress_HC_extStateHC(lzState, (const char*)data, (char*)output.data(), (uint32_t)size, (uint32_t)output.size(), LZ4HC_CLEVEL_OPT_MIN);
	if (compressedSize == 0)
		return false;

	output.resize(compressedSize);
	return true;
}

static bool DecompressLZ4(const void* data, uint32_t size, std::vector<uint8_t>& output)
{
	auto compressedBytesProduced = LZ4_decompress_safe((const char*)data, (char*)output.data(), (uint32_t)size, (uint32_t)output.size());
	if (compressedBytesProduced < 0)
		return false;

	output.resize(compressedBytesProduced);
	return true;
}


static std::string BufferToString(const std::vector<uint8_t>& data)
{
	std::string ret;
	ret.resize(data.size());
	memcpy(ret.data(), data.data(), data.size());
	return ret;
}

//--

const std::string_view InputData = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin egestas, erat at efficitur gravida, augue diam accumsan risus, nec sodales velit dui et quam. Nunc lacus augue, vestibulum et commodo ut, fermentum et est. Ut dignissim ante ac venenatis porttitor. Quisque mattis eu leo non posuere. Maecenas volutpat orci eget urna pellentesque congue. Vivamus molestie, massa a vulputate convallis, quam dui scelerisque tortor, at maximus ipsum purus non arcu. Nam sagittis vel velit ac scelerisque.";

//--

TEST(LZ4, BasicCompress)
{
	std::vector<uint8_t> compressed;
	compressed.resize(2 * InputData.size());

	ASSERT_TRUE(CompressLZ4(InputData.data(), (uint32_t)InputData.size(), compressed));

	std::vector<uint8_t> decompressed;
	decompressed.resize(2 * InputData.size());
	ASSERT_TRUE(DecompressLZ4(compressed.data(), (uint32_t)compressed.size(), decompressed));

	const auto sourceStr = std::string(InputData);
	const auto decompressedStr = BufferToString(decompressed);
	ASSERT_STREQ(sourceStr.c_str(), decompressedStr.c_str());
}

TEST(LZ4, BasicCompressHC)
{
	std::vector<uint8_t> compressed;
	compressed.resize(2 * InputData.size());

	ASSERT_TRUE(CompressLZ4HC(InputData.data(), (uint32_t)InputData.size(), compressed));

	std::vector<uint8_t> decompressed;
	decompressed.resize(2 * InputData.size());
	ASSERT_TRUE(DecompressLZ4(compressed.data(), (uint32_t)compressed.size(), decompressed));

	const auto sourceStr = std::string(InputData);
	const auto decompressedStr = BufferToString(decompressed);
	ASSERT_STREQ(sourceStr.c_str(), decompressedStr.c_str());
}

//--
