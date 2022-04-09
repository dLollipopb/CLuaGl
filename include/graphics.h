#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <memwatch.h>
#include <ft2build.h>
#include <freetype/freetype.h>

lua_State* L;

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            GLuint id, 
                            GLenum severity, 
                            GLsizei length, 
                            const GLchar *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    printf("-----------------------\n");
    printf("Debug message ( %i ): %s\n",id,message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             printf("Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   printf("Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     printf("Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     printf("Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER:           printf("Source: Other"); break;
    } printf("\n\n");

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               printf("Type: Error"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("Type: Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  printf("Type: Undefined Behaviour"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         printf("Type: Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         printf("Type: Performance"); break;
        case GL_DEBUG_TYPE_MARKER:              printf("Type: Marker"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          printf("Type: Push Group"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           printf("Type: Pop Group"); break;
        case GL_DEBUG_TYPE_OTHER:               printf("Type: Other"); break;
    } printf("\n\n");

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         printf("Severity: high"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       printf("Severity: medium"); break;
        case GL_DEBUG_SEVERITY_LOW:          printf("Severity: low"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: printf("Severity: notification"); break;
    } printf("\n\n");
}

void KeyCB(GLFWwindow* window, int key, int scancode, int action, int mods);

typedef struct KeyData
{
    GLFWwindow* win;
    int func;
    int errfunc;
    int active;
    struct KeyData* prev;
    struct KeyData* next;
}KeyData;
KeyData* keydataroot;
KeyData* keydataend;

int lua_createwindow(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<3)
    {
        luaL_error(L,"error arguments count (expected 3 arguments)");
        return 0;
    }
    if(!lua_isnumber(L,1))
    {
        luaL_error(L,"argument 1 must be value");
        return 0;
    }
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be value");
        return 0;
    }
    if(!lua_isstring(L,3))
    {
        luaL_error(L,"argument 3 must be string");
        return 0;
    }
    GLFWwindow* win=glfwCreateWindow(lua_tointeger(L,1),lua_tointeger(L,2),lua_tostring(L,3),0,0);
    glfwMakeContextCurrent(win);

    //glfwSetKeyCallback(win,0);
    //glfwSetKeyCallback(win,KeyCB);

    if(gl3wInit())
    {
        luaL_error(L,"gl3wInit failed");
        return 0;
    }
    glGetError();
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS,&flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        printf("OpenGL debugging context is created.\n");
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(glDebugOutput, 0);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, 0, GL_TRUE);
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    if(keydataroot==0)
    {
        keydataroot=malloc(sizeof(KeyData));
        keydataroot->active=0;
        keydataroot->prev=0;
        keydataroot->next=0;
        keydataroot->win=win;
        keydataend=keydataroot;
    }
    else
    {
        KeyData* newkeydata=malloc(sizeof(KeyData));
        newkeydata->active=0;
        newkeydata->prev=keydataend;
        keydataend=newkeydata;
        newkeydata->next=0;
        newkeydata->win=win;
    }
    lua_pushlightuserdata(L,win);
    return 1;
}

int lua_depthtest(lua_State* L)
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
    if(!lua_isboolean(L,2))
    {
        luaL_error(L,"argument 2 must be boolean");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    if(lua_tointeger(L,2)==0)
        glDisable(GL_DEPTH_TEST);
    else
        glEnable(GL_DEPTH_TEST);
    return 0;
}

int lua_cullface(lua_State* L)
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
    if(!lua_isboolean(L,2))
    {
        luaL_error(L,"argument 2 must be boolean");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    if(lua_tointeger(L,2)==0)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);
    return 0;
}

int lua_windowshouldclose(lua_State* L)
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
    lua_pushboolean(L,glfwWindowShouldClose((GLFWwindow*)lua_touserdata(L,1)));
    return 1;
}

int lua_destroywindow(lua_State* L)
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
    GLFWwindow* win=lua_touserdata(L,1);
    KeyData* current=keydataroot;
    while(current!=0)
    {
        if(current->win==win)
        {
            if(current->active)
                luaL_unref(L,LUA_REGISTRYINDEX,current->func);
            if(current->prev==0)
            {
                if(current->next==0)
                    keydataroot=0;
                else
                {
                    keydataroot=current->next;
                    keydataroot->prev=0;
                }
            }
            else
            {
                if(current->next==0)
                {
                    keydataend=current->prev;
                    keydataend->next=0;
                }
                else
                {
                    current->prev->next=current->next;
                    current->next->prev=current->prev;
                }
            }
            free(current);
            break;
        }
        current=current->next;
    }
    glfwDestroyWindow(win);
    return 0;
}

int lua_swapbuffers(lua_State* L)
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
    glfwSwapBuffers(lua_touserdata(L,1));
    return 0;
}

int lua_pollevents(lua_State* L)
{
    glfwPollEvents();
    return 0;
}

int lua_vertexshader(lua_State* L)
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
        luaL_error(L,"argument 2 must be userdata");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    const char* src=lua_tostring(L,2);
    int shader=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader,1,&src,0);
    glCompileShader(shader);
    int success;
    char buf[512];
    glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(shader,512,0,buf);
        glDeleteShader(shader);
        luaL_error(L,buf);
        return 0;
    }
    lua_pushinteger(L,shader);
    return 1;
}

