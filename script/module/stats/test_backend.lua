function commit_test_game_1(committer)
    
    local game = {
        datetime    = os.time(),
        duration    = 10,
        mode        = "insta ctf",
        map         = "hallo",
        finished    = true,
        players     = 2,
        bots        = 0
    }
    
    local teams = {
        {
            name        = "evil",
            score       = 2,
            win         = true,
            draw        = false
        },
        {
            name        = "good",
            score       = 1,
            win         = false,
            draw        = false
        }
    }
    
    local players = {
        {
            name            = "evil_player",
            team            = "evil",
            ipaddr          = "127.0.0.1",
            ipaddrlong      = 1065353217,
            country         = "GB",
            team_id         = 0,
            playing         = true, 
            timeplayed      = 10, 
            finished        = true, 
            win             = true, 
            rank            = 1,
            botskill        = -1,
            teamkills       = 0,
            score           = 10,
            frags           = 10,
            deaths          = 0,
            suicides        = 0,
            hits            = 10,
            misses          = 0,
            shots           = 10,
            damage          = 10,
            damagewasted    = 0
        },
        {
            name            = "good_player",
            team            = "good",
            ipaddr          = "127.0.0.1",
            ipaddrlong      = 1065353217,
            country         = "GB",
            team_id         = 0,
            playing         = true, 
            timeplayed      = 10, 
            finished        = true, 
            win             = true, 
            rank            = 1,
            botskill        = -1,
            teamkills       = 0,
            score           = 0,
            frags           = 0,
            deaths          = 10,
            suicides        = 0,
            hits            = 0,
            misses          = 20,
            shots           = 0,
            damage          = 0,
            damagewasted    = 20
        }
    }
    
    committer(game, players, teams)
end

