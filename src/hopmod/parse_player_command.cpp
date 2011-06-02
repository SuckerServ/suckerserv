#include <vector>
#include <string>
#include <string.h>

std::vector<std::string> parse_player_command_line(const char * cmdline)
{
    std::vector<std::string> arguments;
    
    while(*cmdline)
    {
        cmdline += strspn(cmdline," ");
        if(!*cmdline) break;
        
        if(cmdline[0]=='\"' || cmdline[0]=='[')
        {
            char start = cmdline[0];
            cmdline++;
            const char * end = cmdline;
            if(*end=='\0') break;
            
            if(start == '[')
            {
                int nested = (*end != ']') + (*end == '[');
                while(nested && *(++end))
                {
                    if(*end =='[') nested++;
                    else if (*end==']') nested--;
                }
            }
            else end = cmdline + strcspn(cmdline,"\"");
            arguments.push_back(std::string(cmdline,end));
            cmdline = end + 1;
        }
        else
        {
            int skip = strcspn(cmdline," ");
            arguments.push_back(std::string(cmdline, cmdline+skip));
            cmdline = cmdline + skip;
        }
    }
    
    return arguments;
}
