
function Authserver(){

    var self = this;
    
    var eventService = new EventDemultiplexer();
    var eventDispatcher = new EventDispatcher(this);

    function getTimeNow(){
        return (new Date()).getTime();
    }
    
    var loading = {
        "server"    : "Server Variables",
        "user"    : "Users",
    };
    
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
        self.executeCommand("-- nop");
    }
    
    this.signalLostConnection = function(){
        eventDispatcher.signalEvent("lost-connection");
    }
    
    this.signalUnauthorized = function(){
        eventDispatcher.signalEvent("unauthorized");
    }
    
    function getServerState(callback){
        self.getServerVariables(["web_admin_session_username", "serverip", "serverport", "uptime"], callback);
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
    
    this.users = new UserUpdateSet(this);

    this.users.addListener("ready", function(){addReady("user");});

    this.executeCommand("-- nop"); //try to trigger an error
}

Authserver.prototype.executeCommand = function(commandLine, responseHandler){
    
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
        url:"/authexec",
        contentType: "text/x-lua",
        data: commandLine,
        success: success,
        error: error
    });
}

Authserver.prototype.makeCommand = function(){
    var commandLine = "server.";
    for(var i = 0; i < arguments.length; i++){
        var argument = "" + arguments[i];
        if(argument.match(/[\r\n ;$@\/#"]/m)){
            argument = argument.replace(/([\r\n"])/gm, "\\$1");
            argument = "\"" + argument + "\"";
        }
        commandLine += (i > 1 ? ", " : "") + (i > 0 ? "\"" : "" ) + argument + (i > 0 ? "\"" : "") + (i == 0 ? "(" : i == arguments.length-1 ? ")" : "");
    }
    return commandLine;
}

Authserver.prototype.getServerVariables = function(varset, completionHandler){

    var serverObject = this;
    var queryvars = $.toJSON(varset);

    $.post("/queryvars", queryvars, function(response, textStatus){
        if(textStatus != "success"){
            completionHandler(false, {});
            serverObject.signalError();
            return;
        }
        
        completionHandler(true, response);
        
    });
}

Authserver.prototype.serverCalls = function(calls, completionHandler){

    var serverObject = this;
    
    $.post("/calls", $.toJSON(calls), function(response, textStatus){
        
        if(textStatus != "success"){
            completionHandler(false, {});
            serverObject.signalError();
            return;
        }
        
        completionHandler(true, response);
        
    });
}
