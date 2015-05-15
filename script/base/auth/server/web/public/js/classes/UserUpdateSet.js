function UserUpdateSet(server){

    var self = this;

    this.server = server;
    this.users = {};
    this.domains = {};
    this.numberOfUsers = 0;

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

    function updateUserArrays(user){

        function inDomain(user){
            return user.domain && user.domain.length;
        }

        self.users[user.domain + "/" + user.name] = user;
        if(inDomain(user)){
            if(self.domains[user.domain] === undefined) self.domains[user.domain] = {};
            self.domains[user.domain][user.name] = user;
       }
    }

    function updateUser(data){

        var user = self.users[data.domain + "/" + data.name];
        $.each(data, function(name, value){
            user[name] = value;
        });

        updateUserArrays(user);
    }

    function getFullUserState(domain, name, callback){
        $.get("/users/" + domain + "/" + name, function(response, textStatus){
            if(textStatus != "success"){
                callback(false);
                return;
            }
            callback(true);
            updateUser(response);
        });
    }

    function resync(callback, options){

        var addNewUser = options && options.addNewUser;

        $.get("/users", function(response, textStatus){
            if(textStatus != "success"){
               	server.signalError();
                return;
            }

            self.users = {};
            self.domains = {};

            $.each(response, function(){
                $.each(this, function(){

                    var id = this.domain + "/" + this.name;

                    if(!self.users[id]){

                        if(addNewUser){
                            self.users[id] = new User(self.server);
                            self.users[id].id = id;
                            self.numberOfUsers++;
                        }
                        else{
                            return;
                        }
                    }

                    updateUser(this);
                });
            });

            callback();
        });
    }


    function event_handler(name, func){
        eventService.addListener(name, func);
    }

    event_handler("adduser", function(name, desc, pubkey, priv){

        var user = new User(self.server);
        self.users[name + "/" + desc] = user;

        self.numberOfUsers++;

        getFullUserState(desc, name, function(success){
            if(!success) return;
            callListeners(listeners.connect, self.users[name + "/" + desc]);
        });
    });

    event_handler("deleteuser", function(name, desc){

        var user = self.users[desc + "/" + name];

        self.numberOfUsers--;

        updateUserArrays(user);
        delete self.users[desc + "/" + name];

        callListeners(listeners.disconnect, user);
    });


    resync(function(){
        callListeners(listeners.ready);
    },{
        addNewUser:true
    });

    eventService.startListening();
}
