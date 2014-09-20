namespace hopmod{
namespace netbans{

using namespace hopmod::ip;

bool check_ban(address::integral_type ip);
void push_bans();
void pop_bans();
void merge_pushed_bans();
void add_ban(const char * addr, bool persist, const char * reason);
void del_ban(const char * addr);
void clear_bans();
void ban_list();

void init();
void shutdown();

} //namespace netbans
} //namespace hopmod
