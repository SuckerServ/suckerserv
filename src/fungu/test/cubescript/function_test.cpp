#include <iostream>
#include <fungu/script.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

void function_vi(int a)
{
    BOOST_CHECK(a==123);
}

BOOST_AUTO_TEST_CASE(call_function_vi)
{
    fungu::script::env runtime;
    fungu::script::env::frame global(&runtime);
    
    fungu::script::function<void (int)> function(function_vi);
    global.bind_object(&function,FUNGU_OBJECT_ID("function1"));
    
    fungu::script::expression expr;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("function1 123;"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin();
    expr.parse(&readptr,code.end()-1,&global);
    
    BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(expr.eval(&global)),"") );
}
