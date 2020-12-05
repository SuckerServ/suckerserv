
function Game(server){

    var self = this;
    this.server = server;
    this.teams = {};
    
    var lastTimeupdate = null;
    
    var eventService = new EventDemultiplexer();
    var eventDispatcher = new EventDispatcher(this);
    
    function event_handler(name, func){
        eventService.addListener(name, func);
    }
    
    function getTimeNow(){
        return (new Date()).getTime();
    }
    
    function getFullGameState(callback){
        server.getServerVariables(["gamemode", "map", "seconds_left"], function(success, response){
            if(!success){
                if(callback) callback.apply(self, [false]);
                return;
            }
            $.each(response, function(name, value){
                self[name] = value;
            });
            
            if(self.seconds_left != null){
                lastTimeupdate = getTimeNow();
            }
            
            if(callback) callback.apply(self, [true]);
        });
    }
    
    this.update = getFullGameState;
    
    this.getTotalSecondsLeft = function(){
        if(!lastTimeupdate) return;
        return Math.max(self.seconds_left - (getTimeNow() - lastTimeupdate)/1000, 0);
    }
    
    function changeTimeLeft(minutesRemaining, secondsRemaining){
        self.timeleft = minutesRemaining;
        self.seconds_left = secondsRemaining;
        lastTimeupdate = getTimeNow();
        eventDispatcher.signalEvent("timeupdate", minutesRemaining, secondsRemaining);
    }
    
    event_handler("timeupdate", changeTimeLeft);
    
    event_handler("mapchange", function(map, gamemode){
        getFullGameState(function(){
             eventDispatcher.signalEvent("mapchange", map, gamemode);
        });
    });
    
    eventService.startListening();
    
    getFullGameState(function(success){
        if(!success){
            server.signalError();
            return;
        }
        eventDispatcher.signalEvent("ready");
    });
}

var GamemodeInfo = {

    "ffa" : {
        "teams"     : false,
        "items"     : true,
        "instagib"  : false
    },
    
    "coop edit" : {
        "teams"     : false,
        "items"     : false,
        "instagib"  : false
    },
    
    "teamplay" : {
        "teams"     : true,
        "items"     : true,
        "instagib"  : false
    },
    
    "instagib" : {
        "teams"     : false,
        "items"     : false,
        "instagib"  : true
    },
    
    "insta team" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : true
    },
    
    "efficiency" : {
        "teams"     : false,
        "items"     : false,
        "instagib"  : false
    },
    
    "effic team" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "tactics" : {
        "teams"     : false,
        "items"     : false,
        "instagib"  : false
    },
    
    "tac team" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "capture" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "regen capture" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "ctf" : {
        "teams"     : true,
        "items"     : true,
        "instagib"  : false
    },
    
    "insta ctf" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : true
    },

    "effic ctf" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "protect" : {
        "teams"     : true,
        "items"     : true,
        "instagib"  : false
    },
    
    "insta protect" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : true
    },

    "effic protect" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "hold" : {
        "teams"     : true,
        "items"     : true,
        "instagib"  : false
    },
    
    "insta hold" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : true
    },

    "effic hold" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },

    "insta collect" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : true
    },

    "effic collect" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },

    "collect" : {
        "teams"     : true,
        "items"     : true,
        "instagib"  : false
    }
};
