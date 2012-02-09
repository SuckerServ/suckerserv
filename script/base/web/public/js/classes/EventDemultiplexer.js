
function EventDemultiplexer(){
    
    var listeners = {};
    var continueListening = true;
    
    this.addListener = function(eventName, eventHandler){
        if(listeners[eventName] === undefined) listeners[eventName] = [];
        listeners[eventName].push(eventHandler);
    }
    
    function createListenerResource(callback){
        var eventsList = "[";
        var count = 0;
        $.each(listeners, function(eventName){
            if(count++ > 0) eventsList += ", ";
            eventsList += "\"" + eventName + "\"";
        });
        eventsList += "]";
        $.post("/listener", eventsList, function(response, textStatus){
            if(textStatus != "success"){
                callback(false);
                return;
            }
            callback(true, response.listenerURI);
        }, "json");
    }
    
    this.startListening = function(errorCallback){
        createListenerResource(function(success, listenerURI){
        
            function runEventloop(){
                
                if(!continueListening) return;
                
                $.getJSON(listenerURI, function(events, textStatus){
                    
                    if(textStatus != "success"){
                        errorCallback();
                        return;
                    }
                    
                    $.each(events, function(){
                        var event = this;
                        var arrayOfListeners = listeners[event.name];
                        if(arrayOfListeners){
                            $.each(arrayOfListeners, function(){
                                var eventHandler = this;
                                eventHandler.apply(null, event.args);
                            });
                        }
                    });
                    
                    runEventloop();
                });
            }
            
            runEventloop();
        });
    }
    
    this.stopListening = function(){
        continueListening = false;
    }
}
