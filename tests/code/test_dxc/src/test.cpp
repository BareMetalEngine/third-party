/***
* Bare Metal Engine
* Written by Tomasz Jonarski (RexDex)
* Basic middleware testing project
***/

#include "build.h"

#ifdef _WIN32
#include <Windows.h>
#include <ole2.h>
#else
#define __EMULATE_UUID 1
#endif

#include <dxc/dxcapi.h>

#pragma optimize("", off)

//--

TEST(Interfaces, CreateLibrary)
{
	IDxcCompiler* compiler = nullptr;
	ASSERT_EQ(0, DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)&compiler));
	ASSERT_NE(nullptr, compiler);

	compiler->Release();
}

TEST(Interfaces, CreateUtils)
{
	IDxcUtils* utils = nullptr;
	ASSERT_EQ(0, DxcCreateInstance(CLSID_DxcUtils, __uuidof(IDxcUtils), (void**)&utils));
	ASSERT_NE(nullptr, utils);

	utils->Release();
}

template< typename T >
struct SimplePtr
{
	T* ptr = nullptr;

	inline SimplePtr() = default;
	inline SimplePtr(T* rawPtr)
		: ptr(rawPtr)
	{}
	inline SimplePtr(const SimplePtr& other)
		: ptr(other.ptr)
	{
		if (ptr) ptr->AddRef();
	}

	SimplePtr& operator=(const SimplePtr& other)
	{
		if (&other != this)
		{
			auto old = ptr;
			ptr = other.ptr;
			if (ptr) ptr->AddRef();
			if (old) old->Release();
		}
		return *this;
	}

	inline ~SimplePtr()
	{
		reset();
	}

	inline void reset()
	{
		if (ptr) ptr->Release();
	}

	inline T* operator->() const
	{
		return ptr;
	}

	inline T* operator*() const
	{
		return ptr;
	}

	inline operator bool() const
	{
		return ptr != nullptr;
	}
};

class DxcTests : public testing::Test
{
public:
	IDxcCompiler* compiler = nullptr;
	IDxcUtils* utils = nullptr;

	virtual void SetUp() override
	{
		ASSERT_EQ(0, DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)&compiler));
		ASSERT_NE(nullptr, compiler);

		ASSERT_EQ(0, DxcCreateInstance(CLSID_DxcUtils, __uuidof(IDxcUtils), (void**)&utils));
		ASSERT_NE(nullptr, utils);
	}

	virtual void TearDown() override
	{
		if (compiler)
		{
			compiler->Release();
			compiler = nullptr;
		}

		if (utils)
		{
			utils->Release();
			utils = nullptr;
		}
	}

#undef CP_UTF8
#define CP_UTF8                   65001       // UTF-8 translation

	SimplePtr<IDxcBlob> CreateTextBlob(std::string_view text)
	{
		IDxcBlobEncoding* blob = nullptr;
		HRESULT hRet = utils->CreateBlob(text.data(), text.length(), CP_UTF8, &blob);
		if (FAILED(hRet))
			return nullptr;

		return blob;
	}
};

TEST_F(DxcTests, SetupTeardown)
{
}

TEST_F(DxcTests, CreateTextBlob)
{
	auto textBlob = CreateTextBlob("Ala ma kota!");
	ASSERT_TRUE(textBlob);
}

TEST_F(DxcTests, TextBlobHasProperSize)
{
	auto textBlob = CreateTextBlob("Ala ma kota!");
	ASSERT_TRUE(textBlob);
	ASSERT_EQ(13, textBlob->GetBufferSize()); // 12 + zero ending
}

TEST_F(DxcTests, TextBlobHasProperText)
{
	auto textBlob = CreateTextBlob("Ala ma kota!");
	ASSERT_TRUE(textBlob);

	const auto* txt = (const char*)textBlob->GetBufferPointer();
	ASSERT_STREQ("Ala ma kota!", txt);
}

//--

TEST_F(DxcTests, IdentityPreprocess)
{
	auto textBlob = CreateTextBlob("Ala ma kota!");
	ASSERT_TRUE(textBlob);

	SimplePtr<IDxcOperationResult> result;
	ASSERT_EQ(0, compiler->Preprocess(textBlob.ptr, L"test.hlsl", nullptr, 0, nullptr, 0, nullptr, &result.ptr));

	HRESULT status = 0;
	result->GetStatus(&status);
	ASSERT_EQ(0, status);

	SimplePtr<IDxcBlob> output;
	ASSERT_EQ(0, result->GetResult(&output.ptr));

	const auto* txt = (const char*)output->GetBufferPointer();
	ASSERT_STREQ("#line 1 \"test.hlsl\"\nAla ma kota!\n", txt);
}