int lua_fragmentshader(lua_State* L)
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
        luaL_error(L,"argument 2 must be userdata");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    const char* src=lua_tostring(L,2);
    int shader=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader,1,&src,0);
    glCompileShader(shader);
    int success;
    char buf[512];
    glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        glGetShaderInfoLog(shader,512,0,buf);
        glDeleteShader(shader);
        luaL_error(L,buf);
        return 0;
    }
    lua_pushinteger(L,shader);
    return 1;
}

int lua_deletesubshader(lua_State* L)
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
    glDeleteShader(lua_tointeger(L,1));
    return 0;
}

int lua_compileshader(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<2)
    {
        luaL_error(L,"error arguments count (expected 2 and more arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    int program=glCreateProgram();
    for(int i=2;i<=n;i++)
    {
        if(!lua_isuserdata(L,1))
        {
            glDeleteProgram(program);
            luaL_error(L,"argument %d must be userdata",i);
            return 0;
        }
        glAttachShader(program,lua_tointeger(L,i));
    }
    glLinkProgram(program);
    int success;
    char buf[512];
    glGetProgramiv(program,GL_LINK_STATUS,&success);
    if(!success)
    {
        glGetProgramInfoLog(program,512,0,buf);
        glDeleteProgram(program);
        luaL_error(L,buf);
        return 0;
    }
    lua_pushinteger(L,program);
    return 1;
}

int lua_deleteshader(lua_State* L)
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
    glDeleteProgram(lua_tointeger(L,1));
    return 0;
}

typedef struct vList
{
    char* name;
    int type;
    int size;
    int count;
    int vertices;
    struct vListsub* data;
    struct vListsub* end;
}vList;

typedef struct vListsub
{
    void* data;
    struct vListsub* next;
}vListsub;

int lua_attributeexists(lua_State* L)
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
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be integer");
        return 0;
    }
    if(!lua_isstring(L,3))
    {
        luaL_error(L,"argument 3 must be string");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    if(glGetAttribLocation(lua_tointeger(L,2),lua_tostring(L,3))>=0)
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);
    return 1;
}

int lua_createvertexdata(lua_State* L)
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
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be integer");
        return 0;
    }
    if(!lua_isstring(L,3))
    {
        luaL_error(L,"argument 3 must be string");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    vList* vertex=malloc(sizeof(vList));
    int program=lua_tointeger(L,2);
    const char* name=lua_tostring(L,3);
    int type;
    int attrloc=glGetAttribLocation(program,name);
    if(attrloc<0)
    {
        luaL_error(L,"attribute \"%s\" it not valid",name);
        return 0;
    }
    glGetActiveAttrib(program,attrloc,0,0,0,&type,0);
    if(type==GL_FLOAT)
    {
        vertex->type=GL_FLOAT;
        vertex->count=1;
        vertex->size=4;
    }
    else if(GL_FLOAT_VEC2<=type<=GL_FLOAT_VEC4)
    {
        vertex->type=GL_FLOAT;
        vertex->count=type-GL_FLOAT_VEC2+2;
        vertex->size=4;
    }
    else if(type==GL_INT)
    {
        vertex->type=GL_INT;
        vertex->count=1;
        vertex->size=4;
    }
    else if(GL_INT_VEC2<=type<=GL_INT_VEC4)
    {
        vertex->type=GL_INT;
        vertex->count=type-GL_INT_VEC2+2;
        vertex->size=4;
    }
    else if(type==GL_UNSIGNED_INT)
    {
        vertex->type=GL_UNSIGNED_INT;
        vertex->count=1;
        vertex->size=4;
    }
    else if(GL_UNSIGNED_INT_VEC2<=type<=GL_UNSIGNED_INT_VEC4)
    {
        vertex->type=GL_UNSIGNED_INT;
        vertex->count=type-GL_UNSIGNED_INT_VEC2+2;
        vertex->size=4;
    }
    else if(type==GL_DOUBLE)
    {
        vertex->type=GL_DOUBLE;
        vertex->count=1;
        vertex->size=8;
    }
    else if(GL_DOUBLE_VEC2<=type<=GL_DOUBLE_VEC4)
    {
        vertex->type=GL_DOUBLE;
        vertex->count=type-GL_DOUBLE_VEC2+2;
        vertex->size=8;
    }
    else
    {
        free(vertex);
        luaL_error(L,"unknown attribute type");
        return 0;
    }
    vertex->vertices=0;
    vertex->data=0;
    vertex->end=0;
    vertex->name=malloc(strlen(name)+1);
    strcpy(vertex->name,name);
    lua_pushlightuserdata(L,vertex);
    return 1;
}

