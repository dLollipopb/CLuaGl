#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "graphics.h"
#include "libtcc.h"

void lua_hook(lua_State* L,lua_Debug* ar)
{
	lua_yield(L,0);
}

typedef struct List
{
	lua_State* thread;
	int threadref;
	int status;
	int args;
	int* argv;
	int func;
	int errfunc;
	struct List* prev;
	struct List* next;
}List;

int tasks;
List* task;
List* nexttask;

int lua_newtask(lua_State* L)
{
	int n=lua_gettop(L);
	if(n<2)
	{
        luaL_error(L,"error arguments count (expected 2 arguments or more)");
        return 0;
	}
	if(!lua_isfunction(L,1))
    {
        luaL_error(L,"argument 1 must be function");
        return 0;
    }
	if(!lua_isfunction(L,2))
    {
        luaL_error(L,"argument 2 must be function");
        return 0;
    }
	if(tasks==0)
	{
		task=malloc(sizeof(List));
		task->next=0;
		task->prev=0;
		task->status=0;
		task->args=n-2;
		task->argv=malloc(sizeof(int*)*task->args);
		for(int i=task->args-1;i>=0;i--)
		    (task->argv)[i]=luaL_ref(L,LUA_REGISTRYINDEX);
		task->errfunc=luaL_ref(L,LUA_REGISTRYINDEX);
		task->func=luaL_ref(L,LUA_REGISTRYINDEX);
		nexttask=task;
		lua_pushinteger(L,task->func);
	}
	else
	{
		List* newtask=malloc(sizeof(List));
		newtask->status=0;
		newtask->prev=nexttask->prev;
		newtask->next=nexttask;
		nexttask->prev=newtask;
		if(newtask->prev)
			newtask->prev->next=newtask;
		if(task==nexttask)
			task=newtask;
		newtask->args=n-2;
		newtask->argv=malloc(sizeof(int*)*newtask->args);
		for(int i=newtask->args-1;i>=0;i--)
		    (newtask->argv)[i]=luaL_ref(L,LUA_REGISTRYINDEX);
		newtask->errfunc=luaL_ref(L,LUA_REGISTRYINDEX);
		newtask->func=luaL_ref(L,LUA_REGISTRYINDEX);
		lua_pushinteger(L,newtask->func);
	}
	tasks++;
	return 1;
}

int lua_update(lua_State* L)
{
	if(tasks!=0)
	{
		int ret;
		if(nexttask->status==0)
		{
			nexttask->status=1;
			nexttask->thread=lua_newthread(L);
			nexttask->threadref=luaL_ref(L,LUA_REGISTRYINDEX);
			lua_sethook(nexttask->thread,lua_hook,LUA_MASKCOUNT,10000);
			lua_rawgeti(nexttask->thread,LUA_REGISTRYINDEX,nexttask->func);
			for(int i=0;i<nexttask->args;i++)
			{
				lua_rawgeti(nexttask->thread,LUA_REGISTRYINDEX,(nexttask->argv)[i]);
				luaL_unref(nexttask->thread,LUA_REGISTRYINDEX,(nexttask->argv)[i]);
			}
			free(nexttask->argv);
			nexttask->argv=0;
			ret=lua_resume(nexttask->thread,0,nexttask->args);
			nexttask->args=0;
		}
		else
			ret=lua_resume(nexttask->thread,0,0);
		if(ret==LUA_YIELD)
		{
			nexttask=nexttask->next;
		}
		else
		{
			if(ret!=0)
			{
				lua_rawgeti(L,LUA_REGISTRYINDEX,nexttask->errfunc);
				lua_pushstring(L,lua_tostring(nexttask->thread,-1));
				int error=lua_pcall(L,1,0,0);
				if(error)
				{
					printf("errfunc error(%i) : %s\n",ret,lua_tostring(nexttask->thread,-1));
					//lua_pop(L,1);
				}
			}
			if(nexttask->prev!=0)
				nexttask->prev->next=nexttask->next;
			if(nexttask->next!=0)
				nexttask->next->prev=nexttask->prev;
			List* nt=nexttask->next;
			if(task==nexttask)
				task=nt;
			luaL_unref(L,LUA_REGISTRYINDEX,nexttask->func);
			luaL_unref(L,LUA_REGISTRYINDEX,nexttask->errfunc);
			free(nexttask);
			nexttask=nt;
			tasks--;
		}
		if(nexttask==0)
			nexttask=task;
		if(ret!=0&&ret!=LUA_YIELD)
		{
			return 1;
		}
	}
	return 0;
}

