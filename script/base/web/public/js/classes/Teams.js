
function Teams(server){

    var self = this;
    this.score = {};
    
    var eventService = new EventDemultiplexer();
    var eventDispatcher = new EventDispatcher(this);
    
    function event_handler(name, func){
        eventService.addListener(name, func);
    }
    
    function getTeamScores(callback){
        $.getJSON("/teams", function(response, textStatus){
            
            if(textStatus != "success"){
                callback(false);
                return;
            }
            
            $.each(response, function(team, value){
                self.score[team] = value.score;
            });
            
            callback(true);
        });
    }
    
    event_handler("scoreupdate", function(team, score){
        self.score[team] = score;
        eventDispatcher.signalEvent("scoreupdate", team, score);
    });
    
    event_handler("mapchange", function(map, gamemode){
        self.score = {};
        if(GamemodeInfo[gamemode].teams){
            getTeamScores(function(success){
                eventDispatcher.signalEvent("mapchange", map, gamemode);
            });
        }
        else{
            eventDispatcher.signalEvent("mapchange", map, gamemode);
        }
    });
    
    eventService.startListening();
    
    getTeamScores(function(success){
         if(!success){
            server.signalError();
            return;
        }
        eventDispatcher.signalEvent("ready");
    });
}

