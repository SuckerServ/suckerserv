function UserUpdateSet(server){

    var self = this;

    this.server = server;
    this.users = {};
    this.domains = {};
    this.numberOfUsers = 0;

/*    this.getUser = function(id){
        return self.users[id];
    }
*/

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

	var id = user.id;
        self.users[id] = user;
        if(inDomain(user)){
            if(self.domains[user.domain] === undefined) self.domains[user.domain] = {};
            self.domains[user.domain][id] = user;
       }
    }

    function updateUser(id, data){

        var user = self.users[id];
        $.each(data, function(name, value){
            user[name] = value;
        });

        updateUserArrays(user);
    }

    function getFullUserState(domain, name, callback){
        $.getJSON("/users/" + domain + "/" + name, function(response, textStatus){
            if(textStatus != "success"){
                callback(false);
                return;
            }
            callback(true, response.id);
            updateUser(response.id, response);
        });
    }

    function resync(callback, options){

        var addNewUser = options && options.addNewUser;

        $.getJSON("/users", function(response, textStatus){
            if(textStatus != "success"){
               	server.signalError();
                return;
            }

            self.users = {};
            self.domains = {};

            $.each(response, function(){
                $.each(this, function(){

                    var id = this.id;

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

                    updateUser(id, this);
                });
            });

            callback();
        });
    }


    function event_handler(name, func){
        eventService.addListener(name, func);
    }

    event_handler("adduser", function(name, desc, pubkey, priv){

        self.numberOfClients++;

        getFullUserState(desc, name, function(success, id){
            if(!success) return;
            var user = new User(self.server);
            user.id = id;
            self.users[id] = user;
            callListeners(listeners.connect, self.users[id]);
        });
    });

    event_handler("deleteuser", function(name, desc){
        $.each(self.domains[desc], function(){
            if(this.name == name){
                self.numberOfClients--;
                updateUserArrays(this);
                delete self.users[this.id];
                callListeners(listeners.disconnect, client);
                return;
            }
        });
    });


    resync(function(){
        callListeners(listeners.ready);
    },{
        addNewUser:true
    });

    eventService.startListening();
}