int lua_addvertexdata(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<1)
    {
        luaL_error(L,"error arguments count");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    vList* vertex=(vList*)lua_touserdata(L,1);
    if(n<vertex->count+1)
    {
        luaL_error(L,"error arguments count (expected %d arguments)",vertex->count+1);
        return 0;
    }
    vListsub* group=malloc(sizeof(vListsub));
    group->data=malloc(vertex->size*vertex->count);
    group->next=0;
    for(int i=2;i<=vertex->count+1;i++)
    {
        if(vertex->type==GL_BYTE)
        {
            if(!lua_isnumber(L,i))
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be byte",i);
                return 0;
            }
            if(lua_tointeger(L,i)>127||lua_tointeger(L,i)<-129)
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be byte",i);
                return 0;
            }
            ((char*)(group->data))[i-2]=(char)lua_tointeger(L,i);
        }
        else if(vertex->type==GL_UNSIGNED_BYTE)
        {
            if(!lua_isnumber(L,i))
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be unsigned byte",i);
                return 0;
            }
            if(lua_tointeger(L,i)>255||lua_tointeger(L,i)<0)
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be unsigned byte",i);
                return 0;
            }
            ((unsigned char*)(group->data))[i-2]=(unsigned char)lua_tointeger(L,i);
        }
        else if(vertex->type==GL_SHORT)
        {
            if(!lua_isnumber(L,i))
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be short",i);
                return 0;
            }
            if(lua_tointeger(L,i)>32767||lua_tointeger(L,i)<-32769)
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be short",i);
                return 0;
            }
            ((short*)(group->data))[i-2]=(short)lua_tointeger(L,i);
        }
        else if(vertex->type==GL_UNSIGNED_SHORT)
        {
            if(!lua_isnumber(L,i))
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be unsigned short",i);
                return 0;
            }
            if(lua_tointeger(L,i)>65535||lua_tointeger(L,i)<0)
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be unsigned short",i);
                return 0;
            }
            ((unsigned short*)(group->data))[i-2]=(unsigned short)lua_tointeger(L,i);
        }
        else if(vertex->type==GL_INT)
        {
            if(!lua_isnumber(L,i))
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be int",i);
                return 0;
            }
            if(lua_tointeger(L,i)>2147483647||lua_tointeger(L,i)<-2147483649)
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be int",i);
                return 0;
            }
            ((int*)(group->data))[i-2]=(int)lua_tointeger(L,i);
        }
        else if(vertex->type==GL_UNSIGNED_INT)
        {
            if(!lua_isnumber(L,i))
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be unsigned int",i);
                return 0;
            }
            if(lua_tointeger(L,i)>4294967295||lua_tointeger(L,i)<0)
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be unsigned int",i);
                return 0;
            }
            ((unsigned int*)(group->data))[i-2]=(unsigned int)lua_tointeger(L,i);
        }
        else if(vertex->type==GL_FLOAT)
        {
            if(!lua_isnumber(L,i))
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be float",i);
                return 0;
            }
            ((float*)(group->data))[i-2]=(float)lua_tonumber(L,i);
        }
        else if(vertex->type==GL_DOUBLE)
        {
            if(!lua_isnumber(L,i))
            {
                free(group->data);
                free(group);
                luaL_error(L,"argument %d must be double",i);
                return 0;
            }
            ((double*)(group->data))[i-2]=(double)lua_tonumber(L,i);
        }
    }
    if(vertex->vertices==0)
    {
        vertex->data=group;
        vertex->end=group;
    }
    else
    {
        vertex->end->next=group;
        vertex->end=group;
    }
    vertex->vertices++;
    return 0;
}

int lua_deletevertexdata(lua_State* L)
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
    vList* vertex=(vList*)lua_touserdata(L,1);
    vListsub* group=vertex->data;
    for(int i=0;i<vertex->vertices;i++)
    {
        vListsub* prev=group;
        group=group->next;
        free(prev->data);
        free(prev);
    }
    free(vertex->name);
    free(vertex);
    return 0;
}

typedef struct sVBO
{
    char* name;
    int vbobuf;
    int count;
    int type;
    int size;
    int vertices;
}sVBO;

int lua_createvbo(lua_State* L)
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
    if(!lua_isuserdata(L,2))
    {
        luaL_error(L,"argument 2 must be userdata");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    vList* vertex=(vList*)lua_touserdata(L,2);
    void* data=malloc(vertex->size*vertex->count*vertex->vertices);
    vListsub* group=vertex->data;
    for(int i=0;i<vertex->vertices;i++)
    {
        for(int j=0;j<vertex->count;j++)
        {
            if(vertex->type==GL_BYTE||vertex->type==GL_UNSIGNED_BYTE)
                ((char*)data)[j+i*vertex->count]=((char*)(group->data))[j];
            else if(vertex->type==GL_SHORT||vertex->type==GL_UNSIGNED_SHORT)
                ((short*)data)[j+i*vertex->count]=((short*)(group->data))[j];
            else if(vertex->type==GL_INT||vertex->type==GL_UNSIGNED_INT)
                ((int*)data)[j+i*vertex->count]=((int*)(group->data))[j];
            else if(vertex->type==GL_FLOAT)
                ((float*)data)[j+i*vertex->count]=((float*)(group->data))[j];
            else if(vertex->type==GL_DOUBLE)
                ((double*)data)[j+i*vertex->count]=((double*)(group->data))[j];
        }
        group=group->next;
    }
    int vbobuf;
    glGenBuffers(1,&vbobuf);

    glBindBuffer(GL_ARRAY_BUFFER,vbobuf);
    glBufferData(GL_ARRAY_BUFFER,vertex->size*vertex->count*vertex->vertices,data,GL_STATIC_DRAW);
    free(data);
    sVBO* vbo=malloc(sizeof(sVBO));
    vbo->vbobuf=vbobuf;
    vbo->vertices=vertex->vertices;
    vbo->count=vertex->count;
    vbo->size=vertex->size;
    vbo->type=vertex->type;
    vbo->name=malloc(strlen(vertex->name)+1);
    strcpy(vbo->name,vertex->name);
    lua_pushlightuserdata(L,vbo);
    return 1;
}

