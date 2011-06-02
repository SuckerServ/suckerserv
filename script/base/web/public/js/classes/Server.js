
function Server(){

    var self = this;
    
    var eventService = new EventDemultiplexer();
    var eventDispatcher = new EventDispatcher(this);
    
    var loading = {
        "server"    : "Server Variables",
        "game"      : "Game Variables",
        "client"    : "Clients",
        "teams"     : "Teams"
    };
    
    var monitor_vars = {
        maxclients : true,
        mastermode : true,
        servername : true
    }
    
    function addReady(loadingKey){
    
        delete loading[loadingKey];
        eventDispatcher.signalEvent("loading", loading);
        
        if(isEmptyObject(loading)){
            eventDispatcher.signalEvent("ready");
            eventDispatcher.emptyListeners("ready");
        }
    }
    
    this.ready = function(callback){
        if(!isEmptyObject(loading)){
            eventDispatcher.addListener("ready", callback);
        }else{
            callback();
        }
    }
    
    this.signalError = function(){
        self.executeCommand("1");
    }
    
    this.signalLostConnection = function(){
        eventDispatcher.signalEvent("lost-connection");
    }
    
    this.signalUnauthorized = function(){
        eventDispatcher.signalEvent("unauthorized");
    }
    
    function getServerState(callback){
        self.getServerVariables(["web_admin_session_username", "serverip", "serverport", "maxclients", "mastermode", "server_password", "uptime", "servername"], callback);
    }
    
    getServerState(function(success, response){
        
        if(!success){
            self.signalError();
            return;
        }
        
        $.each(response, function(name, value){
            self[name] = value;
        });
        
        addReady("server");
    });
    
    function event_handler(name, func){
        eventService.addListener(name, func);
    }
    
    event_handler("shutdown", function(){
        eventDispatcher.signalEvent("shutdown");
    });
    
    event_handler("admin-message", function(admin, message){
        eventDispatcher.signalEvent("admin-message", admin, message);
    });
    
    event_handler("text", function(cn, message){
        eventDispatcher.signalEvent("text", self.clients.getClient(cn), message);
    });
    
    event_handler("sayteam", function(cn, message){
        eventDispatcher.signalEvent("sayteam", self.clients.getClient(cn), message);
    });
    
    event_handler("varchanged", function(varname){
        if(monitor_vars[varname]){
               self.getServerVariables([varname], function(success, response){
                   if(!success) return;
                   self[varname] = response[varname];
                   eventDispatcher.signalEvent("varchanged", varname);
               });
        }
        else{
            eventDispatcher.signalEvent("varchanged", varname);
        }
    });
    
    eventService.startListening();
    
    this.clients = new ClientUpdateSet(this);
    this.game = new Game(this);
    this.teams = new Teams(this);
    
    this.clients.addListener("ready", function(){addReady("client");});
    this.game.addListener("ready", function(){addReady("game");});
    this.teams.addListener("ready", function(){addReady("teams");});
    
    var client_mapchange = 0;
    var game_mapchange = 0;
    var teams_mapchange = 0;
    
    function isReadyToSignalMapchange(){
        return client_mapchange == game_mapchange && client_mapchange == teams_mapchange;
    }
    
    this.clients.addListener("mapchange", function(map, gamemode){
        client_mapchange++;
        if(isReadyToSignalMapchange()){
            eventDispatcher.signalEvent("mapchange", map, gamemode); 
        }
    });
    
    this.game.addListener("mapchange", function(map, gamemode){
        game_mapchange++;
        if(isReadyToSignalMapchange()){
            eventDispatcher.signalEvent("mapchange", map, gamemode); 
        }
    });
    
    this.teams.addListener("mapchange", function(map, gamemode){
        teams_mapchange++;
        if(isReadyToSignalMapchange()){
            eventDispatcher.signalEvent("mapchange", map, gamemode); 
        }
    });
        
    this.executeCommand("1"); //try to trigger an error
}

Server.prototype.executeCommand = function(commandLine, responseHandler){
    
    var serverObject = this;
    
    if(!responseHandler) responseHandler = function(){};
    
    function success(data, textStatus){
        
        serverObject.isConnected = true;
        serverObject.isLoggedIn = true;
        
        responseHandler(true, data);
    }
    
    function error(HttpObject, textStatus, errorThrown){
        
        switch(HttpObject.status){
            case 0:
            case 12029:
                serverObject.signalLostConnection();
                responseHandler(false,"<connection broken>");
                break;
            case 401:
                serverObject.signalUnauthorized();
                responseHandler(false, "<not logged in>");
                break;
            default:
                responseHandler(false, HttpObject.responseText);
        }
    }

    $.ajax({
        type:"POST",
        url:"/serverexec",
        contentType: "text/x-cubescript",
        data: commandLine,
        success: success,
        error: error
    });
}

Server.prototype.makeCommand = function(){
    var commandLine = "";
    for(var i = 0; i < arguments.length; i++){
        commandLine += (i > 0 ? " " : "") + "[" + arguments[i] + "]";
    }
    return commandLine;
}

Server.prototype.getServerVariables = function(varset, completionHandler){

    var serverObject = this;
    var queryvars = $.toJSON(varset);
    $.post("/queryvars", queryvars, function(response, textStatus){
        
        if(textStatus != "success"){
            completionHandler(false, {});
            serverObject.signalError();
            return;
        }
        
        completionHandler(true, response);
        
    }, "json");
}

var mastermodeName = ["open", "veto", "locked", "private", "password"];

Server.prototype.serverCalls = function(calls, completionHandler){

    var serverObject = this;
    
    $.post("/calls", $.toJSON(calls), function(response, textStatus){
        
        if(textStatus != "success"){
            completionHandler(false, {});
            serverObject.signalError();
            return;
        }
        
        completionHandler(true, response);
        
    }, "json");
}

