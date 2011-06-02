#include <fungu/script.hpp>
#include <fungu/script/any_variable.hpp>
using namespace fungu;

extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

fungu::script::env & get_script_env();
void report_script_error(const char *);

const char * ENV_MT = "cubescript_env_class";
const char * PARSER_MT = "cubescript_parser_class";
const char * EXPRESSION_MT = "cubescript_expr_class";

static int create_env(lua_State * L)
{
    script::env * env = new (lua_newuserdata(L, sizeof(script::env))) script::env;
    luaL_getmetatable(L, ENV_MT);
    lua_setmetatable(L, -2);
    env->set_lua_state(L);
    return 1;
}

static int __gc_env(lua_State * L)
{
    reinterpret_cast<script::env *>(luaL_checkudata(L, 1, ENV_MT))->~env();
    return 0;
}

static int env_bind(lua_State * L)
{
    script::env * env = reinterpret_cast<script::env *>(luaL_checkudata(L, 1, ENV_MT));
    const char * symbol = luaL_checkstring(L, 2);
    script::env_object * obj = env->lookup_global_object(symbol);
    
    if(!obj)
    {
        script::any_variable * newobj = new script::any_variable;
        newobj->set_adopted();
        env->bind_global_object(newobj, const_string(symbol));
        obj = newobj;
    }

    try
    {
        obj->assign(script::lua::get_argument_value(L));
    }
    catch(script::error_trace * errinfo)
    {
        return luaL_error(L, errinfo->get_root_info()->get_error().get_error_message().c_str());
    }
    catch(script::error err)
    {
        return luaL_error(L, err.get_error_message().c_str());
    }
    
    return 0;
}

static int env_load_corelib(lua_State * L)
{
    script::env * env = reinterpret_cast<script::env *>(luaL_checkudata(L, 1, ENV_MT));
    script::load_corelib(*env);
    return 0;
}

static void create_env_metatable(lua_State * L)
{
    luaL_newmetatable(L, ENV_MT);
    
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    
    static luaL_Reg funcs[] = {
        {"__gc", __gc_env},
        {"bind", env_bind},
        {"load_corelib", env_load_corelib},
        {NULL, NULL}
    };
    
    luaL_register(L, NULL, funcs);
}

static int eval_file(lua_State * L)
{
    const char * filename = luaL_checkstring(L, 1);
    script::env * env = &get_script_env();
    if(lua_gettop(L) >= 2)
        env = reinterpret_cast<script::env *>(luaL_checkudata(L, 2, ENV_MT));
    try
    {
        script::execute_file(filename, *env);
    }
    catch(script::error_trace * errinfo)
    {
        return luaL_error(L, errinfo->get_root_info()->get_error().get_error_message().c_str());
    }
    catch(script::error err)
    {
        return luaL_error(L, err.get_error_message().c_str());
    }
    return 0;
}

static int eval_string(lua_State * L)
{
    size_t codelen;
    const char * code = luaL_checklstring(L, 1, &codelen);
    script::env * env = &get_script_env();
    if(lua_gettop(L) >= 2)
        env = reinterpret_cast<script::env *>(luaL_checkudata(L, 2, ENV_MT));
    
    try
    {
        script::env_frame frame(env);
        script::execute_text(const_string(code, code + codelen - 1), &frame).push_value(L);
        return 1;
    }
    catch(script::error_trace * errinfo)
    {
        lua_pushnil(L);
        lua_pushstring(L, errinfo->get_root_info()->get_error().get_error_message().c_str());
        return 2;
        //return luaL_error(L, errinfo->get_root_info()->get_error().get_error_message().c_str());
    }
    catch(script::error err)
    {
        lua_pushnil(L);
        lua_pushstring(L, err.get_error_message().c_str());
        return 2;
        //return luaL_error(L, err.get_error_message().c_str());
    }
}

class expression_ptr
{
public:
    expression_ptr(script::expression * expr)
     :m_expr(expr)
    {
        
    }
    
    void set_null_pointer()
    {
        m_expr = NULL;
    }
    
    static expression_ptr * create(lua_State * L, script::expression * expr)
    {
        expression_ptr * obj = new (lua_newuserdata(L, sizeof(expression_ptr))) expression_ptr(expr);
        luaL_getmetatable(L, EXPRESSION_MT);
        lua_setmetatable(L, -2);
        return obj;
    }
    
    static void create_metatable(lua_State * L)
    {
        luaL_newmetatable(L, EXPRESSION_MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &expression_ptr::__gc},
            {"to_string", &expression_ptr::to_string},
            {"eval", &expression_ptr::eval},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<expression_ptr *>(luaL_checkudata(L, 1, EXPRESSION_MT))->~expression_ptr();
        return 0;
    }
    
    static int to_string(lua_State * L)
    {
        expression_ptr * self = reinterpret_cast<expression_ptr * >(luaL_checkudata(L, 1, EXPRESSION_MT));
        lua_pushstring(L, self->m_expr->form_source().c_str());
        return 1;
    }
    
