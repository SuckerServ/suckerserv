#include "test.hpp"
#include "../schedule_service.hpp"

static bool called = false;
static int setcalled(){called = true; return 0;}
static int setcalled_cancel(){called = true; return -1;}

static bool called_1 = false;
static bool called_2 = false;
static int setcalled_1(){called_1 = true; return 0;}
static int setcalled_2(){called_2 = true; return 0;}

bool test_sleep()
{
    called = false;
    schedule_service s;
    s.add_job(setcalled, 10);
    s.update(9);
    TEST_ASSERT(!called);
    s.update(10);
    TEST_ASSERT(called);
    return true;
}

bool test_interval()
{
    called = false;
    schedule_service s;
    s.add_job(setcalled, 10, true);
    s.update(11);
    TEST_ASSERT(called);
    called = false;
    s.update(20);
    TEST_ASSERT(called);
    return true;
}

bool test_interval_cancel()
{
    called = false;
    schedule_service s;
    s.add_job(setcalled_cancel, 10, true);
    s.update(10);
    TEST_ASSERT(called);
    called = false;
    s.update(20);
    TEST_ASSERT(!called);
    return true;
}

bool test_cancel_all()
{
    called = false;
    schedule_service s;
    s.add_job(setcalled, 10);
    s.add_job(setcalled, 5);
    s.cancel_all();
    s.update(10);
    TEST_ASSERT(!called);
    return true;
}

bool test_launch_order()
{
    called_1 = false;
    called_2 = false;
    schedule_service s;
    s.add_job(setcalled_1, 10);
    s.add_job(setcalled_2, 5);
    s.update(4);
    TEST_ASSERT(!called_1 && !called_2);
    s.update(5);
    TEST_ASSERT(called_2 && !called_1);
    s.update(10);
    TEST_ASSERT(called_1);
    return true;
}

int main()
{
    BOOST_TEST(test_sleep());
    BOOST_TEST(test_interval());
    BOOST_TEST(test_interval_cancel());
    BOOST_TEST(test_cancel_all());
    BOOST_TEST(test_launch_order());
    return boost::report_errors();
}
