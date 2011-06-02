#include <iostream>
#include <fungu/script.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

class foo:public fungu::script::env::object
{
public:
    fungu::script::result_type apply(apply_arguments & args,frame * aScope)
    {
        return fungu::immutable_ascii_string();
    }
    
    fungu::script::result_type value()
    {
        return fungu::immutable_ascii_string();
    }
};

BOOST_AUTO_TEST_CASE(object_not_found)
{
    fungu::script::env runtime;
    fungu::script::env::frame global(&runtime);
    BOOST_CHECK(!global.lookup_object(FUNGU_RUNTIME_OBJECT_ID("object1")));
}

BOOST_AUTO_TEST_CASE(simple_use)
{
    fungu::script::env runtime;
    fungu::script::env::frame global(&runtime);
    foo obj1;
    global.bind_object(&obj1,FUNGU_RUNTIME_OBJECT_ID("object1"));
    BOOST_CHECK(global.lookup_object(FUNGU_RUNTIME_OBJECT_ID("object1"))==&obj1);
}

BOOST_AUTO_TEST_CASE(register_and_lookup_from_inners)
{
    fungu::script::env runtime;
    fungu::script::env::frame parent(&runtime);
    fungu::script::env::frame inner(&parent);
    fungu::script::env::frame inner2(&inner);
    
    foo obj1;
    parent.bind_object(&obj1,FUNGU_RUNTIME_OBJECT_ID("object1"));
    
    foo obj2;
    inner.bind_object(&obj2,FUNGU_OBJECT_ID("object2"));
    
    BOOST_CHECK(inner.lookup_object(FUNGU_RUNTIME_OBJECT_ID("object1"))==&obj1);
    BOOST_CHECK(inner2.lookup_object(FUNGU_OBJECT_ID("object1"))==&obj1);
    BOOST_CHECK(inner2.lookup_object(FUNGU_OBJECT_ID("object2"))==&obj2);
}

BOOST_AUTO_TEST_CASE(register_and_lookup_outside_of_scope)
{
    fungu::script::env runtime;
    fungu::script::env::frame parent(&runtime);
    fungu::script::env::frame another(&runtime);
    
    foo obj1;
    parent.bind_object(&obj1,FUNGU_RUNTIME_OBJECT_ID("object1"));
    
    BOOST_CHECK(!another.lookup_object(FUNGU_RUNTIME_OBJECT_ID("object1")));
}

BOOST_AUTO_TEST_CASE(out_of_scope)
{
    fungu::script::env runtime;
    
    {
        fungu::script::env::frame first_parent(&runtime);
        foo obj1;
        first_parent.bind_object(&obj1,FUNGU_RUNTIME_OBJECT_ID("object1"));
    }
    
    {
        fungu::script::env::frame second_parent(&runtime);
        BOOST_CHECK(!second_parent.lookup_object(FUNGU_RUNTIME_OBJECT_ID("object1")));
    }
    
}

BOOST_AUTO_TEST_CASE(dynamic_scope)
{
    fungu::script::env runtime;
    fungu::script::env::frame parent(&runtime);
    fungu::script::env::frame child(&runtime);
    
    foo obj1;
    parent.bind_object(&obj1,FUNGU_RUNTIME_OBJECT_ID("object1")).set_dynamically_scoped();
    
    BOOST_CHECK(parent.lookup_object(FUNGU_OBJECT_ID("object1"))==&obj1);
    BOOST_CHECK(child.lookup_object(FUNGU_OBJECT_ID("object1"))==&obj1);
}
