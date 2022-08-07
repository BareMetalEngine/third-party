/***
* Bare Metal Engine
* Written by Tomasz Jonarski (RexDex)
* Basic middleware testing project
***/

#include "build.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
}

//--

static void* l_alloc(void* ud, void* ptr, size_t osize,
	size_t nsize) {
	(void)ud;  (void)osize;  /* not used */
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

TEST(LuaState, StateCreated)
{
	lua_State* s = lua_newstate(&l_alloc, nullptr);
	ASSERT_NE(nullptr, s);

	lua_close(s);
}

//--

class LuaTest : public testing::Test
{
public:
	lua_State* L = nullptr;

	static bool panicCalled;
	static std::string panicString;

	static int PanicHandler(lua_State* L)
	{
		panicCalled = true;
		panicString = lua_tostring(L, -1);
		return 0;		
	}

	virtual void SetUp() override
	{
		L = lua_newstate(&l_alloc, this);
		ASSERT_NE(nullptr, L);
	}

	virtual void TearDown() override
	{
		lua_close(L);
	}
};

bool LuaTest::panicCalled = false;
std::string LuaTest::panicString = "";

//--

extern "C" int api_getVal(lua_State * lua)
{
	lua_pushinteger(lua, 5);
	return 1;
}

static bool NativeFuctionCalled = false;

extern "C" int api_mul(lua_State * lua)
{
	lua_settop(lua, 2); // set the size of the stack to 2 and crop useless args

	int isnum;
	auto num = lua_tointegerx(lua, 1, &isnum);
	auto num2 = lua_tointegerx(lua, 2, &isnum);

	lua_pushinteger(lua, num * num2);
	NativeFuctionCalled = true;
	return 1;
}

TEST_F(LuaTest, CodeParses)
{
	const char* code = "function Test(a,b)\n  return a+b\nend\n";

	ASSERT_EQ(LUA_OK, luaL_loadstring(L, code));
	ASSERT_EQ(LUA_OK, lua_pcall(L, 0, LUA_MULTRET, 0));
}

TEST_F(LuaTest, CallLuaFunctionFromC)
{
	const char* code = "function Test(a,b)\n  return a+b\nend\n";

	ASSERT_EQ(LUA_OK, luaL_loadstring(L, code));
	ASSERT_EQ(LUA_OK, lua_pcall(L, 0, LUA_MULTRET, 0));

	lua_getglobal(L, "Test");
	lua_pushnumber(L, 3);
	lua_pushnumber(L, 5);

	ASSERT_EQ(LUA_OK, lua_pcall(L, 2, 1, 0));
	ASSERT_TRUE(lua_isnumber(L, -1));

	int ret = lua_tonumber(L, -1);
	lua_pop(L, 1);

	ASSERT_EQ(8, ret);
}

TEST_F(LuaTest, CallCFunctionFromLua)
{
	lua_register(L, "mul", api_mul);

	NativeFuctionCalled = false;

	const char* code = "return mul(3,5)\n";

	ASSERT_EQ(LUA_OK, luaL_loadstring(L, code));
	ASSERT_EQ(LUA_OK, lua_pcall(L, 0, 1, 0));
	ASSERT_TRUE(NativeFuctionCalled);

	int ret = lua_tonumber(L, -1);
	lua_pop(L, 1);

	ASSERT_EQ(15, ret);
}

TEST_F(LuaTest, CallCFunctionFromCViaLua)
{
	lua_register(L, "mul", api_mul);

	NativeFuctionCalled = false;

	lua_getglobal(L, "mul");
	lua_pushnumber(L, 4);
	lua_pushnumber(L, 5);
	ASSERT_EQ(LUA_OK, lua_pcall(L, 2, 1, 0));

	ASSERT_TRUE(NativeFuctionCalled);

	int ret = lua_tonumber(L, -1);
	lua_pop(L, 1);

	ASSERT_EQ(20, ret);
}

//--