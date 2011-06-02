#include "../crypto.hpp"
#include "../md5.h"

#include <string>
#include <stdio.h>
extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

static FILE * urandom = NULL;

namespace ecc{

static void make_seed(unsigned int * output, std::size_t n)
{
    if(!urandom || fread(output, sizeof(unsigned int), n, urandom) < n)
        loopi(n) output[i] = randomMT();
}

class key;

class challenge
{
    friend class key;
    static const char * MT_NAME;
public:
    ~challenge()
    {
        freechallenge(m_answer);
    }
    
    const char * get_challenge()const
    {
        return m_challenge.getbuf();
    }
    
    bool expected_answer(const char * foreign)const
    {
        return checkchallenge(foreign, m_answer);
    }
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT_NAME);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &challenge::__gc},
            {"expected_answer", &challenge::expected_answer},
            {"to_string", &challenge::to_string},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<challenge *>(luaL_checkudata(L, 1, MT_NAME))->~challenge();
        return 0;
    }
    
    static int expected_answer(lua_State * L)
    {
        challenge * self = reinterpret_cast<challenge *>(luaL_checkudata(L, 1, MT_NAME));
        const char * answer = luaL_checkstring(L, 2);
        lua_pushboolean(L, self->expected_answer(answer));
        return 1;
    }
    
    static int to_string(lua_State * L)
    {
        challenge * self = reinterpret_cast<challenge *>(luaL_checkudata(L, 1, MT_NAME));
        lua_pushstring(L, self->get_challenge());
        return 1;
    }
    
    challenge(){}
    challenge(const challenge &){}
    
    vector<char> m_challenge;
    void * m_answer;
};

const char * challenge::MT_NAME = "crypto_ecc_challenge";

class key
{
public:
    static const char * MT_NAME;

    key(const char * stringform)
     :m_printed(stringform)
    {
        m_key = parsepubkey(stringform);
    }
    
    ~key()
    {
        if(m_key) freepubkey(m_key);
    }
        
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT_NAME);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &key::__gc},
            {"generate_challenge", &key::generate_challenge},
            {"to_string", &key::to_string},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<key *>(luaL_checkudata(L, 1, MT_NAME))->~key();
        return 0;
    }
    
    static int generate_challenge(lua_State * L)
    {
        key * self = reinterpret_cast<key *>(luaL_checkudata(L, 1, MT_NAME));
        
        challenge * chal = new (lua_newuserdata(L, sizeof(challenge))) challenge;
        luaL_getmetatable(L, challenge::MT_NAME);
        lua_setmetatable(L, -2);
        
        unsigned int seed[4];
        make_seed(seed, 4);
        
        chal->m_answer = genchallenge(self->m_key, seed, sizeof(seed), chal->m_challenge);
        return 1;
    }
    
    static int to_string(lua_State * L)
    {
        key * self = reinterpret_cast<key *>(luaL_checkudata(L, 1, MT_NAME));
        lua_pushstring(L, self->m_printed.c_str());
        return 1;
    }
    
    key(const key &){}
    void * m_key;
    std::string m_printed;
};

const char * key::MT_NAME = "crypto_ecc_key";

static int generate_key_pair(lua_State * L)
{
    vector<char> privkeyout, pubkeyout;
    
    unsigned int seed[4];
    make_seed(seed, 4);
    
    genprivkey(seed, sizeof(seed), privkeyout, pubkeyout);
    
    lua_pushstring(L, privkeyout.getbuf());
    new (lua_newuserdata(L, sizeof(key))) key(pubkeyout.getbuf());
    
    luaL_getmetatable(L, key::MT_NAME);
    lua_setmetatable(L, -2);
    
    return 2;
}

static int create_key(lua_State * L)
{
    const char * stringform = luaL_checkstring(L, 1);
    new (lua_newuserdata(L, sizeof(key))) key(stringform);
    luaL_getmetatable(L, key::MT_NAME);
    lua_setmetatable(L, -2);
    return 1;
}

static int answer_challenge(lua_State * L)
{
    const char * private_key = luaL_checkstring(L, 1);
    const char * challenge = luaL_checkstring(L, 2);
    
    vector<char> answer;
    answerchallenge(private_key, challenge, answer);
    
    lua_pushstring(L, answer.getbuf());
    
    return 1;
}

} //namespace ecc

static int md5sum(lua_State * L)
{
    size_t inputlen = 0;
    const char * input = luaL_checklstring(L, 1, &inputlen);
    
    MD5_CTX context;
    MD5Init(&context);
    MD5Update(&context, (unsigned char *)input, inputlen);
    MD5Final(&context);
        
    const size_t outputlen = sizeof(context.digest)*2;
    char output[outputlen + 1];
    
    assert(sizeof(context.digest[0]) == 1);
    
    static const char * digitset = "0123456789abcdef";
    for(unsigned int i = 0, j = 0; i < sizeof(context.digest); i++, j+=2)
    {
        output[j] = digitset[context.digest[i] / 16];
        output[j + 1] = digitset[context.digest[i] % 16];
    }
    
    output[outputlen] = '\0';
    
    lua_pushlstring(L, output, outputlen);
    return 1;
}

static int tigersum(lua_State * L)
{
    const char * input = luaL_checkstring(L, 1);
    string output;
    hashstring(input, output, sizeof(output));
    lua_pushstring(L, output);
    return 1;
}

namespace lua{
namespace module{

void open_crypto(lua_State * L)
{
    urandom = fopen("/dev/urandom","r");
    if(!urandom)
    {
        fprintf(stderr, "Crypto module warning: couldn't open /dev/urandom for reading -- will be using randomMT() instead!");
    }
    else
    {
        seedMT(time(NULL));
    }
    
    static luaL_Reg functions[] = {
        {"md5sum", md5sum},
        {"tigersum", tigersum},
        {NULL, NULL}
    };
    
    luaL_register(L, "crypto", functions);
    
    static luaL_Reg ecc_functions[] = {
        {"generate_key_pair", ecc::generate_key_pair},
        {"key", ecc::create_key},
        {"answer_challenge", ecc::answer_challenge},
        {NULL, NULL}
    };
    
    lua_newtable(L);
    luaL_register(L, NULL, ecc_functions);
    
    lua_setfield(L, -2, "sauerecc");
    
    ecc::key::register_class(L);
    ecc::challenge::register_class(L);
}

} //namespace module
} //namespace lua
