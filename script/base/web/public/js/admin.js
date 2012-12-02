$(document).ready(function(){
    setTimeout(initWebAdmin, 1);
});

function initWebAdmin(){
        
    $("#nav a").fancybox({
        type:"iframe",
        autoDimensions:false
    });
    
    var server = new Server;
    
    server.addListener("unauthorized", function(){
        location.href = "/login?return=" + document.location;
    });
    
    server.addListener("lost-connection", function(){
        alert("Not connected");
        $("input").attr("disabled", true);
    });
    
    createLoadingScreen(server);
    
    server.ready(function(){
        
        installWidget("#chat-shell",    createChatShell,       server);
        installWidget("#command-links", createCommandLinks,    server);
        installWidget("#command-shell", createCommandShell,    server);
        installWidget("#gameinfo",      createGameinfoView,    server);
        installWidget("#netstats",      createNetstatsView,    server);
        installWidget("#serverinfo",    createServerInfoView,  server);
        
        createClientTablesManager({
            "spectators" : document.getElementById("spectators"),
            "players"    : document.getElementById("players")
        }, server);
        
        document.title = server.servername + " - " + document.title;
        
        $("#loading").remove();
    });
}

function installWidget(pattern, createFunction, server){
    $(pattern).each(function(){createFunction(this, server);});
}

function createLoadingScreen(server){
    
    var div = document.createElement("div");
    div.id = "loading";
    div.style.backgroundColor = "#ffffff";
    div.style.position = "absolute";
    div.style.left = "0px";
    div.style.top = "0px";
    div.style.width = "100%";
    div.style.height = "100%";
    document.body.appendChild(div);
    
    var icon = new Image();
    icon.alt = "Loading...";
    icon.src = "/static/presentation/loading.gif";
    
    var p = document.createElement("p");
    $(p).text("Waiting for data snapshots...");
    var ul = document.createElement("ul");
    
    server.addListener("loading",function(loadList){
        $(ul).empty();
        $.each(loadList, function(name, value){
            if(value){
                var li = document.createElement("li");
                $(li).text(value);
                ul.appendChild(li);
            }
        });
        if(isEmptyObject(loadList)){
            $(p).text("Waiting for the GUI to load..");
        }
    });
    
    div.appendChild(icon);
    div.appendChild(p);
    div.appendChild(ul);
}

function createChatShell(parent, server){
    
    $(parent).addClass("text-shell");
    
    var heading = document.createElement("div");
    heading.id = "chat-shell-heading";
    heading.className = "text-shell-heading";
    $(heading).text("Chat");
    
    var outputContainer = document.createElement("div");
    outputContainer.id = "chat-shell-output";
    outputContainer.className="text-shell-output";
    
    var outputContainer = document.createElement("div");
    outputContainer.id = "chat-shell-output";
    outputContainer.className="text-shell-output";
    
    var output = document.createElement("div");
    output.id = "chat-shell-output-padding";
    output.className = "text-shell-output-padding";
    outputContainer.appendChild(output);
    
    var input = document.createElement("input");
    input.id = "chat-shell-input";
    input.className= "text-shell-input";
    input.type = "text";
    
    parent.appendChild(heading);
    parent.appendChild(outputContainer);
    parent.appendChild(input);
    
    function addTextMessage(playerName, text, className){
    
        var message = document.createElement("p");
        message.className = "chat-shell-message";
        
        var name = document.createElement("span");
        name.className = "chat-shell-name";
        if(className){
            name.className += " " + className;
        }
        name.appendChild(document.createTextNode(playerName));
        
        var messageText = document.createElement("span");
        messageText.className = "chat-shell-text";
        messageText.appendChild(document.createTextNode(text));
        
        var clear = document.createElement("div");
        clear.className = "clear";
        
        message.appendChild(name);
        message.appendChild(messageText);
        
        output.appendChild(message);
        output.appendChild(clear);
        
        $(outputContainer).scrollTop(output.scrollHeight);
    }
    
    server.addListener("text", function(client, message){
        var chatClass = null;
        if(client.priv == "admin") chatClass = "chat-shell-admin";
        else if(client.priv == "master") chatClass = "chat-shell-master";
        addTextMessage(client.name + "(" + client.cn + ")", message, chatClass);
    });

    server.addListener("sayteam", function(client, message){
        var chatClass = null;
        if(client.priv == "admin") chatClass = "chat-shell-team-admin";
        else if(client.priv == "master") chatClass = "chat-shell-team-master";
        else chatClass = "chat-shell-team";
        addTextMessage(client.name + "(" + client.cn + ")", message, chatClass);
    });
    
    server.addListener("admin-message", function(admin, message){
        addTextMessage(admin, message, "chat-shell-admin");
    });
    
    $(input).keypress(function(e){
        switch(e.which){
            case 13: // enter
            {
                if(this.value.length == 0) return;
                server.executeCommand(server.makeCommand("console", server.web_admin_session_username, this.value), function(){
                    input.value = "";
                });
            }
        }
    });
}