int lua_deletevbo(lua_State* L)
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
    sVBO* vbo=lua_touserdata(L,1);
    glDeleteBuffers(1,&vbo->vbobuf);
    free(vbo->name);
    free(vbo);
    return 0;
}

typedef struct sVAO
{
    int vbocount;
    sVBO** vbo;
    int vaobuf;
    int vertices;
    int type;
}sVAO;

int lua_createvao(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<4)
    {
        luaL_error(L,"error arguments count (expected 4 or more arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be integer");
        return 0;
    }
    if(!lua_isstring(L,3))
    {
        luaL_error(L,"argument 3 must be string");
        return 0;
    }
    for(int i=4;i<=n;i++)
    {
        if(!lua_isuserdata(L,i))
        {
            luaL_error(L,"argument %d must be userdata",i);
            return 0;
        }
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    int program=lua_tointeger(L,2);
    sVAO* vao=malloc(sizeof(sVAO));
    vao->vbocount=n-2;
    const char* type=lua_tostring(L,3);
    if(strcmp(type,"triangles")==0)
        vao->type=GL_TRIANGLES;
    glGenVertexArrays(1,&vao->vaobuf);
    glBindVertexArray(vao->vaobuf);

    int vertices=((sVBO*)lua_touserdata(L,4))->vertices;

    for(int i=4;i<=n;i++)
    {
        sVBO* vbo=lua_touserdata(L,i);
        glBindBuffer(GL_ARRAY_BUFFER,vbo->vbobuf);
        int attrloc=glGetAttribLocation(program,vbo->name);
        glVertexAttribPointer(attrloc,vbo->count,vbo->type,GL_FALSE,0,0);
        glEnableVertexAttribArray(attrloc);
    }
    vao->vertices=vertices;
    lua_pushlightuserdata(L,vao);
    return 1;
}


int lua_deletevao(lua_State* L)
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
    sVAO* vao=lua_touserdata(L,1);
    glDeleteBuffers(1,&vao->vaobuf);
    free(vao);
    return 0;
}

lua_Number getd(lua_State* L,int n)
{
    lua_rawgeti(L,-1,n);
    if(lua_isnil(L,-1))
    {
        luaL_error(L,"uniform bind error");
        return 0;
    }
    double o=lua_tonumber(L,-1);
    lua_pop(L,1);
    return o;
}

int geti(lua_State* L,int n)
{
    lua_rawgeti(L,-1,n);
    if(lua_isnil(L,-1))
    {
        luaL_error(L,"uniform bind error");
        return 0;
    }
    int o=lua_tointeger(L,-1);
    lua_pop(L,1);
    return o;
}

