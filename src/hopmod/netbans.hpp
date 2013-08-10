namespace hopmod{
namespace netbans{

using namespace hopmod::ip;

bool check_ban(address::integral_type ip);
void add_ban(const char *addr);
void clear_bans();

} //namespace netbans
} //namespace hopmod
