// (c) 2011 Thomas
// Description: Functions for loading map items, flags and bases statically 

void add_item(int n, int v) // normal items
{
    if (gamemillis) return;
    server_entity se = { NOTUSED, 0, false };
    while(sents.length()<=n) sents.add(se);
    sents[n].type = v;
    if(canspawnitem(sents[n].type))
    {
        if(m_mp(gamemode) && delayspawn(sents[n].type)) sents[n].spawntime = spawntime(sents[n].type);
        else sents[n].spawned = true;
    }
    notgotitems = false;
}


void add_flag(int num, int team, int y) // ctf flags
{
    if ((!m_hold && !m_ctf && !m_protect) || gamemillis) return;
    vec o;
    o[0] = max(0/DMF, 0.0f); // unused
    o[1] = max(y/DMF, 0.0f);
    o[2] = max(0/DMF, 0.0f); // unused
    if(m_hold) ctfmode.addholdspawn(o);
    else ctfmode.addflag(num, o, team, m_protect ? lastmillis : 0);
    ctfmode.notgotflags = false;
}

void prepare_hold_mode()
{
    if (!m_hold || gamemillis) return;
    if(ctfmode.holdspawns.length()) while(ctfmode.flags.length() < ctfmode.HOLDFLAGS)
    {
        int i = ctfmode.flags.length();
        if(!ctfmode.addflag(i, vec(0, 0, 0), 0, 0)) break;
        ctfservmode::flag &f = ctfmode.flags[i];
        ctfmode.spawnflag(i);
        sendf(-1, 1, "ri6", N_RESETFLAG, i, ++f.version, f.spawnindex, 0, 0);
    }
}

void add_base(int type, int x, int y, int z) // capture bases
{
    if (!m_capture || gamemillis) return;
    int ammotype = type;
    vec o;
    o[0] = max(x/DMF, 0.0f);
    o[1] = max(y/DMF, 0.0f);
    o[2] = max(z/DMF, 0.0f);
		
    capturemode.addbase(ammotype>=GUN_SG && ammotype<=GUN_PISTOL ? ammotype : min(ammotype, 0), o);
    capturemode.notgotbases = false;
}

void prepare_capture_mode()
{
    if (!m_capture || gamemillis) return;
    capturemode.sendbases();
    loopv(clients) if(clients[i]->state.state==CS_ALIVE) capturemode.entergame(clients[i]);
}

