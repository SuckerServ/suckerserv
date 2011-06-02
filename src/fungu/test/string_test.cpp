#define BOOST_TEST_MODULE string_test
#include <boost/test/unit_test.hpp>

#include <fungu/string.hpp>
using namespace fungu;

#include <iostream>

BOOST_AUTO_TEST_CASE(less_than_compare)
{
    std::string s1("aaaaaaaaaa");
    std::string s2("z");
    BOOST_CHECK_EQUAL(string::comparison(s1,s2),-1);
    BOOST_CHECK_EQUAL(string::less_than(s1,s2),true);
}

BOOST_AUTO_TEST_CASE(equal_compare)
{
    std::string s1("test");
    std::string s2("test");
    BOOST_CHECK_EQUAL(string::comparison(s1,s2),0);
    BOOST_CHECK_EQUAL(string::equals(s1,s2),true);
    
    std::string apples("apples");
    std::string oranges("oranges");
    BOOST_CHECK_NE(string::equals(apples,oranges),true);
}

BOOST_AUTO_TEST_CASE(more_than_compare)
{
    std::string s1("az");
    std::string s2("aaaaaaaa");
    BOOST_CHECK_EQUAL(string::comparison(s1,s2),1);
    BOOST_CHECK_EQUAL(string::more_than(s1,s2),true);
}

BOOST_AUTO_TEST_CASE(multi_type_comparison)
{
    std::string s1("test");
    const_string s2(FUNGU_LITERAL_STRING("test"));
    BOOST_CHECK_EQUAL(string::equals(s1,s2),true);
}

BOOST_AUTO_TEST_CASE(constructors)
{
    const_string cs_default;
    BOOST_CHECK_EQUAL(cs_default.length(),(unsigned int)0);
    BOOST_CHECK_EQUAL(*cs_default.begin(),'\0');
    
    const char * literal_string = "tesz";
    const_string cs_literal(literal_string,literal_string+sizeof("test")-2);
    BOOST_CHECK_EQUAL(cs_literal.length(),(unsigned int)4);
    BOOST_CHECK_EQUAL(cs_literal.begin(),literal_string);
    BOOST_CHECK_EQUAL(*(cs_literal.end()-1),'z');
    
    std::string std_string("abc");
    const_string cs_string(std_string);
    BOOST_CHECK_EQUAL(cs_string.length(),(unsigned int)3);
    BOOST_CHECK_EQUAL(*cs_string.begin(),'a');
    BOOST_CHECK_EQUAL(*(cs_string.end()-1),'c');
    
    const_string cs_copied("abcdef");
    BOOST_CHECK_EQUAL(cs_copied.length(),(unsigned int)6);
    BOOST_CHECK_EQUAL(*cs_copied.begin(),'a');
    BOOST_CHECK_EQUAL(*(cs_copied.end()-1),'f');
    
    const_string cs_copy_ctor(cs_copied);
    BOOST_CHECK_EQUAL(cs_copy_ctor.length(),(unsigned int)6);
    BOOST_CHECK_EQUAL(*cs_copy_ctor.begin(),'a');
    BOOST_CHECK_EQUAL(*(cs_copy_ctor.end()-1),'f');
}

BOOST_AUTO_TEST_CASE(substring)
{
    const_string str(std::string("test"));
    BOOST_CHECK_EQUAL(str.substring(str.begin(),str.begin()).length(),(unsigned int)1);
}

BOOST_AUTO_TEST_CASE(to_int)
{
    BOOST_CHECK_EQUAL(const_string("21").to_int<int>(),21);
    BOOST_CHECK_EQUAL(const_string("-21").to_int<int>(),-21);
    BOOST_CHECK_EQUAL(const_string("2147483647").to_int<int>(),2147483647);
    BOOST_CHECK_EQUAL(const_string("-2147483646").to_int<int>(),-2147483646);
    BOOST_REQUIRE_THROW(const_string("4294967295").to_int<int>(),std::bad_cast);
    BOOST_REQUIRE_THROW(const_string("text").to_int<int>(),std::bad_cast);
    BOOST_CHECK_EQUAL(const_string("4294967295").to_int<unsigned int>(),(unsigned int)0xFFFFFFFF);
    BOOST_REQUIRE_THROW(const_string("-1").to_int<unsigned int>(),std::bad_cast);
}

BOOST_AUTO_TEST_CASE(from_int)
{
    BOOST_CHECK_EQUAL(const_string::from_int(21),"21");
    BOOST_CHECK_EQUAL(const_string::from_int(-1),"-1");
    BOOST_CHECK_EQUAL(const_string::from_int(2147483647),"2147483647");
    BOOST_CHECK_EQUAL(const_string::from_int(-2147483646),"-2147483646");
    BOOST_CHECK_EQUAL(const_string::from_int<unsigned int>(0xFFFFFFFF),"4294967295");
}

BOOST_AUTO_TEST_CASE(scan_newline_test)
{
    const char * unixstyle = "\n";
    const char * winstyle = "\r\n";
    const char * nonewline = "  ";
    
    const char * ptr = unixstyle;
    BOOST_CHECK(scan_newline(&ptr) && ptr == &unixstyle[1]);
    
    ptr = winstyle;
    BOOST_CHECK(scan_newline(&ptr) && ptr == &winstyle[2]);
    
    ptr = nonewline;
    BOOST_CHECK(!scan_newline(&ptr) && ptr == &nonewline[0]);
}
