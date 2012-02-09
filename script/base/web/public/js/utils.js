/*
    Copyright (C) 2010 Graham Daws
*/

function HtmlTable(){
    
    var table = document.createElement("table");
    table.cellSpacing = 0;
    table.cellPadding = 2;
    
    var columns = [];
    var sortPriority = [];
    
    var thead = document.createElement("thead");
    table.appendChild(thead);
    
    var tbody = document.createElement("tbody");
    table.appendChild(tbody);
        
    this.columns = function(fields, sortByFields){
        
        columns = fields;
        
        var tr = document.createElement("tr");
        for(var i = 0; i < columns.length; i++){
            var th = document.createElement("th");
            th.appendChild(document.createTextNode(columns[i].label));
            tr.appendChild(th);
        }
        thead.appendChild(tr);
        
        sortPriority = sortByFields || [];
    }
    
    function insertRow(tr){
        var trCollection = table.getElementsByTagName("tr");
        if(!sortPriority[0]){
            table.appendChild(tr);
            return;
        }
        var sortKey = sortPriority[0].key;
        var sortOrder = sortPriority[0].order;
        var key = tr.HtmlTableClassData[sortKey];
        for(var i = 0; i < trCollection.length; i++){
            if(!trCollection[i].HtmlTableClassData) continue;
            var foundPosition = sortOrder(key, trCollection[i].HtmlTableClassData[sortKey]);
            if(foundPosition == 1){
                trCollection[i].parentNode.insertBefore(tr, trCollection[i]);
                return;
            }
            else if(foundPosition == 0)
            {
                for(var j = 1; j < sortPriority.length; j++){
                    var altSortKey = sortPriority[j].key;
                    var foundPosition = sortPriority[j].order(tr.HtmlTableClassData[altSortKey], trCollection[i].HtmlTableClassData[altSortKey]);
                    if(foundPosition == 1){
                        trCollection[i].parentNode.insertBefore(tr, trCollection[i]);
                        return;
                    }
                    else if(foundPosition == -1 && (i + 1 < sortPriority.length)){
                        trCollection[i].parentNode.insertBefore(tr, trCollection[i + 1]);
                        return;
                    }
                }
            }
        }
        tbody.appendChild(tr);
    }
    
    this.row = function(data, className){

        var rowInterface = {};
        rowInterface.field = {};

        var tr = document.createElement("tr");
        tr.className = className;
        tr.HtmlTableClassData = data;
        rowInterface.tableRowElement = tr;

        rowInterface.remove = function(){
            tr.parentNode.removeChild(tr);
        }

        rowInterface.update = function(newData){
            tr.parentNode.removeChild(tr);
            for(var key in newData){
                data[key] = newData[key];
            }
            tr.HtmlTableClassData = data;
            var tdCollection = tr.getElementsByTagName("td");
            for(var i = 0; i < columns.length; i++){
                if(!columns[i].key) continue;
                tdCollection[i].textContent = data[columns[i].key];
            }
            insertRow(tr);
        }
        
        rowInterface.getRowElement = function(){
            return tr;
        }

        for(var i = 0; i < columns.length; i++){
            var td = document.createElement("td");
            var key = columns[i].key;
            td.className = columns[i].className || "";
            if(key){
                td.appendChild(document.createTextNode(data[key]));
                tr.appendChild(td);
            }else{
                var cellFunction = columns[i].cellFunction;
                if(cellFunction){
                    td.appendChild(cellFunction(data));
                    tr.appendChild(td);
                }
            }
        }
        
        insertRow(tr);
        return rowInterface;
    }

    this.attachTo = function(parent){
        parent.appendChild(table);
    }
    
    this.getTableElement = function(){
        return table;
    }
}
function descendingOrder(a, b){
    if(a==b) return 0;
    if(a > b) return 1;
    else return -1;
}

function ascendingOrder(a, b){
    if(a==b) return 0;
    if(a < b) return 1;
    else return -1;
}

function EventDispatcher(publicInterface){
    var self = this;
    var listeners = {};
    this.addListener = function(eventName, eventHandler){
        if(listeners[eventName] === undefined) listeners[eventName] = [];
        listeners[eventName].push(eventHandler);
    }
    this.signalEvent = function(){
        var arrayOfListeners = listeners[arguments[0]];
        if(!arrayOfListeners) return;
        var callListenersArguments = Array.prototype.slice.call(arguments, 1);
        $.each(arrayOfListeners, function(){
            this.apply(publicInterface, callListenersArguments);
        });
    }
    this.emptyListeners = function(eventName){
        if(eventName){
            listeners[eventName] = null;
        }
        else{
            listeners = {};
        }
    }
    if(publicInterface){
        publicInterface.addListener = function(eventName, eventHandler){
            self.addListener(eventName, eventHandler);
        }
    }
}

function isEmptyObject(object){
    for(var x in object){return false;}
    return true;
}

function Timer(delay, func){
	var started = false;
	var intervalId;
	this.start = function(){
		if(started) return;
		intervalId = window.setInterval(func, delay);
		started = true;
	}
	this.stop = function(){
		if(!started) return;
		window.clearInterval(intervalId);
		started = false;
	}
}

function leadingZero(x){
    if(x < 10 && x > -1) return "0" + x;
    else return x;
}

