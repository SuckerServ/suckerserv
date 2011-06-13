#include "../register_class.hpp"
#include "../to.hpp"
#include "../create_object.hpp"
#include "../../crypto.hpp"
#include "../../lib/md5.h"

#ifndef WITHOUT_OPENSSL
#include <openssl/evp.h>
#include <openssl/hmac.h>
#endif

#include <string>
#include <stdio.h>
#include <lua.hpp>
#include "module.hpp"

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
        lua_pop(L, 1);
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
        lua_pop(L, 1);
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

static char * hex_encode(const unsigned char * input_begin, const unsigned char * input_end, 
    char * output_begin, 
    char * output_end)
{
    static const char * hex_digit = "0123456789abcdef";
    
    while(input_begin != input_end && output_begin + 1 < output_end)
    {
        *(output_begin++) = hex_digit[*input_begin / 16];
        *(output_begin++) = hex_digit[*input_begin % 16];
        
        input_begin++;
    }
    
    return output_begin;
}

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
    
    hex_encode(context.digest, context.digest + sizeof(context.digest), output, output + outputlen);
    
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

#ifndef WITHOUT_OPENSSL

class hmac
{
public:
    typedef HMAC_CTX target_type;
    static const char * CLASS_NAME;
    static int register_class(lua_State * L);
    static int create_object(lua_State * L);
private:
    static int __gc(lua_State * L);
    static int update(lua_State * L);
    static int digest(lua_State * L);
};

const char * hmac::CLASS_NAME = "hmac";

int hmac::__gc(lua_State * L)
{
    HMAC_CTX_cleanup(lua::to<hmac>(L, 1));
    return 0;
}

int hmac::register_class(lua_State * L){
    static luaL_Reg member_functions[] = {
        {"__gc", &hmac::__gc},
        {"update", &hmac::update},
        {"digest", &hmac::digest},
        {NULL, NULL}
    };
    lua::register_class(L, CLASS_NAME, member_functions);
    return 0;
}

int hmac::create_object(lua_State * L)
{
    const char * hash_function_name = luaL_checkstring(L, 1);
    
    size_t key_len;
    const char * key = luaL_checklstring(L, 2, &key_len);
    
    const EVP_MD * hash_function = EVP_get_digestbyname(hash_function_name);
    if(!hash_function)
    {
        luaL_error(L, "unknown hash function");
        return 0;
    }
    
    target_type * self = lua::create_object<hmac>(L);
    
    HMAC_CTX_init(self);
    HMAC_Init(self, key, static_cast<int>(key_len), hash_function);
    
    return 1;
}

int hmac::update(lua_State * L)
{
    target_type * self = lua::to<hmac>(L, 1);
    std::size_t data_len;
    const char * data = luaL_checklstring(L, 2, &data_len);
    HMAC_Update(self, reinterpret_cast<const unsigned char *>(data), data_len);
    return 0;
}

int hmac::digest(lua_State * L)
{
    target_type * self = lua::to<hmac>(L, 1);
    
    unsigned char * md = new unsigned char[EVP_MAX_MD_SIZE];
    unsigned int md_len = EVP_MAX_MD_SIZE;
    
    HMAC_Final(self, md, &md_len);
    
    std::size_t output_len = md_len * 2 + 1;
    char * output = new char[output_len];
    char * output_end = hex_encode(md, md + md_len, output, output + output_len);
    *output_end = '\0';
    
    lua_pushstring(L, output);
    
    delete [] md;
    delete [] output;
    
    return 1;
}

#endif

static int shutdown_crypto(lua_State * L)
{
    fclose(urandom);
    return 0;
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
    #ifndef WITHOUT_OPENSSL
        {"hmac", hmac::create_object},
    #endif
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
    lua_pop(L, 1);
    
    ecc::key::register_class(L);
    ecc::challenge::register_class(L);
    
    #ifndef WITHOUT_OPENSSL
    hmac::register_class(L);
    #endif
    
    lua::on_shutdown(L, shutdown_crypto);
}

} //namespace module
} //namespace lua