int lua_drawmesh(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<3)
    {
        luaL_error(L,"error arguments count (expected 3 or 4 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isuserdata(L,2))
    {
        luaL_error(L,"argument 2 must be userdata");
        return 0;
    }
    if(!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be integer");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    int program=lua_tointeger(L,3);
    glUseProgram(program);
    if(n>=4)
    {
        if(!lua_istable(L,4))
        {
            luaL_error(L,"argument 4 must be table");
            return 0;
        }
        lua_pushnil(L);
        int un=1;
        int texcount=0;
        while (lua_next(L,4)!=0)
        {
            if(lua_isstring(L,-2))
            {
                int loc=glGetUniformLocation(program,lua_tostring(L,-2));
                if(loc>=0)
                {
                    int type;
                    glGetActiveUniform(program,loc,0,0,0,&type,0);
                    if(type==GL_FLOAT||type==GL_INT||type==GL_UNSIGNED_INT||type==GL_DOUBLE||type==GL_SAMPLER_2D)
                    {
                        if(!lua_isnumber(L,-1))
                        {
                            luaL_error(L,"uniform %d must contain number",un);
                            return 0;
                        }
                    }
                    else if(GL_FLOAT_VEC2<=type<=GL_INT_VEC4||GL_BOOL_VEC2<=type<=GL_FLOAT_MAT4||GL_FLOAT_MAT2x3<=type<=GL_FLOAT_MAT4x3||
                        GL_DOUBLE_MAT2<=type<=GL_DOUBLE_MAT4x3)
                    {
                        if(!lua_istable(L,-1))
                        {
                            luaL_error(L,"uniform %d must contain tables",un);
                            return 0;
                        }
                    }
                    float mat[16];
                    if(type==GL_SAMPLER_2D)
                    {
                        glActiveTexture(GL_TEXTURE0+texcount);
                        int tex=lua_tointeger(L,-1);
                        glBindTexture(GL_TEXTURE_2D,tex);
                        glUniform1i(loc,texcount);
                        texcount++;
                    }
                    else if(type==GL_FLOAT)
                        glUniform1f(loc,lua_tonumber(L,-1));
                    else if(type==GL_DOUBLE)
                        glUniform1d(loc,lua_tonumber(L,-1));
                    else if(type==GL_INT)
                        glUniform1i(loc,lua_tointeger(L,-1));
                    else if(type==GL_UNSIGNED_INT)
                        glUniform1ui(loc,lua_tointeger(L,-1));
                    else if(type==GL_FLOAT_VEC2)
                        glUniform2f(loc,getd(L,1),getd(L,2));
                    else if(type==GL_FLOAT_VEC3)
                        glUniform3f(loc,getd(L,1),getd(L,2),getd(L,3));
                    else if(type==GL_FLOAT_VEC4)
                        glUniform4f(loc,getd(L,1),getd(L,2),getd(L,3),getd(L,4));
                    else if(type==GL_DOUBLE_VEC2)
                        glUniform2d(loc,getd(L,1),getd(L,2));
                    else if(type==GL_DOUBLE_VEC3)
                        glUniform3d(loc,getd(L,1),getd(L,2),getd(L,3));
                    else if(type==GL_DOUBLE_VEC4)
                        glUniform4d(loc,getd(L,1),getd(L,2),getd(L,3),getd(L,4));
                    else if(type==GL_INT_VEC2)
                        glUniform2i(loc,geti(L,1),geti(L,2));
                    else if(type==GL_INT_VEC3)
                        glUniform3i(loc,geti(L,1),geti(L,2),geti(L,3));
                    else if(type==GL_INT_VEC4)
                        glUniform4i(loc,geti(L,1),geti(L,2),geti(L,3),geti(L,4));
                    else if(type==GL_UNSIGNED_INT_VEC2)
                        glUniform2ui(loc,geti(L,1),geti(L,2));
                    else if(type==GL_UNSIGNED_INT_VEC3)
                        glUniform3ui(loc,geti(L,1),geti(L,2),geti(L,3));
                    else if(type==GL_UNSIGNED_INT_VEC4)
                        glUniform4ui(loc,geti(L,1),geti(L,2),geti(L,3),geti(L,4));
                    if(type==GL_FLOAT_MAT2||type==GL_FLOAT_MAT3||type==GL_FLOAT_MAT4)
                        for(int i=0;i<4;i++)
                            mat[i]=getd(L,i+1);
                    if(type==GL_FLOAT_MAT3||type==GL_FLOAT_MAT4)
                        for(int i=4;i<9;i++)
                            mat[i]=getd(L,i+1);
                    if(type==GL_FLOAT_MAT4)
                        for(int i=9;i<16;i++)
                            mat[i]=getd(L,i+1);
                    if(type==GL_FLOAT_MAT2)
                        glUniformMatrix2fv(loc,1,1,mat);
                    else if(type==GL_FLOAT_MAT3)
                        glUniformMatrix3fv(loc,1,1,mat);
                    else if(type==GL_FLOAT_MAT4)
                        glUniformMatrix4fv(loc,1,1,mat);
                }
            }
            lua_pop(L, 1);
            un++;
        }
    }
    //exit(0);
    sVAO* vao=lua_touserdata(L,2);
    glBindVertexArray(vao->vaobuf);
    glDrawArrays(vao->type,0,vao->vertices);
    glUseProgram(0);
    glBindVertexArray(0);
    return 0;
}

int lua_clearcolor(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<5)
    {
        luaL_error(L,"error arguments count (expected 5 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be number");
        return 0;
    }
    if(!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be number");
        return 0;
    }
    if(!lua_isnumber(L,4))
    {
        luaL_error(L,"argument 4 must be number");
        return 0;
    }
    if(!lua_isnumber(L,5))
    {
        luaL_error(L,"argument 5 must be number");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    glClearColor(lua_tonumber(L,2),lua_tonumber(L,3),lua_tonumber(L,4),lua_tonumber(L,5));
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}

int lua_cleardepth(lua_State* L)
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
    glfwMakeContextCurrent(lua_touserdata(L,1));
    glClear(GL_DEPTH_BUFFER_BIT);
    return 0;
}

int lua_timedelta(lua_State* L)
{
    lua_pushnumber(L,glfwGetTime());
    glfwSetTime(0);
    return 1;
}

int lua_windowsize(lua_State* L)
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
    int width,height;
    glfwMakeContextCurrent(lua_touserdata(L,1));
    glfwGetFramebufferSize(lua_touserdata(L,1),&width,&height);
    lua_pushnumber(L,width);
    lua_pushnumber(L,height);
    return 2;
}

