#include <iostream>
#include <fungu/script.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

fungu::script::env gs_env;

BOOST_AUTO_TEST_CASE(parse_word)
{
    fungu::script::expression::word<fungu::script::expression::word_exit_terminals> word;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("hello world"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin();
    
    BOOST_CHECK( word.parse(&readptr,code.begin()+2,gs_env.get_global_scope()) == fungu::script::PARSE_PARSING );
    BOOST_CHECK( readptr == code.begin() + 3 );
    
    BOOST_CHECK( word.parse(&readptr,code.end()-1,gs_env.get_global_scope()) == fungu::script::PARSE_COMPLETE );
    BOOST_CHECK( readptr == code.begin() + 5 );
}

BOOST_AUTO_TEST_CASE(parse_quote)
{
    fungu::script::expression::quote quote;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("\"hello\" world"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin()+1;
    
    BOOST_CHECK( quote.parse(&readptr,code.begin()+2,gs_env.get_global_scope()) == fungu::script::PARSE_PARSING );
    BOOST_CHECK( readptr == code.begin() + 3 );
    
    BOOST_CHECK( quote.parse(&readptr,code.end()-1,gs_env.get_global_scope()) == fungu::script::PARSE_COMPLETE );
    BOOST_CHECK( readptr == code.begin() + 7 );
}

BOOST_AUTO_TEST_CASE(parse_block)
{
    fungu::script::expression::block block;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("[hello []] world"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin()+1;
    
    BOOST_CHECK( block.parse(&readptr,code.begin()+2,gs_env.get_global_scope()) == fungu::script::PARSE_PARSING );
    BOOST_CHECK( readptr == code.begin() + 3 );
    
    BOOST_CHECK( block.parse(&readptr,code.end()-1,gs_env.get_global_scope()) == fungu::script::PARSE_COMPLETE );
    BOOST_CHECK( readptr == code.begin() + 10 );
}

BOOST_AUTO_TEST_CASE(parse_comment)
{
    fungu::script::expression::comment comment;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("//hello world\n"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin()+1;
    
    BOOST_CHECK( comment.parse(&readptr,code.begin()+2,gs_env.get_global_scope()) == fungu::script::PARSE_PARSING );
    BOOST_CHECK( readptr == code.begin() + 3 );
    
    BOOST_CHECK( comment.parse(&readptr,code.end()-1,gs_env.get_global_scope()) == fungu::script::PARSE_COMPLETE );
    BOOST_CHECK( readptr == code.begin() + 13 );
}

BOOST_AUTO_TEST_CASE(parse_expression)
{
    fungu::script::expression expr;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("hello world \"hello world\" [hello world] //hello world\n"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin();
    
    BOOST_CHECK( expr.parse(&readptr,code.begin()+2,gs_env.get_global_scope()) == fungu::script::PARSE_PARSING );
    BOOST_CHECK( readptr == code.begin() + 3 );
    
    BOOST_CHECK( expr.parse(&readptr,code.end()-1,gs_env.get_global_scope()) == fungu::script::PARSE_COMPLETE );
    BOOST_CHECK( readptr == code.end() );
}

BOOST_AUTO_TEST_CASE(parse_subexpression)
{
    fungu::script::expression expr;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("hello (hello world)\n"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin();
    BOOST_CHECK(expr.parse(&readptr,code.end()-1,gs_env.get_global_scope()) == fungu::script::PARSE_COMPLETE);
}

BOOST_AUTO_TEST_CASE(parse_macro)
{
    fungu::script::expression::macro macro;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("@@@test "));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin()+1;
    BOOST_CHECK( macro.parse(&readptr,code.begin()+1,gs_env.get_global_scope()) == fungu::script::PARSE_PARSING );
    BOOST_CHECK( macro.parse(&readptr,code.end()-1,gs_env.get_global_scope()) == fungu::script::PARSE_COMPLETE );
    BOOST_CHECK( macro.get_evaluation_level() == 3 );
}

BOOST_AUTO_TEST_CASE(parse_macro_expression)
{
    fungu::script::expression::macro macro;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("@@@(hello world [hello world])"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin()+1;
    BOOST_CHECK( macro.parse(&readptr,code.begin()+1,gs_env.get_global_scope()) == fungu::script::PARSE_PARSING );
    BOOST_CHECK( macro.parse(&readptr,code.end()-1,gs_env.get_global_scope()) == fungu::script::PARSE_COMPLETE );
    BOOST_CHECK( macro.get_evaluation_level() == 3 );
}

BOOST_AUTO_TEST_CASE(eval_word)
{
    fungu::script::expression::word<fungu::script::expression::word_exit_terminals> word;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("hello\n"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin();
    word.parse(&readptr,code.end()-1,NULL);
    BOOST_CHECK( 
        fungu::string::equals(
            fungu::script::lexical_cast<fungu::immutable_ascii_string>(
                word.eval(NULL)),"hello") );   
}

class testclass:public fungu::script::env::object
{
public:
    virtual fungu::script::result_type apply(apply_arguments & args,fungu::script::env::frame *)
    {
        BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(args.front()),"arg1") );
        args.pop_front();
        
        BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(args.front()),"arg2") );
        args.pop_front();
        
        BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(args.front()),"arg3") );
        args.pop_front();
        
        return fungu::immutable_ascii_string(FUNGU_LITERAL_STRING("testclass_apply_result"));
    }
    virtual fungu::script::result_type value()
    {
        return fungu::immutable_ascii_string(FUNGU_LITERAL_STRING("deref value"));
    }
};

BOOST_AUTO_TEST_CASE(eval_word_deref)
{
    fungu::script::env runtime;
    fungu::script::env::frame global(&runtime);
    testclass obj1;
    global.bind_object(&obj1,fungu::immutable_ascii_string("object1"));
    
    fungu::script::expression::symbol<fungu::script::expression::word_exit_terminals> symbol;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("$object1\n"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin()+1;
    symbol.parse(&readptr,code.end()-1,&global);
    
    BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(
         symbol.eval(&global)),"deref value") );
}