function createCommandShell(parent, server){
    
    $(parent).addClass("text-shell");
    
    var heading = document.createElement("div");
    heading.id = "command-shell-heading";
    $(heading).text("Command Shell");
    
    var outputContainer = document.createElement("div");
    outputContainer.id = "command-shell-output";
    
    var output = document.createElement("div");
    output.id = "command-shell-output-padding";
    outputContainer.appendChild(output);
    
    var input = document.createElement("input");
    input.type = "text";
    
    parent.appendChild(heading);
    parent.appendChild(outputContainer);
    parent.appendChild(input);
    
    $(input).keypress(function(e){
    
        switch(e.which){
            case 13: // enter
            {
                if(this.value.length == 0) return;
                
                server.executeCommand(this.value, function(status, responseBody){
                    
                    var echo = document.createElement("div");
                    echo.className = "echo-output";
                    echo.appendChild(document.createTextNode(input.value));
                    output.appendChild(echo);
                    
                    var serverResponse = document.createElement("div");
                    
                    if(status){
                        serverResponse.className = "normal-output";
                        if(!responseBody.length) serverResponse.className = "normal-no-output";
                    }
                    else{
                        serverResponse.className = "error-output";
                    }
                    
                    serverResponse.appendChild(document.createTextNode(responseBody));
                    output.appendChild(serverResponse);
                    $(outputContainer).scrollTop(output.scrollHeight);
                    input.value = "";
                });
            
                break;
            }
        }
        
    });
}

