#include "cube.h"
#include <stdio.h>

int main(int argc, const char ** argv)
{
    FILE * rng = fopen("/dev/urandom","r");
    if(!rng) return 1;
    
    unsigned int seed[256];
    if(!fread(seed, sizeof(unsigned int), sizeof(seed)/sizeof(unsigned int), rng))
        return -1;
    
    vector<char> privkey, pubkey;
    genprivkey(seed,sizeof(seed), privkey, pubkey);
    
    puts(privkey.getbuf());
    puts(pubkey.getbuf());
    
    fclose(rng);
    
    return 0;
}
