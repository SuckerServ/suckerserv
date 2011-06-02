#include "test.hpp"
#include "../player_command.hpp"
#include <iostream>

typedef std::vector<std::string> tokens;

bool test_blank()
{
    TEST_ASSERT(parse_player_command_line("").size() == 0);
    TEST_ASSERT(parse_player_command_line("  ").size() == 0);
    return true;
}

bool test_empty_args()
{
    TEST_ASSERT(parse_player_command_line("command  \"\" []").size() == 3);
    TEST_ASSERT(parse_player_command_line("command  \"\"").size() == 2);
    
    tokens args = parse_player_command_line("command \"\" []");
    TEST_ASSERT( args[0] == "command" );
    TEST_ASSERT( args[1].empty() );
    TEST_ASSERT( args[2].empty() );
    
    TEST_ASSERT(parse_player_command_line("[]").size() == 1);
    TEST_ASSERT(parse_player_command_line("\"\"").size() == 1);
    
    return true;
}

bool test_malformed()
{
    TEST_ASSERT(parse_player_command_line("command [").size()==1);
    TEST_ASSERT(parse_player_command_line("command \"").size()==1);
    // "command [a" and "command \"a" are acceptable cases
    return true;
}

bool test_normal()
{
    tokens args = parse_player_command_line("command arg1 arg2 arg3");
    
    TEST_ASSERT(args.size() == 4);
    TEST_ASSERT(args[0] == "command");
    TEST_ASSERT(args[1] == "arg1");
    TEST_ASSERT(args[2] == "arg2");
    TEST_ASSERT(args[3] == "arg3");
    
    tokens args2 = parse_player_command_line("command [arg1] \"arg2\" arg3");
    
    TEST_ASSERT(args2.size() == 4);
    TEST_ASSERT(args2[0] == "command");
    TEST_ASSERT(args2[1] == "arg1");
    TEST_ASSERT(args2[2] == "arg2");
    TEST_ASSERT(args2[3] == "arg3");
    
    tokens args3 = parse_player_command_line("[command] arg1");
    
    TEST_ASSERT(args3.size() == 2);
    TEST_ASSERT(args3[0] == "command");
    TEST_ASSERT(args3[1] == "arg1");
    
    tokens args4 = parse_player_command_line("\"command\" arg1");
    
    TEST_ASSERT(args4.size() == 2);
    TEST_ASSERT(args4[0] == "command");
    TEST_ASSERT(args4[1] == "arg1");
    
    return true;
}

bool test_nested_blocks()
{
    tokens args = parse_player_command_line("[[]]");
    TEST_ASSERT(args.size()==1);
    TEST_ASSERT(args[0]=="[]");
    return true;
}

int main()
{
    BOOST_TEST(test_blank());
    BOOST_TEST(test_empty_args());
    BOOST_TEST(test_malformed());
    BOOST_TEST(test_normal());
    BOOST_TEST(test_nested_blocks());
    return boost::report_errors();
}
