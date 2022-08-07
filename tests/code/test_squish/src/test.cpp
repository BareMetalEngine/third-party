/***
* Bare Metal Engine
* Written by Tomasz Jonarski (RexDex)
* Basic middleware testing project
***/

#include "build.h"

//--

#include <squish.h>

//--

uint8_t SIMPLE_BLOCK[4][4][4] =
{
	{ {128,128,128,255}, {128,128,128,255}, {128,128,128,0}, {128,128,128,0},},
	{ {128,128,128,255}, {128,128,128,255}, {128,128,128,0}, {128,128,128,0},},
	{ {255,255,255,255}, {128,128,128,255}, {128,128,128,255}, {128,128,128,255},},
	{ {128,128,128,255}, {0,0,0,255}, {128,128,128,255}, {128,128,128,255},},
};

#define DXT1_BLOCK_SIZE 8
#define DXT3_BLOCK_SIZE 16

const char* HexTable =
"00\00001\00002\00003\00004\00005\00006\00007\00008\00009\0000a\0000b\0000c\0000d\0000e\0000f\000"
"10\00011\00012\00013\00014\00015\00016\00017\00018\00019\0001a\0001b\0001c\0001d\0001e\0001f\000"
"20\00021\00022\00023\00024\00025\00026\00027\00028\00029\0002a\0002b\0002c\0002d\0002e\0002f\000"
"30\00031\00032\00033\00034\00035\00036\00037\00038\00039\0003a\0003b\0003c\0003d\0003e\0003f\000"
"40\00041\00042\00043\00044\00045\00046\00047\00048\00049\0004a\0004b\0004c\0004d\0004e\0004f\000"
"50\00051\00052\00053\00054\00055\00056\00057\00058\00059\0005a\0005b\0005c\0005d\0005e\0005f\000"
"60\00061\00062\00063\00064\00065\00066\00067\00068\00069\0006a\0006b\0006c\0006d\0006e\0006f\000"
"70\00071\00072\00073\00074\00075\00076\00077\00078\00079\0007a\0007b\0007c\0007d\0007e\0007f\000"
"80\00081\00082\00083\00084\00085\00086\00087\00088\00089\0008a\0008b\0008c\0008d\0008e\0008f\000"
"90\00091\00092\00093\00094\00095\00096\00097\00098\00099\0009a\0009b\0009c\0009d\0009e\0009f\000"
"a0\000a1\000a2\000a3\000a4\000a5\000a6\000a7\000a8\000a9\000aa\000ab\000ac\000ad\000ae\000af\000"
"b0\000b1\000b2\000b3\000b4\000b5\000b6\000b7\000b8\000b9\000ba\000bb\000bc\000bd\000be\000bf\000"
"c0\000c1\000c2\000c3\000c4\000c5\000c6\000c7\000c8\000c9\000ca\000cb\000cc\000cd\000ce\000cf\000"
"d0\000d1\000d2\000d3\000d4\000d5\000d6\000d7\000d8\000d9\000da\000db\000dc\000dd\000de\000df\000"
"e0\000e1\000e2\000e3\000e4\000e5\000e6\000e7\000e8\000e9\000ea\000eb\000ec\000ed\000ee\000ef\000"
"f0\000f1\000f2\000f3\000f4\000f5\000f6\000f7\000f8\000f9\000fa\000fb\000fc\000fd\000fe\000ff\000";

void BytesToHexString(std::stringstream& str, const uint8_t* data, uint32_t length)
{
	const auto* end = data + length;
	while (data < end)
	{
		const auto byte = *data++;
		const char* txt = HexTable + (3 * byte);
		str << txt;
	}
}

template< typename T >
std::string DataToHex(const T& data)
{
	std::stringstream str;
	BytesToHexString(str, (const uint8_t*)&data, sizeof(data));
	return str.str();
}

//--

TEST(Squish, CompressBlockBC1)
{
	uint8_t block[DXT1_BLOCK_SIZE];
	squish::Compress((const uint8_t*)&SIMPLE_BLOCK, block, squish::kDxt1);

	auto blockHex = DataToHex(block);
	EXPECT_STREQ("0000fffffafaa9a2", blockHex.c_str());
}

TEST(Squish, CompressBlockBC2)
{
	uint8_t block[DXT3_BLOCK_SIZE];
	squish::Compress((const uint8_t*)&SIMPLE_BLOCK, block, squish::kDxt3);

	auto blockHex = DataToHex(block);
	EXPECT_STREQ("ff00ff00fffffffffbde0000aaaaa8a6", blockHex.c_str());
}

TEST(Squish, CompressBlockBC3)
{
	uint8_t block[DXT3_BLOCK_SIZE];
	squish::Compress((const uint8_t*)&SIMPLE_BLOCK, block, squish::kDxt5);

	auto blockHex = DataToHex(block);
	EXPECT_STREQ("00053ff003fffffffbde0000aaaaa8a6", blockHex.c_str());
}

TEST(Squish, CompressBlockBC4)
{
	uint8_t block[DXT1_BLOCK_SIZE];
	squish::Compress((const uint8_t*)&SIMPLE_BLOCK, block, squish::kBc4);

	auto blockHex = DataToHex(block);
	EXPECT_STREQ("8085000000070003", blockHex.c_str());
}

TEST(Squish, CompressBlockBC5)
{
	uint8_t block[DXT3_BLOCK_SIZE];
	squish::Compress((const uint8_t*)&SIMPLE_BLOCK, block, squish::kBc5);

	auto blockHex = DataToHex(block);
	EXPECT_STREQ("80850000000700038085000000070003", blockHex.c_str());
}

//--
