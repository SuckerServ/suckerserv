#include <map>
#include <string>
#include <limits>
#include <iostream>

class global_player_id
{
public:
    global_player_id()
     :ip(0)
    {
        
    }
    
    global_player_id(const std::string & n, unsigned long i)
     :name(n), ip(i)
    {
        
    }
    
    bool operator<(const global_player_id & x)const
    {
        return ip < x.ip || (ip == x.ip && name < x.name);
    }
    
    std::string name;
    unsigned long ip;
};

typedef unsigned int local_player_id;
static local_player_id next_id = 1;
static const local_player_id max_id = std::numeric_limits<local_player_id>::max();

typedef std::map<global_player_id, local_player_id> global_to_local_id_map;
static global_to_local_id_map id_map;

local_player_id get_player_id(const char * name, unsigned long ip)
{
    global_player_id gid(name, ip);
    global_to_local_id_map::iterator it = id_map.find(gid);
    
    if(it == id_map.end())
    {
        if(next_id == max_id)
        {
            std::cerr<<"Exhausted free player ID range. Server should be restarted."<<std::endl;
            return max_id;
        }
        
        local_player_id id = next_id++;
        id_map[gid] = id;
        return id;
    }
    else return it->second;
}

void clear_player_ids()
{
    id_map.clear();
}