    static int eval(lua_State * L)
    {
        expression_ptr * self = reinterpret_cast<expression_ptr * >(luaL_checkudata(L, 1, EXPRESSION_MT));
        if(!self->m_expr) return luaL_error(L, "object expired");
        int argc = lua_gettop(L);
        script::env * env = &get_script_env();
        if(argc>=2)
            env = reinterpret_cast<script::env *>(luaL_checkudata(L, 2, ENV_MT));
        script::env_frame frame(env);
        try
        {
            self->m_expr->eval(&frame).push_value(L);
        }
        catch(const script::error & error)
        {
            lua_pushnil(L);
            lua_pushstring(L, error.get_error_message().c_str());
            return 2;
        }
        catch(script::error_trace * error_info)
        {
            std::string error_message = error_info->get_root_info()->get_error().get_error_message();
            const script::source_context * source_info = error_info->get_root_info()->get_source_context();
            std::string source = (source_info ? source_info->get_location() : "");
            int line = (source_info ? source_info->get_line_number() : 0);
            lua_pushnil(L);
            lua_pushstring(L, error_message.c_str());
            if(source.length())
            {
                lua_pushstring(L, source.c_str());
                lua_pushinteger(L, line);
            }
            else
            {
                lua_pushnil(L);
                lua_pushnil(L);
            }
            return 4;
        }
        return 1;
    }
    
    script::expression * m_expr;
};

class parser
{
public:
    parser(script::env * env, lua_State * L, int functionIndex)
     :m_L(L),
      m_frame(env),
      m_parser(&m_frame, &parser::parsed_expression, this)
    {
        lua_pushvalue(L, functionIndex);
        m_function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    
    ~parser()
    {
        lua_unref(m_L, m_function_ref);
    }

    static void create_metatable(lua_State * L)
    {
        luaL_newmetatable(L, PARSER_MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &parser::__gc},
            {"parse", &parser::parse},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
    }
private:
    static void parsed_expression(script::expression * expr, script::env_frame * frame, void * parser_object)
    {
        parser * self = reinterpret_cast<parser *>(parser_object);
        
        lua_rawgeti(self->m_L, LUA_REGISTRYINDEX, self->m_function_ref);
        
        expression_ptr * expr_ptr = expression_ptr::create(self->m_L, expr);
        
        if(lua_pcall(self->m_L, 1, 0, 0) != 0)
            report_script_error(lua_tostring(self->m_L, -1));
        
        expr_ptr->set_null_pointer();
    }
    
    static int __gc(lua_State * L)
    {
        reinterpret_cast<parser *>(luaL_checkudata(L, 1, PARSER_MT))->~parser();
        return 0;
    }
    
    static int parse(lua_State * L)
    {
        parser * self = reinterpret_cast<parser *>(luaL_checkudata(L, 1, PARSER_MT));
        
        size_t codelen;
        const char * code = luaL_checklstring(L, 2, &codelen);
        
        bool completed = false;
        std::string error_message;
        std::string source;
        int line = -1;
        
        try
        {
            self->m_parser.feed(code, codelen);
            completed = !self->m_parser.is_parsing_expression();
        }
        catch(const script::error & error)
        {
            error_message = error.get_error_message();
        }
        catch(script::error_trace * error)
        {
            const script::error_trace * root_info = error->get_root_info();
            error_message = root_info->get_error().get_error_message();
            const script::source_context * source_info = root_info->get_source_context();
            std::string source = (source_info ? source_info->get_location() : "");
            int line = (source_info ? source_info->get_line_number() : 0);
            source = root_info->get_source_context()->get_location();
            line = root_info->get_source_context()->get_line_number();
        }
        
        lua_pushboolean(L, completed);

        lua_pushstring(L, error_message.c_str());
        
        if(source.length()) 
        {
            lua_pushstring(L, source.c_str());
            lua_pushinteger(L, line);
        }
        else
        {
            lua_pushnil(L);
            lua_pushnil(L);
        }
        
        return 4;
    }
    
    lua_State * m_L;
    int m_function_ref;
    script::env_frame m_frame;
    script::eval_stream m_parser;
};

static int create_parser(lua_State * L)
{
    int argc = lua_gettop(L);
    
    script::env * env = &get_script_env();
    if(argc >= 1 && lua_type(L, 1) == LUA_TUSERDATA)
        env = reinterpret_cast<script::env *>(luaL_checkudata(L, 1, ENV_MT));
    
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    new (lua_newuserdata(L, sizeof(parser))) parser(env, L, 2);
    luaL_getmetatable(L, PARSER_MT);
    lua_setmetatable(L, -2);
    return 1;
}

namespace lua{
namespace module{

void open_cubescript(lua_State * L)
{
    static luaL_Reg functions[] = {
        {"env", create_env},
        {"eval_file", eval_file},
        {"eval_string", eval_string},
        {"parser", create_parser},
        {NULL, NULL}
    };
    
    luaL_register(L, "cubescript", functions);
    
    create_env_metatable(L);
    parser::create_metatable(L);
    expression_ptr::create_metatable(L);
}

} //namespace module
} //namespace lua