int lua_viewport(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<5)
    {
        luaL_error(L,"error arguments count (expected 5 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be integer");
        return 0;
    }
    if(!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be integer");
        return 0;
    }
    if(!lua_isnumber(L,4))
    {
        luaL_error(L,"argument 4 must be integer");
        return 0;
    }
    if(!lua_isnumber(L,5))
    {
        luaL_error(L,"argument 5 must be integer");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    glViewport(lua_tointeger(L,2),lua_tointeger(L,3),lua_tointeger(L,4),lua_tointeger(L,5));
    return 0;
}

int error_callback;

int lua_errorcallback(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<5)
    {
        luaL_error(L,"error arguments count (expected 1 argument)");
        return 0;
    }
    if(!lua_isfunction(L,1))
    {
        luaL_error(L,"argument 1 must be function");
        return 0;
    }
    if(error_callback!=-1)
        luaL_unref(L,LUA_REGISTRYINDEX,error_callback);
    error_callback=luaL_ref(L,LUA_REGISTRYINDEX);
    return 0;
}

void errcb(int code,const char* str)
{
    printf("glfw error : ( %i ) %s\n",code,str);
}

typedef struct Image
{
    unsigned char* data;
    int width,height,nrChannels;
}Image;

int lua_loadimage(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<1)
    {
        luaL_error(L,"error arguments count (expected 1 argument)");
        return 0;
    }
    if(!lua_isstring(L,1))
    {
        luaL_error(L,"argument 1 must be string");
        return 0;
    }
    int width,height,nrChannels;
    const char* name=lua_tostring(L,1);
    unsigned char* data=stbi_load(name,&width,&height,&nrChannels,STBI_rgb_alpha);
    if(!data)
    {
        luaL_error(L,"failed to load texture \"%s\"\n",name);
        return 0;
    }
    Image* img=malloc(sizeof(Image));
    img->data=data;
    img->width=width;
    img->height=height;
    img->nrChannels=nrChannels;
    lua_pushlightuserdata(L,img);
    return 1;
}

int lua_deleteimage(lua_State* L)
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
    Image* img=lua_touserdata(L,1);
    stbi_image_free(img->data);
    free(img);
    return 0;
}

int lua_loadtexture(lua_State* L)
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
    if(!lua_isuserdata(L,2))
    {
        luaL_error(L,"argument 2 must be userdata");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    Image* img=lua_touserdata(L,2);
    int texture;
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img->width,img->height,0,GL_RGBA,GL_UNSIGNED_BYTE,img->data);
    glGenerateMipmap(GL_TEXTURE_2D);
    lua_pushnumber(L,texture);
    return 1;
}

int lua_createtexture(lua_State* L)
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
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be number");
        return 0;
    }
    if(!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be number");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    int texture;
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,lua_tointeger(L,2),lua_tointeger(L,3),0,GL_RGBA,GL_UNSIGNED_BYTE,0);
    //glGenerateMipmap(GL_TEXTURE_2D);
    lua_pushnumber(L,texture);
    return 1;
}

int lua_textureclear(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<6)
    {
        luaL_error(L,"error arguments count (expected 6 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be number");
        return 0;
    }
    if(!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be number");
        return 0;
    }
    if(!lua_isnumber(L,4))
    {
        luaL_error(L,"argument 4 must be number");
        return 0;
    }
    if(!lua_isnumber(L,5))
    {
        luaL_error(L,"argument 5 must be number");
        return 0;
    }
    if(!lua_isnumber(L,6))
    {
        luaL_error(L,"argument 6 must be number");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    int texture=lua_tointeger(L,2);
    glBindTexture(GL_TEXTURE_2D,texture);
    float color[4];
    color[0]=lua_tonumber(L,3);
    color[1]=lua_tonumber(L,4);
    color[2]=lua_tonumber(L,5);
    color[3]=lua_tonumber(L,6);
    unsigned int red[] = {255,0,0,255};
    glClearTexImage(texture,0,GL_RGBA,GL_UNSIGNED_BYTE,red);
    return 0;
}

int lua_createdepthtexture(lua_State* L)
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
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be number");
        return 0;
    }
    if(!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be number");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    int texture;
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,lua_tointeger(L,2),lua_tointeger(L,3),0,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT,0);
    //glGenerateMipmap(GL_TEXTURE_2D);
    lua_pushnumber(L,texture);
    return 1;
}

int lua_createstenciltexture(lua_State* L)
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
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be number");
        return 0;
    }
    if(!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be number");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    int texture;
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_STENCIL_INDEX8,lua_tointeger(L,2),lua_tointeger(L,3),0,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,0);
    //glGenerateMipmap(GL_TEXTURE_2D);
    lua_pushnumber(L,texture);
    return 1;
}

int lua_deletetexture(lua_State* L)
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
    int tex=lua_tointeger(L,1);
    glDeleteTextures(1,&tex);
    return 0;
}

