#include "cube.h"
#include <stdio.h>

int main(int argc, const char ** argv)
{
    FILE * rng = fopen("/dev/urandom","r");
    if(!rng) return 1;
    
    char seed[256 * sizeof(unsigned int) + 1];
    if(fread(seed, sizeof(unsigned int), sizeof(seed)/sizeof(unsigned int), rng) != sizeof(seed)/sizeof(unsigned 
int))
        return -1;

    seed[sizeof(seed)-1] = '\0';

    vector<char> privkey, pubkey;
    genprivkey(seed, privkey, pubkey);
    
    puts(privkey.getbuf());
    puts(pubkey.getbuf());
    
    fclose(rng);
    
    return 0;
}
