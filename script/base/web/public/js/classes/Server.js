
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
        self.executeCommand("// nop");
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
        
    this.executeCommand("-- nop"); //try to trigger an error
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
        contentType: "text/x-lua",
        data: commandLine,
        success: success,
        error: error
    });
}

Server.prototype.makeCommand = function(){
    var commandLine = "server." + arguments[0] + "("; // Commands are always a function in the server table
    for(var i = 1; i < arguments.length; i++){
        var argument = arguments[i];
        if (typeof argument !== "number") { // Number as passed as-is, everything else is converted to string
          argument = argument.toString();
          argument = argument.replace(/\\/g, "\\\\"); // Escape backslashes
          argument = argument.replace(/"/g, "\\\""); // Escape double-quotes
          argument = "\"" + argument + "\""; // Enclose content in double-quotes to make a string
        }
        commandLine += (i > 1 ? ", " : "") + argument; // Separate subsequent arguments with comma
    }
    commandLine += ")";
    return commandLine;
}

Server.prototype.makeVariableSetter = function(){
    var argument = arguments[1];
    if (typeof argument !== "number") { // Number as passed as-is, everything else is converted to string
      argument = argument.toString();
      argument = argument.replace(/\\/g, "\\\\"); // Escape backslashes
      argument = argument.replace(/"/g, "\\\""); // Escape double-quotes
      argument = "\"" + argument + "\""; // Enclose content in double-quotes to make a string
    }
    return "server." + arguments[0] + " = " + argument; // Variables are always a key in the server table
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

