#include "prefix_tree.hpp"
using namespace hopmod::ip;
#include <iostream>
#include <cassert>
#include <limits>

address_prefix ip_127_0_0_1     (address(127, 0, 0, 1), address_mask(32));
address_prefix ip_127           (address(127, 0, 0, 0), address_mask(8));
address_prefix ip_127_1_0_1     (address(127, 1, 0, 1), address_mask(32));
address_prefix ip_240_0_0_1     (address(240, 0, 0, 1), address_mask(32));

void test_insert_and_lookup()
{
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127_0_0_1, 1);
        
        address_prefix lookup = ip_127_0_0_1;
        assert(tree.next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 1);
        
        address_prefix lookup = ip_127_0_0_1;
        assert(tree.next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127_0_0_1, 1);
        tree.insert(ip_127, 2);
        
        address_prefix lookup = ip_127_0_0_1;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 2);
        
        child = child->next_match(&lookup);
        assert(child && child->value() == 1);
        
        assert(!child->next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 2);
        tree.insert(ip_127_0_0_1, 1);
        
        address_prefix lookup = ip_127_0_0_1;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 2);
        
        child = child->next_match(&lookup);
        assert(child && child->value() == 1);
        
        assert(!child->next_match(&lookup));
    }

    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 2);
        tree.insert(ip_127_0_0_1, 1);
        tree.insert(ip_240_0_0_1, 3);
        
        address_prefix lookup = ip_127_0_0_1;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 2);
        
        child = child->next_match(&lookup);
        assert(child && child->value() == 1);
        
        assert(!child->next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 2);
        tree.insert(ip_127_0_0_1, 1);
        tree.insert(ip_240_0_0_1, 3);
        tree.insert(ip_127_1_0_1, 4);
        
        address_prefix lookup = ip_127_0_0_1;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 2);
        
        child = child->next_match(&lookup);
        assert(child && child->value() == 1);
        
        assert(!child->next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 2);
        tree.insert(ip_127_0_0_1, 1);
        tree.insert(ip_240_0_0_1, 3);
        tree.insert(ip_127_1_0_1, 4);
        
        address_prefix lookup = ip_127_1_0_1;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 2);
        
        child = child->next_match(&lookup);
        assert(child && child->value() == 4);
        
        assert(!child->next_match(&lookup));
    }
}

void test_erase()
{
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 1);
        tree.insert(ip_127_0_0_1, 2);
        tree.insert(ip_240_0_0_1, 3);
        tree.insert(ip_127_1_0_1, 4);
        
        tree.erase(ip_127);
        
        address_prefix lookup = ip_127;
        assert(!tree.next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 1);
        tree.insert(ip_127_0_0_1, 2);
        tree.insert(ip_240_0_0_1, 3);
        tree.insert(ip_127_1_0_1, 4);
        
        tree.erase(ip_127);
        
        address_prefix lookup = ip_127_1_0_1;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 4);
        assert(!child->next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 1);
        tree.insert(ip_127_0_0_1, 2);
        tree.insert(ip_240_0_0_1, 3);
        tree.insert(ip_127_1_0_1, 4);
        
        tree.erase(ip_240_0_0_1);
        
        address_prefix lookup = ip_127_1_0_1;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 1);
        
        child = child->next_match(&lookup);
        assert(child && child->value() == 4);
        
        assert(!child->next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 1);
        tree.insert(ip_127_0_0_1, 2);
        tree.insert(ip_240_0_0_1, 3);
        tree.insert(ip_127_1_0_1, 4);
        
        tree.erase(ip_127_0_0_1);
        
        address_prefix lookup = ip_127_1_0_1;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 1);
        
        child = child->next_match(&lookup);
        assert(child && child->value() == 4);
        
        assert(!child->next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 1);
        tree.insert(ip_127_0_0_1, 2);
        tree.insert(ip_240_0_0_1, 3);
        tree.insert(ip_127_1_0_1, 4);
        
        tree.erase(ip_127);
        tree.erase(ip_127_0_0_1);
        
        address_prefix lookup = ip_127_1_0_1;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 4);
        assert(!child->next_match(&lookup));
    }
    
    {
        address_prefix_tree<int> tree;
        tree.insert(ip_127, 1);
        
        tree.erase(address_prefix(address(0), address_mask(0)));
        
        address_prefix lookup = ip_127;
        
        address_prefix_tree<int> * child = tree.next_match(&lookup);
        assert(child && child->value() == 1);
        assert(!child->next_match(&lookup));
    }
}


int main()
{
    test_insert_and_lookup();
    test_erase();
    return 0;
}
