$(document).ready(function(){
    setTimeout(initWebAdmin, 1);
});

function initWebAdmin(){
        
    $("#nav a").fancybox({
        type:"iframe",
        autoDimensions:false
    });
    
    var server = new Authserver;
    
    server.addListener("unauthorized", function(){
        location.href = "/login?return=" + document.location;
    });
    
    server.addListener("lost-connection", function(){
        alert("Not connected");
        $("input").attr("disabled", true);
    });
    
    createLoadingScreen(server);
    
    server.ready(function(){
        
        installWidget("#command-links", createCommandLinks,    server);
        installWidget("#command-shell", createCommandShell,    server);
        installWidget("#netstats",      createNetstatsView,    server);
        installWidget("#serverinfo",    createServerInfoView,  server);
        installWidget("#user-command-links", createUserCommandLinks,    server);
        
        createUserTablesManager({
            "users" : document.getElementById("users")
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

function createUserTablesManager(containerElements, server){
    
    var spectatorsTable = null;
    var singlePlayersTables = null;
    var domains = {};
    var inTeammode = true;
    
    function updateClasses(user){
        $(user.guiTableRow.getRowElement())
            .removeClass("none")
            .removeClass("master")
            .removeClass("admin")
            .removeClass("dead")
            .removeClass("editing")
            .removeClass("spectator")
            .removeClass("alive")
            .addClass(user.priv)
    }
    
    function updateTableRow(user){
        if(!user.guiTableRow) return;
        user.guiTableRow.update(user);
        updateClasses(user);
    }
    
    function checkForEmptyTables(){
        $.each(domains, function(name){
            if(isEmptyObject(server.users.domains[name])){
                if(domains[name]){
                    domains[name].destroy();
                    delete domains[name];
                }
            }
        });
    }
    
    function removeTableRow(user){
        if(!user.guiTableRow) return;
        user.guiTableRow.remove();
        delete user.guiTableRow;
        checkForEmptyTables();
    }
    
    function addUserTableRow(user){
    
        if(!containerElements.users) return;
        
//        if(inTeammode){
            if(!domains[user.domain]){
                
                var table = new UsersTable(containerElements.users);
                domains[user.domain] = table;
                
                var heading = document.createElement("caption");
                heading.className = "playertable-heading";
                $(heading).html("<span class=\"playertable-teamname\"> " + user.domain + "</span>:&nbsp;<span class=\"playertable-teamscore\">0</span>");
                $(table.getTableElement()).prepend(heading);
                
/*                if(client.team == "good"){
                    $(heading).addClass("playertable-heading-team_good");
                }
                else if(client.team == "evil"){
                    $(heading).addClass("playertable-heading-team_evil");
                }
*/
                
//                updateTeamScore();
            }
            domains[user.domain].addUser(user);
            updateClasses(user);
    }
    
    function addUsers(users){
        $.each(users, function(){
            addUserTableRow(this);
        });
    }
    
/*    function updateTeamScore(team, score){
        $(".playertable-teamscore", teams[team].getTableElement()).text(score);
    }
*/
//function updateTeamScore(team, score){ 
//           $(".playertable-teamscore", domains[domain].getTableElement()).text(server.clients.numberOfClients);
//}
/*    var events = [
        "rename", "spawn", "privilege", "suicide"
    ];
    
    for(var i = 0; i < events.length; i++){
        server.users.addListener(events[i], updateTableRow);
    }
*/
    
    server.users.addListener("adduser", function(user){
        addUserTableRow(user);
    });
    
    server.users.addListener("deleteuser", removeTableRow);
    
/*    server.clients.addListener("reteam", function(client){
        removeTableRow(client);
        addUserTableRow(client);
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
            addUserTableRow(client);
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
        
        addUsers(server.clients.players);
    });
    
    server.teams.addListener("scoreupdate", function(team, score){
        updateTeamScore(team, score);
    });
    
    server.teams.addListener("scoreflag", function(cn, team, score){
        updateTeamScore(team, score);
    });
    
*/
    addUsers(server.users.users);
    
/*    $.each(server.clients.spectators, function(){
        addSpectatorTableRow(this);
    });
*/
}

function createUserControlLinks(user){
    var deluser= document.createElement("a");
    deluser.className = "kick-button";
    deluser.href="#";
    deluser.title="Delete";
    deluser.onclick = function(){
        var yes = confirm("Are you sure you want to delete " + user.name + "?");
        if(yes){
            user.deluser();
        }
        return false;
    }
    return deluser;
}

function createUserCommandLinks(parent, server){
    function addUser(){
        var domain = prompt("Domain");
        if(!domain) return;

        var name = prompt("User name");
        if(!name) return;

        var pubkey = prompt("Public key");
        if(!pubkey) return;

        var priv = prompt("Privilege");
        if(!priv) return;

        server.executeCommand(server.makeCommand("add_user", name, domain, pubkey, priv));
    }

    function addDomain(){
        var domain = prompt("Domain name");
        if(!domain) return;

        var case_insensitive = prompt("Case insensitive: 0 or 1");
        if(!case_insensitive) return;

        server.executeCommand(server.makeCommand("add_domain", domain, case_insensitive));
    }

    var userCommands = [
        {label:"Add user", _function:addUser},
        {label:"Add domain", _function:addDomain}
    ];

    function createLinks(commands, groupTitle){

       	var container = document.createElement("div");
        container.className = "user-command-links-group";

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

    createLinks(userCommands, "Manage users");
}


function UsersTable(parent){

    var table = new HtmlTable();
    table.columns([
        {label:"Name",          key:"name"},
        {label:"Pubkey",          key:"pubkey"},
        {label:"Privileges",         key:"priv"},
        {label:"", cellFunction: createUserControlLinks, className:"player-control-links"}
    ], [{key:"name", order: descendingOrder}]);

    table.attachTo(parent);

    this.getParentElement = function(){
        return parent;
    }

    this.getTableElement = function(){
        return table.getTableElement();
    }

    this.addUser = function(user){
        user.guiTableRow = table.row(user);
    }

    this.removeUser = function(user){
        user.guiTableRow.remove();
    }

    this.destroy = function(){
        $(table.getTableElement()).remove();
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
        $.get("/netstats", function(response, textStatus){
                
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
    
    var serverName = document.createElement("span");
    var localAddress = document.createElement("span");
    
    parent.appendChild(serverName);
    parent.appendChild(localAddress);
    
    $(serverName).text(server.servername);
    $(localAddress).text(server.serverip + ":" + server.serverport);
    
    function updateServerName(){
        $(serverName).text(server.servername);
    }
    
    var var_observers = {
        servername : updateServerName
    };
    
    server.addListener("varchanged", function(varname){
        var func = var_observers[varname];
        if(func) func();
    });
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
    
    function changeServerName(){
        var msg = prompt("Change the server name");
        if(!msg) return;
        server.executeCommand(server.makeCommand("servername", msg));
    }
    
    var changeMessageCommands = [
        {label:"Server Name", _function:changeServerName}
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
}