int lua_createframebuffer(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<4)
    {
        luaL_error(L,"error arguments count (expected 3 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be number");
        return 0;
    }
    if(!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be number");
        return 0;
    }
    if(!lua_isnumber(L,4))
    {
        luaL_error(L,"argument 4 must be number");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    unsigned int fbo;
    unsigned int texture=lua_tointeger(L,2);
    unsigned int texture2=lua_tointeger(L,3);
    unsigned int texture3=lua_tointeger(L,4);
    glGenFramebuffers(1,&fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    //glBindTexture(GL_TEXTURE_2D,texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,texture2,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_STENCIL_ATTACHMENT,GL_TEXTURE_2D,texture3,0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1,&fbo);
        luaL_error(L,"framebuffer is not complete");
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    lua_pushnumber(L,fbo);
    return 1;
}

int lua_deleteframebuffer(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<2)
    {
        luaL_error(L,"error arguments count (expected 3 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be number");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    unsigned int fbo=lua_tointeger(L,2);
    glDeleteFramebuffers(1,&fbo);
    return 0;
}

int lua_bindframebuffer(lua_State* L)
{
    int n=lua_gettop(L);
    if(n<1)
    {
        luaL_error(L,"error arguments count (expected 2 arguments)");
        return 0;
    }
    if(!lua_isuserdata(L,1))
    {
        luaL_error(L,"argument 1 must be userdata");
        return 0;
    }
    glfwMakeContextCurrent(lua_touserdata(L,1));
    if(n>=2)
    {
        if(!lua_isnumber(L,2))
        {
            luaL_error(L,"argument 2 must be integer");
            return 0;
        }
        else
            glBindFramebuffer(GL_FRAMEBUFFER,lua_tointeger(L,2));
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    }
    return 0;
}

int lua_mousegetpos(lua_State* L)
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
    double xpos,ypos;
    glfwGetCursorPos(lua_touserdata(L,1),&xpos,&ypos);
    lua_pushnumber(L,xpos);
    lua_pushnumber(L,ypos);
    return 2;
}

int lua_mousesetpos(lua_State* L)
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
    if(!lua_isnumber(L,2))
    {
        luaL_error(L,"argument 2 must be number");
        return 0;
    }
    if(!lua_isnumber(L,3))
    {
        luaL_error(L,"argument 3 must be number");
        return 0;
    }
    glfwSetCursorPos(lua_touserdata(L,1),lua_tonumber(L,2),lua_tonumber(L,3));
    return 0;
}

int lua_mouseenable(lua_State* L)
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
    if(!lua_isboolean(L,2))
    {
        luaL_error(L,"argument 2 must be boolean");
        return 0;
    }
    GLFWwindow* win=lua_touserdata(L,1);
    glfwMakeContextCurrent(win);
    if(lua_toboolean(L,2)==0)
    {
        glfwSetInputMode(win,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(win,GLFW_RAW_MOUSE_MOTION,GLFW_TRUE);
    }
    else
    {
        glfwSetInputMode(win,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
    }
    return 0;
}

int lua_keycallback(lua_State* L)
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
    GLFWwindow* win=lua_touserdata(L,1);
    glfwMakeContextCurrent(win);
    KeyData* current;
    if(!lua_isfunction(L,2))
    {
        if(!lua_isnil(L,2))
        {
            printf("keycb nil\n");
            glfwSetKeyCallback(win,0);
            current=keydataroot;
            while(current!=0)
            {
                if(current->win==win&&current->active)
                {
                    current->active=0;
                    luaL_unref(L,LUA_REGISTRYINDEX,current->func);
                    break;
                }
                current=current->next;
            }
            return 0;
        }
        luaL_error(L,"argument 2 must be function");
        return 0;
    }
    if(!lua_isfunction(L,3))
    {
        luaL_error(L,"argument 3 must be function");
        return 0;
    }
    //glfwSetKeyCallback(win,0);
    current=keydataroot;
    while(current!=0)
    {
        if(current->win==win)
        {
            if(current->active)
                luaL_unref(L,LUA_REGISTRYINDEX,current->func);
            else
                current->active=1;
            current->errfunc=luaL_ref(L,LUA_REGISTRYINDEX);
            current->func=luaL_ref(L,LUA_REGISTRYINDEX);
            break;
        }
        current=current->next;
    }
    glfwSetKeyCallback(win,KeyCB);
    return 0;
}


int lua_newtask(lua_State* L);
int lua_closetask(lua_State* L);

void KeyCB(GLFWwindow* win, int key, int scancode, int action, int mods)
{
    glfwMakeContextCurrent(win);
    KeyData* current=keydataroot;
    while(current!=0)
    {
        if(current->win==win)
        {
            lua_State* thread=lua_newthread(L);
            lua_pop(L,1);
            lua_rawgeti(thread,LUA_REGISTRYINDEX,current->func);
            lua_rawgeti(thread,LUA_REGISTRYINDEX,current->errfunc);
            lua_pushlightuserdata(thread,win);
            lua_pushnumber(thread,key);
            lua_pushnumber(thread,scancode);
            lua_pushnumber(thread,action);
            lua_pushnumber(thread,mods);
            lua_newtask(thread);
            break;
        }
        current=current->next;
    }
}

