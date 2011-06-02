
function ClientUpdateSet(server){

    var self = this;
    
    this.server = server;
    this.clients = {};
    this.spectators = {};
    this.players = {};
    this.teams = {};
    this.numberOfClients = 0;
    
    this.getClient = function(cn){
        return self.clients[cn];
    }
    
    var eventService = new EventDemultiplexer();
    var listeners = {};
    
    this.addListener = function(eventName, eventHandler){
        if(listeners[eventName] === undefined) listeners[eventName] = [];
        listeners[eventName].push(eventHandler);
    }
    
    function callListeners(){
        var arrayOfListeners = arguments[0];
        if(!arrayOfListeners) return;
        var callListenersArguments = Array.prototype.slice.call(arguments, 1);
        $.each(arrayOfListeners, function(){
            this.apply(null, callListenersArguments);
        });
    }
    
    function updateClientArrays(client){
    
        function inTeam(client){
            return client.team && client.team.length;
        }
        
        var cn = client.cn;
        if(client.status == "spectator"){
            delete self.players[cn];
            if(inTeam(client)){
                delete self.teams[client.team][cn];
            }
            self.spectators[cn] = client;
        }
        else{
            delete self.spectators[cn];
            self.players[cn] = client;
            if(inTeam(client)){
                if(self.teams[client.team] === undefined) self.teams[client.team] = {};
                self.teams[client.team][cn] = client;
            }
        }
    }
    
    function updateClient(cn, data){
    
        var client = self.clients[cn];
        $.each(data, function(name, value){
            client[name] = value;
        });
        
        updateClientArrays(client);
    }
    
    function getFullClientState(cn, callback){
        $.getJSON("/clients/" + cn, function(response, textStatus){
            if(textStatus != "success"){
                callback(false);
                return;
            }
            updateClient(cn, response);
            callback(true);
        });
    }
    
    function resync(callback, options){
        
        var addNewClient = options && options.addNewClient;
        
        $.getJSON("/clients", function(response, textStatus){
            if(textStatus != "success"){
                server.signalError();
                return;
            }
            
            self.spectators = {};
            self.players = {};
            self.teams = {};
            
            $.each(response, function(){
                
                var cn = this.cn;
                
                if(!self.clients[cn]){
                    
                    if(addNewClient){
                        self.clients[cn] = new Client(self.server);
                        self.clients[cn].cn = cn;
                        self.clients[cn].is_bot = cn >= 128;

                        if(!self.clients[cn].is_bot){
                            self.numberOfClients++;
                        }
                    }
                    else{
                        return;
                    }
                }
                
                updateClient(cn, this);
            });
            
            callback();
        });
    }
    
    function event_handler(name, func){
        eventService.addListener(name, func);
    }
    
    event_handler("connect", function(cn){
    
        var client = new Client(self.server);
        client.cn = cn;
        self.clients[cn] = client;
        client.is_bot = cn >= 128;
        
        if(!client.is_bot){
            self.numberOfClients++;
        }
        
        getFullClientState(cn, function(success){
            if(!success) return;
            callListeners(listeners.connect, self.clients[cn]);
        });
    });
    
    event_handler("disconnect", function(cn){
    
        var client = self.clients[cn];
        client.status = "spectator";
        
        if(!client.is_bot){
            self.numberOfClients--;
        }
        
        updateClientArrays(client);
        delete self.clients[cn];
        delete self.spectators[cn];

        callListeners(listeners.disconnect, client);
    });
    
    event_handler("rename", function(cn, oldname, newname){
        self.clients[cn].name = newname;
        callListeners(listeners.rename, self.clients[cn], oldname);
    });
    
    event_handler("reteam", function(cn, oldteam, newteam){
        self.clients[cn].team = newteam;
        delete self.teams[oldteam][cn];
        updateClientArrays(self.clients[cn]);
        callListeners(listeners.reteam, self.clients[cn], oldteam, newteam);
    });
    
    event_handler("frag", function(target, actor){
        self.clients[target].deaths++;
        self.clients[target].status = "dead";
        self.clients[actor].frags++;
        if(target != actor){
            callListeners(listeners.frag, self.clients[actor], self.clients[target]);
        }
        else{
            self.clients[actor].frags--;
            callListeners(listeners.suicide, self.clients[actor]);
        }
    });
    
    event_handler("suicide", function(cn){
        self.clients[cn].suicides++;
        self.clients[cn].deaths++;
        self.clients[cn].status = "dead";
        callListeners(listeners.suicide, self.clients[cn]);
    });
    
    event_handler("spawn", function(cn){
        self.clients[cn].status = "alive";
        callListeners(listeners.spawn, self.clients[cn]);
    });
    
    event_handler("spectator", function(cn, joined){
        self.clients[cn].status = (joined ? "spectator" : "dead");
        updateClientArrays(self.clients[cn]);
        callListeners(listeners.spectator, self.clients[cn], joined);
    });
    
    event_handler("privilege", function(cn, oldpriv, newpriv){
        var privNames = ["none", "master", "admin"];
        self.clients[cn].priv = privNames[newpriv];
        callListeners(listeners.privilege, self.clients[cn]);
    });
    
    event_handler("mapchange", function(map, gamemode){
        resync(function(){
            callListeners(listeners.mapchange, map, gamemode);
        });
    });
    
    resync(function(){
        callListeners(listeners.ready);
    },{
        addNewClient:true
    });
    
    eventService.startListening();
}

