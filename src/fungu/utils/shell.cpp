
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>

#include <fungu/script.hpp>
#include <fungu/script/lua/object_wrapper.hpp>
#include <fungu/script/lua/lua_function.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/bind.hpp>

using namespace fungu;
static script::env runtime;

void print_script_error(const script::error_info * errinfo)
{
    const script::source_context * source = errinfo->get_root_info()->get_source_context();
    script::error error = errinfo->get_error();
    error.get_error_message();
    std::cerr<<"Error";
    if(source) std::cerr<<" "<<source->get_location()<<":"<<source->get_line_number();
    std::cerr<<": "<<error.get_error_message();
    std::cerr<<std::endl;
}

void evaluator(script::expression * expr,script::env::frame * frame)
{
    if(!expr->is_empty_expression())
    {
        const_string result = expr->eval(frame).to_string();
        if(result.length()) std::cout<<result<<std::endl;
    }
}

void echo(const char * text)
{
    std::cout<<text<<std::endl;
}

int return_int(int x){return x;}
unsigned int return_uint(unsigned int x){return x;}

void register_object_with_lua(lua_State * L,const_string id, script::env::object * obj)
{
    //script::lua::register_object(L,LUA_GLOBALINDEX,obj,id.copy().c_str());
}

int server_newindex(lua_State * L)
{
    bool is_func = (lua_type(L,-1) == LUA_TFUNCTION);
    std::string key = lua_tostring(L, -2);
    lua_rawset(L,-3);
    if(is_func)
        runtime.get_global_scope()->bind_global_object(new script::lua::lua_function(runtime,-1,key.c_str()),const_string(key));
    return 0;
}

int server_index(lua_State * L)
{
    const char * key = lua_tostring(L, -1);
    
    return 0;
}

int main(int argc,char ** argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("expr", po::value<std::string>(), "evaluate expression")
        ("script",po::value<std::string>(), "execute script file");
    
    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc,argv, desc), vm);
        po::notify(vm);
    }
    catch(std::exception & e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    
    if(vm.count("help"))
    {
        std::cout<<desc<<std::endl;
        return 1;
    }
    
    
    script::load_corelib(runtime);
    
    script::function<void (const char *)> echo_func(echo);
    runtime.bind_global_object(&echo_func,FUNGU_OBJECT_ID("echo"));
    
    script::function<int (int)> return_int_func(return_int);
    runtime.bind_global_object(&return_int_func,FUNGU_OBJECT_ID("int"));
    
    script::function<unsigned int (unsigned int)> return_uint_func(return_uint);
    runtime.bind_global_object(&return_uint_func,FUNGU_OBJECT_ID("uint"));
    
    lua_State * L = luaL_newstate();
    luaL_openlibs(L);
    
    runtime.set_lua_state(L);
    //runtime.set_bind_observer(boost::bind(register_object_with_lua,L,_1,_2));
    
    lua_pushlightuserdata(L, runtime.get_global_scope());
    lua_setfield(L,LUA_REGISTRYINDEX, "fungu_script_global_frame");
    
    lua_newtable(L); // Server table
    lua_newtable(L); // Server metatable
    lua_pushcclosure(L, server_newindex, 0);
    lua_setfield(L, -2, "__newindex");
    //lua_pushcclosure(L, server_index, 0);
    //lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
    lua_setglobal(L, "server");
    
    try
    {
        if(vm.count("expr"))
        {
            script::execute_text(vm["expr"].as<std::string>(),runtime.get_global_scope());
            return 0;
        }
        
        if(vm.count("script"))
        {
            script::throw_if_error(
                script::execute_file(vm["script"].as<std::string>().c_str(),runtime.get_global_scope()));
            return 0;
        }
    }
    catch(script::error_info * error_info)
    {
        print_script_error(error_info);
        delete error_info;
        return 1;
    }
    
    const char * line = NULL;
    
    script::eval_stream reader(runtime.get_global_scope(),&evaluator);
    
    script::file_source_context source("stdin");
    source.set_line_number(1);
    runtime.set_source_context(&source);
    
    do
    {
        line = readline(!reader.is_parsing_expression() ? "fungu> " : ">> ");
        if(line)
        {
            if(line[0]!='\0') add_history(line);
            
            try
            {
                reader.feed(line,strlen(line));
                reader.feed("\n",1);
            }
            catch(script::error_info * errinfo)
            {
                print_script_error(errinfo);
                delete errinfo;
            }
        }
    }while(line && !runtime.get_global_scope()->has_expired());
    
    std::cout<<"\nBye."<<std::endl;
    
    return 0;
}