BOOST_AUTO_TEST_CASE(eval_quote)
{
    fungu::script::expression::quote quote;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("\"hello world\""));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin()+1;
    quote.parse(&readptr,code.end()-1,NULL);
    BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(
        quote.eval(NULL)),"hello world") );   
}

BOOST_AUTO_TEST_CASE(eval_block)
{
    fungu::script::expression::block block;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("[hello\n [@test] world]"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin()+1;
    block.parse(&readptr,code.end()-1,NULL);
    BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(
        block.eval(NULL)),"hello\n [@test] world") );   
}

BOOST_AUTO_TEST_CASE(eval_block_macro)
{
    fungu::script::env runtime;
    fungu::script::env::frame global(&runtime);
    testclass obj1;
    global.bind_object(&obj1,fungu::immutable_ascii_string("object1"));
    
    fungu::script::expression::block block;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("[hello @object1 world]"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin()+1;
    block.parse(&readptr,code.end()-1,&global);
    
    BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(block.eval(&global)),"hello deref value world") );   
}

BOOST_AUTO_TEST_CASE(eval_expression)
{
    fungu::script::env runtime;
    fungu::script::env::frame global(&runtime);
    testclass obj1;
    global.bind_object(&obj1,fungu::immutable_ascii_string("object1"));
    
    fungu::script::expression expr;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("object1 arg1 [arg2] \"arg3\";"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin();
    expr.parse(&readptr,code.end()-1,&global);
    
    BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(expr.eval(&global)),"testclass_apply_result") );
}

BOOST_AUTO_TEST_CASE(eval_numeric_expression)
{
    fungu::script::env runtime;
    fungu::script::env::frame global(&runtime);
    
    fungu::script::expression expr;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("123;"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin();
    expr.parse(&readptr,code.end()-1,&global);
    
    BOOST_CHECK( fungu::string::equals(fungu::script::lexical_cast<fungu::immutable_ascii_string>(expr.eval(&global)),"123") );
}

BOOST_AUTO_TEST_CASE(form_expression_source)
{
    fungu::script::env runtime;
    fungu::script::env::frame global(&runtime);
    
    fungu::script::expression expr;
    fungu::immutable_ascii_string code(FUNGU_LITERAL_STRING("hello arg1 [arg2] \"arg3\" (arg4);"));
    fungu::immutable_ascii_string::const_iterator readptr=code.begin();
    expr.parse(&readptr,code.end()-1,&global);
    
    BOOST_CHECK( fungu::string::equals(expr.form_source(),"hello arg1 [arg2] \"arg3\" (arg4)\n") );
}