function createClientTablesManager(containerElements, server){
    
    var spectatorsTable = null;
    var singlePlayersTables = null;
    var teams = {};
    var inTeammode = false;
    
    function updatePredicates(){
        inTeammode = GamemodeInfo[server.game.gamemode].teams;
    }
    
    updatePredicates();
    
    function updateClasses(client){
        $(client.guiTableRow.getRowElement())
            .removeClass("none")
            .removeClass("master")
            .removeClass("admin")
            .removeClass("dead")
            .removeClass("editing")
            .removeClass("spectator")
            .removeClass("alive")
            .addClass(client.priv)
            .addClass(client.status);
    }
    
    function updateTableRow(client){
        if(!client.guiTableRow) return;
        client.guiTableRow.update(client);
        updateClasses(client);
    }
    
    function checkForEmptyTables(){
        if(spectatorsTable && isEmptyObject(server.clients.spectators)){
            spectatorsTable.destroy();
            spectatorsTable = null;
        }
        if(inTeammode){
            $.each(teams, function(name){
                if(isEmptyObject(server.clients.teams[name])){
                    if(teams[name]){
                        teams[name].destroy();
                        delete teams[name];
                    }
                }
            });
        }else{
            if(singlePlayersTables && isEmptyObject(server.clients.players)){
                singlePlayersTables.destroy();
                singlePlayersTables = null;
            }
        }
    }
    
    function removeTableRow(client){
        if(!client.guiTableRow) return;
        client.guiTableRow.remove();
        delete client.guiTableRow;
        checkForEmptyTables();
    }
    
    function addPlayerTableRow(client){
    
        if(!containerElements.players) return;
        
        if(inTeammode){
            if(!teams[client.team]){
                
                var table = new PlayersTable(containerElements.players);
                teams[client.team] = table;
                
                var heading = document.createElement("caption");
                heading.className = "playertable-heading";
                $(heading).html("<span class=\"playertable-teamname\"> " + client.team + "</span>:&nbsp;<span class=\"playertable-teamscore\">0</span>");
                $(table.getTableElement()).prepend(heading);
                
                if(client.team == "good"){
                    $(heading).addClass("playertable-heading-team_good");
                }
                else if(client.team == "evil"){
                    $(heading).addClass("playertable-heading-team_evil");
                }
                
                updateTeamScore(client.team, server.teams.score[client.team]);
            }
            teams[client.team].addClient(client);
            updateClasses(client);
        }else{
            if(!singlePlayersTables){
                singlePlayersTables = new PlayersTable(containerElements.players);
            }
            singlePlayersTables.addClient(client);
            updateClasses(client);
        }
    }
    
    function addSpectatorTableRow(client){
        
        if(!containerElements.spectators) return;
        
        if(!spectatorsTable){
            spectatorsTable = new SpectatorsTable(containerElements.spectators);
        }
        
        spectatorsTable.addClient(client);
        updateClasses(client);
    }
    
    function addPlayers(players){
        $.each(players, function(){
            addPlayerTableRow(this);
        });
    }
    
    function updateTeamScore(team, score){
        $(".playertable-teamscore", teams[team].getTableElement()).text(score);
    }
    
    var events = [
        "rename", "spawn", "privilege", "suicide"
    ];
    
    for(var i = 0; i < events.length; i++){
        server.clients.addListener(events[i], updateTableRow);
    }
    
    server.clients.addListener("connect", function(client){
        if(client.status == "spectator"){
            addSpectatorTableRow(client);
        }
        else{
            addPlayerTableRow(client);
        }
    });
    
    server.clients.addListener("disconnect", removeTableRow);
    
    server.clients.addListener("reteam", function(client){
        removeTableRow(client);
        addPlayerTableRow(client);
    });
    
    server.clients.addListener("frag", function(actor, target){
        updateTableRow(actor);
        updateTableRow(target);
    });
    
    server.clients.addListener("spectator", function(client, joined){
        removeTableRow(client);
        if(joined){
            addSpectatorTableRow(client);
        }else{
            addPlayerTableRow(client);
        }
    });
    
    server.addListener("mapchange", function(){
        
        if(singlePlayersTables){
            singlePlayersTables.destroy();
            singlePlayersTables = null;
        }
        
        $.each(teams, function(name){
            this.destroy();
        });
        teams = {};
        
        updatePredicates();
        
        addPlayers(server.clients.players);
    });
    
    server.teams.addListener("scoreupdate", function(team, score){
        updateTeamScore(team, score);
    });
    
    server.teams.addListener("scoreflag", function(cn, team, score){
        updateTeamScore(team, score);
    });
    
    addPlayers(server.clients.players);
    
    $.each(server.clients.spectators, function(){
        addSpectatorTableRow(this);
    });
}

function createPlayerControlLinks(client){
    var kick = document.createElement("a");
    kick.className = "kick-button";
    kick.href="#";
    kick.title="Kick";
    kick.onclick = function(){
        var yes = confirm("Are you sure you want to kick " + client.name + "(" + client.cn + ")");
        if(yes){
            client.kick(1440);
        }
        return false;
    }
    return kick;
}

function PlayersTable(parent){
    
    var table = new HtmlTable();
    table.columns([
        {label:"CN",            key:"cn"},
        {label:"IP Addr",       key:"ip"},
        {label:"Name",          key:"name"},
        {label:"Ping",          key:"ping"},
        {label:"Frags",         key:"frags"},
        {label:"Deaths",        key:"deaths"},
        {label:"Accuracy",        key:"accuracy"},
        {label:"Teamkills",     key:"teamkills"},
        {label:"", cellFunction: createPlayerControlLinks, className:"player-control-links"}
    ], [{key:"frags", order: descendingOrder}, {key:"deaths", order: ascendingOrder}]);
    
    table.attachTo(parent);
    
    this.getParentElement = function(){
        return parent;
    }
    
    this.getTableElement = function(){
        return table.getTableElement();
    }
    
    this.addClient = function(client){
        client.guiTableRow = table.row(client);
    }
    
    this.removeClient = function(client){
        client.guiTableRow.remove();
    }
    
    this.destroy = function(){
        $(table.getTableElement()).remove();
    }
}

function SpectatorsTable(parent){
    
    var table = new HtmlTable();
    table.columns([
        {label:"CN",        key:"cn"},
        {label:"IP Addr",   key:"ip"},
        {label:"Name",      key:"name"},
        {label:"Ping",      key:"ping"},
        {label:"", cellFunction: createPlayerControlLinks, className:"player-control-links"}
    ]);
    table.attachTo(parent);
    
    var heading = document.createElement("caption");
    heading.className = "playertable-heading";
    $(heading).text("Spectators");
    $(table.getTableElement()).prepend(heading);
    
    this.getParentElement = function(){
        return parent;
    }
    
    this.addClient = function(client){
        client.guiTableRow = table.row(client);
    }
    
    this.removeClient = function(client){
        client.guiTableRow.remove();
    }
    
    this.destroy = function(){
        $(parent).empty();
    }
}