int graphicsInit(lua_State* argL)
{
    L=argL;
    stbi_set_flip_vertically_on_load(1);
    error_callback=-1;
    if(!glfwInit())
    {
        printf("glfwInit failed");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
    //glfwSetErrorCallback(&errcb);
    keydataroot=0;

    lua_pushcfunction(L,&lua_createwindow);
    lua_setglobal(L,"createwindow");
    lua_pushcfunction(L,&lua_windowshouldclose);
    lua_setglobal(L,"windowshouldclose");
    lua_pushcfunction(L,&lua_destroywindow);
    lua_setglobal(L,"destroywindow");
    lua_pushcfunction(L,&lua_swapbuffers);
    lua_setglobal(L,"swapbuffers");
    lua_pushcfunction(L,&lua_pollevents);
    lua_setglobal(L,"pollevents");
    lua_pushcfunction(L,&lua_vertexshader);
    lua_setglobal(L,"vertexshader");
    lua_pushcfunction(L,&lua_fragmentshader);
    lua_setglobal(L,"fragmentshader");
    lua_pushcfunction(L,&lua_deletesubshader);
    lua_setglobal(L,"deletesubshader");
    lua_pushcfunction(L,&lua_compileshader);
    lua_setglobal(L,"compileshader");
    lua_pushcfunction(L,&lua_deleteshader);
    lua_setglobal(L,"deleteshader");
    lua_pushcfunction(L,&lua_attributeexists);
    lua_setglobal(L,"attributeexists");
    lua_pushcfunction(L,&lua_createvertexdata);
    lua_setglobal(L,"createvertexdata");
    lua_pushcfunction(L,&lua_addvertexdata);
    lua_setglobal(L,"addvertexdata");
    lua_pushcfunction(L,&lua_deletevertexdata);
    lua_setglobal(L,"deletevertexdata");
    lua_pushcfunction(L,&lua_createvbo);
    lua_setglobal(L,"createvbo");
    lua_pushcfunction(L,&lua_deletevbo);
    lua_setglobal(L,"deletevbo");
    lua_pushcfunction(L,&lua_createvao);
    lua_setglobal(L,"createvao");
    lua_pushcfunction(L,&lua_deletevao);
    lua_setglobal(L,"deletevao");
    lua_pushcfunction(L,&lua_drawmesh);
    lua_setglobal(L,"drawmesh");
    lua_pushcfunction(L,&lua_clearcolor);
    lua_setglobal(L,"clearcolor");
    lua_pushcfunction(L,&lua_cleardepth);
    lua_setglobal(L,"cleardepth");
    lua_pushcfunction(L,&lua_timedelta);
    lua_setglobal(L,"timedelta");
    lua_pushcfunction(L,&lua_windowsize);
    lua_setglobal(L,"windowsize");
    lua_pushcfunction(L,&lua_viewport);
    lua_setglobal(L,"viewport");
    lua_pushcfunction(L,&lua_loadimage);
    lua_setglobal(L,"loadimage");
    lua_pushcfunction(L,&lua_deleteimage);
    lua_setglobal(L,"deleteimage");
    lua_pushcfunction(L,&lua_loadtexture);
    lua_setglobal(L,"loadtexture");
    lua_pushcfunction(L,&lua_createtexture);
    lua_setglobal(L,"createtexture");
    lua_pushcfunction(L,&lua_textureclear);
    lua_setglobal(L,"textureclear");
    lua_pushcfunction(L,&lua_createdepthtexture);
    lua_setglobal(L,"createdepthtexture");
    lua_pushcfunction(L,&lua_createstenciltexture);
    lua_setglobal(L,"createstenciltexture");
    lua_pushcfunction(L,&lua_deletetexture);
    lua_setglobal(L,"deletetexture");
    lua_pushcfunction(L,&lua_createframebuffer);
    lua_setglobal(L,"createframebuffer");
    lua_pushcfunction(L,&lua_deleteframebuffer);
    lua_setglobal(L,"deleteframebuffer");
    lua_pushcfunction(L,&lua_bindframebuffer);
    lua_setglobal(L,"bindframebuffer");
    lua_pushcfunction(L,&lua_keycallback);
    lua_setglobal(L,"keycallback");
    lua_pushcfunction(L,&lua_mouseenable);
    lua_setglobal(L,"mouseenable");
    lua_pushcfunction(L,&lua_mousegetpos);
    lua_setglobal(L,"mousegetpos");
    lua_pushcfunction(L,&lua_mousesetpos);
    lua_setglobal(L,"mousesetpos");
    lua_pushcfunction(L,&lua_depthtest);
    lua_setglobal(L,"depthtest");
    lua_pushcfunction(L,&lua_cullface);
    lua_setglobal(L,"cullface");
    return 0;
}

/*void graphicsFinal()
{
    if(keydataroot!=0)
    {
        KeyData* current=keydataroot;
        while(current!=0)
        {
            if(current->active)
                luaL_unref(L,LUA_REGISTRYINDEX,current->func);
            KeyData* prev=current;
            current=current->next;
            free(current);
        }
    }
}*/