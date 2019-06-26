extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
}
#include <cstring>
#include <string> // Find out why placement new depends on a stdlib header

class directory_iterator
{
public:
    static const char * MT;

    directory_iterator(const char * dirname)
    {
        m_dir = opendir(dirname);
        
        m_dir_path = new char[strlen(dirname) + 1];
        strcpy(m_dir_path, dirname);
    }
    
    ~directory_iterator()
    {
        if(m_dir) closedir(m_dir);
        delete [] m_dir_path;
    }
    
    static void create_metatable(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &directory_iterator::__gc},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
    }
    
    static int next(lua_State * L)
    {
        directory_iterator * self = reinterpret_cast<directory_iterator *>(luaL_checkudata(L, 1, MT));
        if(!self->m_dir) return 0;
        struct dirent * entry = readdir(self->m_dir);
        if(!entry) return 0;
        
	    std::string filename;
	    filename  = self->m_dir_path;
	    filename += "/";
	    filename += entry->d_name; 
	    
        struct stat info;
        if (stat(filename.c_str(), &info)) return 0;
        
        unsigned char file_type = DT_UNKNOWN;
        if (S_ISREG(info.st_mode))       file_type = DT_REG;
        else if (S_ISDIR(info.st_mode))  file_type = DT_DIR;
        else if (S_ISFIFO(info.st_mode)) file_type = DT_FIFO;
        else if (S_ISLNK(info.st_mode))  file_type = DT_LNK;
        else if (S_ISBLK(info.st_mode))  file_type = DT_BLK;
        else if (S_ISCHR(info.st_mode))  file_type = DT_CHR;
        else if (S_ISSOCK(info.st_mode)) file_type = DT_SOCK;
	
        lua_pushinteger(L, file_type);
        lua_pushstring(L, entry->d_name);
        
        return 2;
    }
    
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<directory_iterator *>(luaL_checkudata(L, 1, MT))->~directory_iterator();
        return 0;
    }
    
    DIR * m_dir;
    char * m_dir_path;
};

const char * directory_iterator::MT = "directory_iterator";

static int begin_dir_iterator(lua_State * L)
{
    const char * dirname = luaL_checkstring(L, 1);
    lua_pushcfunction(L, &directory_iterator::next);
    
    new(lua_newuserdata(L, sizeof(directory_iterator)))directory_iterator(dirname);
    luaL_getmetatable(L, directory_iterator::MT);
    lua_setmetatable(L, -2);
    
    lua_pushnil(L);
    
    return 3;
}

namespace lua{
namespace module{

void open_filesystem(lua_State * L)
{    
    static luaL_Reg functions[] = {
        {"dir", begin_dir_iterator},
        {NULL, NULL}
    };
    
    luaL_register(L, "filesystem", functions);
    
    lua_pushinteger(L, DT_FIFO);
    lua_setfield(L, -2, "FIFO");
    
    lua_pushinteger(L, DT_DIR);
    lua_setfield(L, -2, "DIRECTORY");
    
    lua_pushinteger(L, DT_REG);
    lua_setfield(L, -2, "FILE");
    
    lua_pushinteger(L, DT_SOCK);
    lua_setfield(L, -2, "SOCKET");
        
    directory_iterator::create_metatable(L);
}

} //namespace module
} //namespace lua
