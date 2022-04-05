#include "libtcc.h"

int lua_createtccstate(Lua_State* L)
{
	TCCState* s=tcc_new();
	if (!s) {
        luaL_error(L,"Could not create tcc state");
        return 0;
    }
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
    lua_pushlightuserdata(L,s);
    return 1;
}

int lua_compile(Lua_State* L)
{
    int n=lua_gettop(L);
    if(n<2)
    {
        luaL_error(L,"error arguments count (expected 2 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isstring(L,2))
    {
        luaL_error(L,"argument 2 must be string");
        return 0;
    }
    if(tcc_compile_string((TCCState*)lua_touserdata(L,1),lua_tostring(L,2))==-1)
    {
        luaL_error(L,"compile error");
        return 0;
    }
    if (tcc_relocate(s,TCC_RELOCATE_AUTO)<0)
    {
        luaL_error(L,"relocate error");
        return 0;
    }
    return 0;
}

int lua_addsymbol(Lua_State* L)
{
    int n=lua_gettop(L);
    if(n<3)
    {
        luaL_error(L,"error arguments count (expected 3 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isstring(L,2))
    {
        luaL_error(L,"argument 2 must be string");
        return 0;
    }
    if(!lua_isuserdata(L,3)&&!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be pointer");
        return 0;
    }
    if(!lua_isuserdata(L,3))
    	tcc_add_symbol((TCCState*)lua_touserdata(L,1),lua_tostring(L,2),lua_touserdata(L,3));
    if(!lua_isnumber(L,3))
    	tcc_add_symbol((TCCState*)lua_touserdata(L,1),lua_tostring(L,2),lua_tointeger(L,3));
    return 0;
}

int lua_getsymbol(Lua_State* L)
{
    int n=lua_gettop(L);
    if(n<2)
    {
        luaL_error(L,"error arguments count (expected 2 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isstring(L,2))
    {
        luaL_error(L,"argument 2 must be string");
        return 0;
    }
    void* smb=tcc_get_symbol((TCCState*)lua_touserdata(L,1),lua_tostring(L,2));
    lua_pushlightuserdata(L,smb);
    return 1;
}

int lua_deletetccstate(Lua_State* L)
{
    int n=lua_gettop(L);
    if(n<1)
    {
        luaL_error(L,"error arguments count (expected 1 argument)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    tcc_delete((TCCState*)lua_touserdata(L,1));
    return 0;
}