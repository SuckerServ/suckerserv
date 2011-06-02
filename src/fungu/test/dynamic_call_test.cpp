#define BOOST_TEST_MODULE dynamic_call_test
#include <boost/test/unit_test.hpp>

#include <string>
#include <deque>
#include <boost/lexical_cast.hpp>

#include <fungu/dynamic_call.hpp>
using namespace fungu;

class serializer
{
public:
    std::string get_void_value()
    {
        return "null";
    }
    
    template<typename T>
    std::string serialize(const T & value)
    {
        return boost::lexical_cast<std::string>(value);
    }
    
    template<typename T>
    T deserialize(const std::string & strval, target_tag<T>)
    {
        return boost::lexical_cast<T>(strval);
    }
};

void void_nullary(){}
int int_nullary(){return 3;}

BOOST_AUTO_TEST_CASE(nullary_calls)
{
    serializer s;
    std::deque<std::string> args;
    
    dynamic_call<void ()> vf0(void_nullary);
    BOOST_CHECK_EQUAL(vf0(args,s),"null");
    
    dynamic_call<int ()> if0(int_nullary);
    BOOST_CHECK_EQUAL(if0(args,s),"3");
}

void void_unary(int a)
{
    BOOST_CHECK_EQUAL(a,1);
}

int int_unary(int a)
{
    BOOST_CHECK_EQUAL(a,1);
    return 3;
}

BOOST_AUTO_TEST_CASE(unary_calls)
{
    serializer s;
    std::deque<std::string> args;
    
    dynamic_call<void (int)> vf1(void_unary);
    args.push_back("1");
    BOOST_CHECK_EQUAL(vf1(args,s),"null");
    BOOST_CHECK_EQUAL(args.empty(),true);
    
    dynamic_call<int (int)> if1(int_unary);
    args.push_back("1");
    BOOST_CHECK_EQUAL(if1(args,s),"3");
    BOOST_CHECK_EQUAL(args.empty(),true);
}

void test_reference_arg(const std::string & a)
{
    BOOST_CHECK_EQUAL(a,"test string");
}

BOOST_AUTO_TEST_CASE(reference_argument)
{
    serializer s;
    std::deque<std::string> args;
    dynamic_call<void (const std::string &)> f1(test_reference_arg);
    args.push_back("test string");
    f1(args,s);
}

BOOST_AUTO_TEST_CASE(not_enough_args)
{
    serializer s;
    std::deque<std::string> args;
    dynamic_call<void (const std::string &)> f1(test_reference_arg);
    BOOST_CHECK_THROW(f1(args,s),missing_args);
}

struct foo
{
    std::string emptyTestMethod()
    {
        return "success";
    }
};

BOOST_AUTO_TEST_CASE(simple_method_call)
{
    dynamic_method_call<foo, std::string ()> method_call(&foo::emptyTestMethod);
    serializer s;
    std::deque<std::string> args;
    foo object;
    BOOST_CHECK_EQUAL(method_call(&object,args,s),"success");
}

