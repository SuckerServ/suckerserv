#ifndef TEST_HPP
#define TEST_HPP

#include <boost/detail/lightweight_test.hpp>

#define TEST_ASSERT(c) if(!(c)) return false;
#define TEST_EXPECT_THROW(c, e) try{ c ; return false;}catch( e ){}

#endif