function createGameinfoView(parent, server){
    
    var map = document.createElement("span");
    var gamemode = document.createElement("span");
    var timeleft = document.createElement("span");
    
    map.id = "gameinfo-map";
    gamemode.id = "gameinfo-gamemode";
    timeleft.id = "gameinfo-timeleft";
    
    $(map).text(server.game.map);
    $(gamemode).text(server.game.gamemode);
    var countdownTimer = new Timer(1000, updateTimeleft);
    countdownTimer.start();
    
    parent.appendChild(gamemode);
    parent.appendChild(document.createTextNode(": "));
    parent.appendChild(map);
    parent.appendChild(document.createTextNode(", "));
    parent.appendChild(timeleft);
    
    server.addListener("mapchange", function(mapName, gamemodeName){
        $(map).text(mapName);
        $(gamemode).text(gamemodeName);
    });
    
    function updateTimeleft(){
        
        var seconds = server.game.getTotalSecondsLeft();

        if(seconds === undefined){
            $(timeleft).text("unknown");
            return;
        }
        
        if(seconds == 0){
            $(timeleft).text("intermission");
        }
        else{
            $(timeleft).text(Math.floor(seconds / 60) + ":" + leadingZero(Math.floor(seconds % 60)));
        }
    }
    
}

function createNetstatsView(parent, server){
    
    var download_rate = document.createElement("span");
    download_rate.id = "netstats-download-rate";
    download_rate.title = "download rate";
    
    var download_rate_chart = document.createElement("span");
    var download_rate_points = [];
    
    var upload_rate = document.createElement("span");
    upload_rate.id = "netstats-upload-rate";
    upload_rate.title = "upload rate";
    
    var upload_rate_chart = document.createElement("span");
    var upload_rate_points = [];
    
    parent.appendChild(download_rate);
    parent.appendChild(download_rate_chart);
    
    parent.appendChild(upload_rate);
    parent.appendChild(upload_rate_chart);
    
    var pingStatsDiv = document.createElement("span");
    
    var minPing = document.createElement("span");
    var avgPing = document.createElement("span");
    var maxPing = document.createElement("span");
    
    pingStatsDiv.appendChild(document.createTextNode("Player pings: "));

    pingStatsDiv.appendChild(minPing);
    pingStatsDiv.appendChild(avgPing);
    pingStatsDiv.appendChild(maxPing);
    
    parent.appendChild(pingStatsDiv);
    
    var chart_options = {
        type : "line",
        width : "100px",
        lineColor: "blue"
    }
    
    function updateNetstats(){
        $.getJSON("/netstats", function(response, textStatus){
                
            if(textStatus != "success"){
                return;
            }

            var value = response.download_rate/1000;
            download_rate_points.push(value);
            if(download_rate_points.length == 13){
                download_rate_points.shift();
            }
            
            $(download_rate).text(value + "kb/sec");
            $(download_rate_chart).sparkline(download_rate_points, chart_options);
            download_rate.title = "download rate\n\nDownloaded: " + response.downloaded/1000000 + "MB";
            
            value = response.upload_rate/1000;
            upload_rate_points.push(value);
            if(upload_rate_points.length == 13){
                upload_rate_points.shift();
            }
            $(upload_rate).text(value + "kb/sec");
            $(upload_rate_chart).sparkline(upload_rate_points, chart_options);
            upload_rate.title = "upload rate\n\nUploaded: " + response.uploaded/1000000 + "MB";
            
            if(response.min_ping && response.avg_ping && response.max_ping){
                $(minPing).text("min(" + response.min_ping + ")");
                $(avgPing).text("avg(" + response.avg_ping + ")");
                $(maxPing).text("max(" + response.max_ping + ")");
                $(pingStatsDiv).show();
            }
            else{
                $(pingStatsDiv).hide();
            }
        });
    }
    
    updateNetstats();
    
    var timer = new Timer(5000, updateNetstats);
    timer.start();
}