TEST_F(DxcTests, SimpleReplacementFromDefine)
{
	auto textBlob = CreateTextBlob("REPLACE_ME");
	ASSERT_TRUE(textBlob);

	std::vector<DxcDefine> defines;
	defines.emplace_back(DxcDefine{ L"REPLACE_ME", L"Dupa" });

	SimplePtr<IDxcOperationResult> result;
	ASSERT_EQ(0, compiler->Preprocess(textBlob.ptr, L"test.hlsl", nullptr, 0, defines.data(), defines.size(), nullptr, &result.ptr));

	HRESULT status = 0;
	result->GetStatus(&status);
	ASSERT_EQ(0, status);

	SimplePtr<IDxcBlob> output;
	ASSERT_EQ(0, result->GetResult(&output.ptr));

	const auto* txt = (const char*)output->GetBufferPointer();
	ASSERT_STREQ("#line 1 \"test.hlsl\"\nDupa\n", txt);
}

class SimpleIncludeHandler : public IDxcIncludeHandler
{
public:
	SimpleIncludeHandler()
	{}

	void store(std::string_view file, SimplePtr<IDxcBlob> blob)
	{
		IncludeFile entry;
		entry.name = std::wstring(file.begin(), file.end());
		entry.blob = blob;
		m_files.push_back(entry);
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override { return 4317; }

	virtual ULONG STDMETHODCALLTYPE AddRef(void) override { return 1; }

	virtual ULONG STDMETHODCALLTYPE Release(void) override { return 1; }

	virtual HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override
	{
		for (const auto& entry : m_files)
		{
			if (entry.name == pFilename)
			{
				*ppIncludeSource = entry.blob.ptr;
				entry.blob.ptr->AddRef();
				return 0;
			}
		}

		return ERROR_FILE_NOT_FOUND;
	}

private:
	struct IncludeFile
	{
		std::wstring name;
		SimplePtr<IDxcBlob> blob;
	};

	std::vector<IncludeFile> m_files;
};

TEST_F(DxcTests, SimpleIncludeFromHandler)
{
	auto textBlob = CreateTextBlob("First\n#include \"other.hlsl\"\nLast\n");
	ASSERT_TRUE(textBlob);

	auto innerBlob = CreateTextBlob("Middle");
	ASSERT_TRUE(innerBlob);

	SimpleIncludeHandler includes;
	includes.store("./other.hlsl", innerBlob);

	std::vector<DxcDefine> defines;

	SimplePtr<IDxcOperationResult> result;
	ASSERT_EQ(0, compiler->Preprocess(textBlob.ptr, L"test.hlsl", nullptr, 0, defines.data(), defines.size(), &includes, &result.ptr));

	HRESULT status = 0;
	result->GetStatus(&status);
	ASSERT_EQ(0, status);

	SimplePtr<IDxcBlob> output;
	ASSERT_EQ(0, result->GetResult(&output.ptr));

	const auto* txt = (const char*)output->GetBufferPointer();
	ASSERT_STREQ("#line 1 \"test.hlsl\"\nFirst\n\n#line 1 \"./other.hlsl\"\nMiddle\n#line 3 \"test.hlsl\"\nLast\n", txt);
}

static const char* TRIVIAL_PS =
R""""(
float4 ps_main() : SV_TARGET
{
	return float4(0,1,1,0);
}
)"""";


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

std::string BlobToHexString(IDxcBlob* blob)
{
	std::stringstream str;
	BytesToHexString(str, (const uint8_t*)blob->GetBufferPointer(), blob->GetBufferSize());
	return str.str();
}

static const char* TRIVIAL_PS_RESULT = "445842430000000000000000000000000000000001000000f2090000070000003c0000004c0000005c00000096000000fa000000620500007e05000053464930080000000000000000000000495347310800000000000000080000004f5347313200000001000000080000000000000028000000000000004000000003000000000000000f0000000000000053565f54617267657400505356305c000000300000000000000000000000000000000000000000000000ffffffff00000000000100000100000000000000000000000000000000000000040000000000000001000000000000001000000000000000000000000100441003000000535441546004000060000000180100004458494c0001000010000000480400004243c0de210c00000f0100000b82200002000000130000000781239141c80449061032399201840c250508191e048b628010450242920b42841032143808184b0a32428848901420434688a500193242e4480e901122c4504151818ce183e58a0421460651180000030000001b88e0ffffffff0740020000491800000100000013820000892000000e00000032220809206485041322a484041322e384a19014124c888c0b84844c1028230025008a39023098234066008a013343453610900203000000131472c08774608736688779680372c0870daf500e6dd00e7a500e6d000f7a300772a0077320076d900e71a0077320076d900e78a0077320076d900e7160077a300772d006e9300772a0077320076d900e7640077a600774d006e6100776a0077320076d600e7320077a300772d006e6600774a0077640076de00e78a0077160077a300772a007764007439e0008000000000000000000863c0610000100000000000000648100000a000000321e981019114c908c092647c604439a121801288632288f522804a2922890118042a01c4b00080079180000520000001a034c90460213443518630b733b03b12b939b4b7b73039971b90141a10b3b9b7b912a622a0a9a2afa9ab98179314b730b634bd9100413844198200cc306612036080441016e6e8230101b860321260802b001d83010cbb22160360c83d24c10926643f0f0809bfb6a0b4b73633265f5451526775646374120920902a16c08880902b14c1008668230141384c1d82060d986859026aab2868bb0b40dc1b66100386043a1441d00b048739ba39b9b200c078db9b4b3af39ba09c2806c203e30080331a8c2c666d7e6924656e646372508aa90e1b9d895c9cda5bdb94d09882664782e76616c7665725382a20e199ecb1c5a1859995cd31b5919db9400a94486e74297075716e4e6f646174697f6e636372568ea90e1b9d8a595dd25914dd185d1954d099e3a64782e656e747279506f696e74735382ae0b199ecbd85b9d1b5d99dcdc94400c000000791800004c0000003308801cc4e11c6614013d88433884c38c4280077978077398710ce6000fed100ef4800e330c421ec2c11dcea11c6630053d88433884831bcc033dc8433d8c033dcc788c7470077b08077948877070077a700376788770208719cc110eec900ee1300f6e300fe3f00ef0500e3310c41dde211cd8211dc2611e6630893bbc833bd04339b4033cbc833c84033bccf0147660077b680737688772680737808770908770600776280776f8057678877780875f08877118877298877998812ceef00eeee00ef5c00eec300362c8a11ce4a11ccca11ce4a11cdc611cca211cc4811dca6106d6904339c84339984339c84339b8c33894433888033b94c32fbc833cfc823bd4033bb0c30cc421077c70037a288776808719d1430ef8e006e4200ee7e006f6100ef2c00ee1900fef500ff4000000712000000700000016500d97ef3cbe3439118152d3434d7e71db06703cd2cf004803000000000000484153481400000000000000b663faf7a3b561caa2e69ae176a0d9604458494c6c040000600000001b0100004458494c0001000010000000540400004243c0de210c0000120100000b82200002000000130000000781239141c80449061032399201840c250508191e048b628010450242920b42841032143808184b0a32428848901420434688a500193242e4480e901122c4504151818ce183e58a0421460651180000030000001b88e0ffffffff0740020000491800000100000013820000892000000e00000032220809206485041322a484041322e384a19014124c888c0b84844c1028230025008a39023098234066008a013343453610900203000000131472c08774608736688779680372c0870daf500e6dd00e7a500e6d000f7a300772a0077320076d900e71a0077320076d900e78a0077320076d900e7160077a300772d006e9300772a0077320076d900e7640077a600774d006e6100776a0077320076d600e7320077a300772d006e6600774a0077640076de00e78a0077160077a300772a007764007439e0000000000000000000000863c0610000100000000000000648100000a000000321e981019114c908c092647c604439a121801288632280fa2922890118042a01c4b000800000000791800003f0000001a034c90460213443518630b733b03b12b939b4b7b73039971b90141a10b3b9b7b912a622a0a9a2afa9ab98179314b730b634bd9100413844198200cc3066120260803b141180c0a70731b06c4202608c9b22150260802c0036eeeab2d2ccd8dc994d5175598dc5919dd048138260804b221202608443241209409c2504c1006638340551b16c279a0481a2642b23604d78601c0800d05d3640050858dcdaecd258daccc8d6e4a105421c373b12b939b4b7b739b12104dc8f05cecc2d8eccae4a604461d323c9739b430b232b9a637b232b6290152870ccfc52eadec2e896c8a2e8cae6c4aa0d421c3732973a393cb837a4b73a39b9b126400000000791800004c0000003308801cc4e11c6614013d88433884c38c4280077978077398710ce6000fed100ef4800e330c421ec2c11dcea11c6630053d88433884831bcc033dc8433d8c033dcc788c7470077b08077948877070077a700376788770208719cc110eec900ee1300f6e300fe3f00ef0500e3310c41dde211cd8211dc2611e6630893bbc833bd04339b4033cbc833c84033bccf0147660077b680737688772680737808770908770600776280776f8057678877780875f08877118877298877998812ceef00eeee00ef5c00eec300362c8a11ce4a11ccca11ce4a11cdc611cca211cc4811dca6106d6904339c84339984339c84339b8c33894433888033b94c32fbc833cfc823bd4033bb0c30cc421077c70037a288776808719d1430ef8e006e4200ee7e006f6100ef2c00ee1900fef500ff4000000712000000700000016500d97ef3cbe3439118152d3434d7e71db06703cd2cf004803000061200000140000001304412c100000000500000034a54054024550065423006304200882f80700002306090082606044c6f324c28841028020181891f13c453062900020080646643c0f118c1824000882811119cf33080800000000";

TEST_F(DxcTests, CompileSimpleShader)
{
	auto textBlob = CreateTextBlob(TRIVIAL_PS);
	ASSERT_TRUE(textBlob);

	SimplePtr<IDxcOperationResult> result;
	ASSERT_EQ(0, compiler->Compile(textBlob.ptr, L"test.hlsl", L"ps_main", L"ps_6_0", nullptr, 0, nullptr, 0, nullptr, &result.ptr));

	HRESULT status = 0;
	result->GetStatus(&status);
	ASSERT_EQ(0, status);

	SimplePtr<IDxcBlob> output;
	ASSERT_EQ(0, result->GetResult(&output.ptr));

	const auto* bytecode = (const char*)output->GetBufferPointer();
	ASSERT_EQ(0, strncmp(bytecode, "DXBC", 4));

	const auto byteCodeStr = BlobToHexString(*output);
	//EXPECT_STREQ(TRIVIAL_PS_RESULT, byteCodeStr.c_str());
}

static const char* TRIVIAL_PS_DISASSM = 
R""""(;
; Input signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; no parameters
;
; Output signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; SV_Target                0   xyzw        0   TARGET   float   xyzw
;
; shader hash: b663faf7a3b561caa2e69ae176a0d960
;
; Pipeline Runtime Information: 
;
; Pixel Shader
; DepthOutput=0
; SampleFrequency=0
;
;
; Output signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; SV_Target                0                              
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
;
;
; ViewId state:
;
; Number of inputs: 0, outputs: 4
; Outputs dependent on ViewId: {  }
; Inputs contributing to computation of Outputs:
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @ps_main() {
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 1.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 1.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

attributes #0 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.viewIdState = !{!4}
!dx.entryPoints = !{!5}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 6}
!3 = !{!"ps", i32 6, i32 0}
!4 = !{[2 x i32] [i32 0, i32 4]}
!5 = !{void ()* @ps_main, !"ps_main", !6, null, null}
!6 = !{null, !7, null}
!7 = !{!8}
!8 = !{i32 0, !"SV_Target", i8 9, i8 16, !9, i8 0, i32 1, i8 4, i32 0, i8 0, !10}
!9 = !{i32 0}
!10 = !{i32 3, i32 15}
)"""";

TEST_F(DxcTests, CompileSimpleShaderAndDisassemble)
{
	auto textBlob = CreateTextBlob(TRIVIAL_PS);
	ASSERT_TRUE(textBlob);

	SimplePtr<IDxcOperationResult> result;
	ASSERT_EQ(0, compiler->Compile(textBlob.ptr, L"test.hlsl", L"ps_main", L"ps_6_0", nullptr, 0, nullptr, 0, nullptr, &result.ptr));

	HRESULT status = 0;
	result->GetStatus(&status);
	ASSERT_EQ(0, status);

	SimplePtr<IDxcBlob> output;
	ASSERT_EQ(0, result->GetResult(&output.ptr));

	SimplePtr<IDxcBlobEncoding> dis;
	ASSERT_EQ(0, compiler->Disassemble(output.ptr, &dis.ptr));

	const auto* disTxt = (const char*)dis->GetBufferPointer();
    EXPECT_STRNE("", disTxt);
	//EXPECT_STREQ(TRIVIAL_PS_DISASSM, disTxt);
}

//--