int lua_closetask(lua_State* L)
{
	int n=lua_gettop(L);
	if(n<1)
	{
        luaL_error(L,"error arguments count (expected 1 argument)");
        return 0;
	}
	if(!lua_isnumber(L,1))
    {
        luaL_error(L,"argument 1 must be integer");
        return 0;
    }
    int func=lua_tointeger(L,1);
	List* current=task;
	List* current2;
	while(current!=0)
	{
		if(current->func==func)
		{
			if(current->prev!=0)
				current->prev->next=current->next;
			if(current->next!=0)
				current->next->prev=current->prev;
			List* nt=current->next;
			if(task==current)
				task=nt;
			if(current->status)
				luaL_unref(L,LUA_REGISTRYINDEX,current->threadref);
			else
			{
				for(int i=0;i<current->args;i++)
					luaL_unref(L,LUA_REGISTRYINDEX,current->argv[i]);
				free(current->argv);
			}
			luaL_unref(L,LUA_REGISTRYINDEX,current->func);
			luaL_unref(L,LUA_REGISTRYINDEX,current->errfunc);
			if(nexttask==current)
				nexttask=current->next;
			current2=current;
			free(current);
			current=current;
			tasks--;
			if(nexttask==0)
				nexttask=task;
			break;
		}
		else
			current=current->next;
	}
	return 0;
}

int lua_createtccstate(lua_State* L)
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

int lua_compile(lua_State* L)
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
    if (tcc_relocate((TCCState*)lua_touserdata(L,1),TCC_RELOCATE_AUTO)<0)
    {
        luaL_error(L,"relocate error");
        return 0;
    }
    return 0;
}

int lua_addsymbol(lua_State* L)
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
    if(!lua_isuserdata(L,3))
    {
        luaL_error(L,"argument 3 must be userdata");
        return 0;
    }
    tcc_add_symbol((TCCState*)lua_touserdata(L,1),lua_tostring(L,2),lua_touserdata(L,3));
    return 0;
}

int lua_getsymbol(lua_State* L)
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
    //lua_pushlightuserdata(L,smb);
    lua_pushcfunction(L,smb);
    return 1;
}

int lua_deletetccstate(lua_State* L)
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

int main()
{
	lua_State* L=luaL_newstate();
	List* task=0;
	tasks=0;
	luaL_openlibs(L);
	lua_pushcfunction(L,&lua_newtask);
	lua_setglobal(L,"newtask");
	lua_pushcfunction(L,&lua_closetask);
	lua_setglobal(L,"closetask");
	lua_pushcfunction(L,&lua_createtccstate);
	lua_setglobal(L,"createtccstate");
	lua_pushcfunction(L,&lua_compile);
	lua_setglobal(L,"compile");
	lua_pushcfunction(L,&lua_addsymbol);
	lua_setglobal(L,"addsymbol");
	lua_pushcfunction(L,&lua_getsymbol);
	lua_setglobal(L,"getsymbol");
	lua_pushcfunction(L,&lua_deletetccstate);
	lua_setglobal(L,"deletetccstate");
	lua_pushlightuserdata(L,(void*)(&lua_pushinteger));
	lua_setglobal(L,"lua_pushinteger");
	lua_pushlightuserdata(L,(void*)(&lua_pushboolean));
	lua_setglobal(L,"lua_pushboolean");
	lua_pushlightuserdata(L,(void*)(&lua_pushstring));
	lua_setglobal(L,"lua_pushstring");
	lua_pushlightuserdata(L,(void*)(&lua_pushlightuserdata));
	lua_setglobal(L,"lua_pushlightuserdata");
	if(graphicsInit(L))
	{
		printf("graphics init error\n");
		return -1;
	}
	int error=luaL_loadstring(L,"require(\"main\")")||lua_pcall(L,0,0,0);
	if(error)
	{
		fprintf(stderr,"%s\n",lua_tostring(L,-1));
		lua_pop(L,1);
	}
	while(tasks!=0)
		lua_update(L);
	//graphicsFinal();
	lua_gc(L,LUA_GCCOLLECT,0);
	lua_close(L);
	glfwTerminate();
	return 0;
};