function createServerInfoView(parent, server){
    
    var players = document.createElement("span");
    var mastermode = document.createElement("span");
    var serverName = document.createElement("span");
    var localAddress = document.createElement("span");
    
    parent.appendChild(players);
    parent.appendChild(mastermode);
    parent.appendChild(serverName);
    parent.appendChild(localAddress);
    
    $(serverName).text(server.servername);
    $(mastermode).text(mastermodeName[server.mastermode]);
    $(localAddress).text(server.serverip + ":" + server.serverport);
    
    function updateClientCount(){
        $(players).text(server.clients.numberOfClients + "/" + server.maxclients);
    }
    
    function updateServerName(){
        $(serverName).text(server.servername);
    }
    
    function updateMastermode(){
        $(mastermode).text(mastermodeName[server.mastermode]);
    }
    
    var var_observers = {
        maxclients : updateClientCount,
        servername : updateServerName,
        mastermode : updateMastermode
    };
    
    server.addListener("varchanged", function(varname){
        var func = var_observers[varname];
        if(func) func();
    });
    
    server.clients.addListener("connect", updateClientCount);
    server.clients.addListener("disconnect", updateClientCount);
    
    updateClientCount();
}

function createCommandLinks(parent, server){

    function saveCurrentConfiguration(){
        server.serverCalls([{return_id:"a", name:"saveconf_get_conf", args:[]}], function(success, response){
            if(!response.a){
                alert("An error has occurred: function not found.");
                return;
            }
            if(response.a.error){
                alert("An error has occurred: " + response.a.values[1]);
                return;
            }
            var saveconf = response.a.values[1];
            
            var output = "";
            $.each(saveconf, function(name, value){
                output += name + "\t\t\t\t " + value + "\n";
            });
            
            var ok = confirm("Configuration to be saved:\n\n" + output);
            if(ok){
                server.executeCommand("saveconf");
            }
        });
    }

    function shutdown(){
        if(!confirm("Are you sure you want to shutdown the server now?")) return;
        server.executeCommand("shutdown");
    }
    
    function restart(){
        if(!confirm("Are you sure you want to restart the server now?")) return;
        server.executeCommand("restart_now");
    }
    
    function reload(){
        server.executeCommand("reload_lua");
    }
    
    var serverCommands = [
        {label:"Save Current Configuration", _function:saveCurrentConfiguration},
        {label:"Update", _function:reload},
        {label:"Restart", _function:restart},
        {label:"Shutdown", _function:shutdown}
    ];
    
    function changeMotd(){
        var msg = prompt("Change the Message of the Day");
        if(!msg) return;
        server.executeCommand(server.makeCommand("motd", msg));
    }
    
    function changeServerName(){
        var msg = prompt("Change the server name");
        if(!msg) return;
        server.executeCommand(server.makeCommand("servername", msg));
    }
    
    var changeMessageCommands = [
        {label:"Server Name", _function:changeServerName},
        {label:"MOTD", _function:changeMotd}
    ];
    
    function setBannedIP(){
        var ip = prompt("IP address (e.g. 127.0.0.1 or 127/8)");
        if(!ip) return;
        var bantime = prompt("Ban time (value in hours)", "-1");
        if(!bantime) return;
        if(bantime > 0) bantime = bantime * 3600;
        var reason = prompt("Reason for ban");
        if(!reason) return;
        server.executeCommand(server.makeCommand("ban", ip, bantime, reason));
    }
    
    function unsetBannedIP(){
        var ip = prompt("IP address (e.g. 127.0.0.1 or 127/8");
        if(!ip) return;
        server.executeCommand(server.makeCommand("unban", ip));
    }
    
    var playerCommands = [
        {label:"Add IP ban", _function:setBannedIP},
        {label:"Remove IP ban", _function:unsetBannedIP}
    ];
    
    function createLinks(commands, groupTitle){
        
        var container = document.createElement("div");
        container.className = "command-links-group";
        
        var ul = document.createElement("ul");
        
        for(var i = 0; i < commands.length; i++){
            
            var a = document.createElement("a");
            $(a).text(commands[i].label);
            a.href = "#";
            a.onclick = commands[i]._function;

            var li = document.createElement("li");
            li.appendChild(a);
            ul.appendChild(li);
        }
        
        var heading = document.createElement("div");
        $(heading).text(groupTitle);
        container.appendChild(heading);
        container.appendChild(ul);
        
        parent.appendChild(container);
    }
    
    createLinks(serverCommands, "Server Control");
    createLinks(changeMessageCommands, "Change Server Messages");
    createLinks(playerCommands, "Players");
}


