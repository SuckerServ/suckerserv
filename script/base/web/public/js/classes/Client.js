
function Client(server){

    this.serverObject = server;
    
    this.cn = -1;
    this.sessionid = -1;
    this.ip = "";
    
    this.name = "";
    this.team = "";
    
    this.status = "";
    this.priv = "";
    this.ping = 0;
    this.is_bot = false;
    
    this.frags = 0;
    this.deaths = 0;
    this.suicides = 0;
    this.teamkills = 0;
    this.shots = 0;
    this.misses = 0;
    this.accuracy = 0;
}

Client.prototype.kick = function(bantime, reason){
    var admin = this.serverObject.web_admin_username;
    bantime = bantime || -1;
    reason = reason || "";
    this.serverObject.executeCommand(this.serverObject.makeCommand("kick", this.cn, bantime, admin, reason), function(){});
}

Client.prototype.freeze = function(){
    this.serverObject.executeCommand(this.serverObject.makeCommand("player_freeze", this.cn));
}

Client.prototype.slay = function(){
    this.serverObject.executeCommand(this.serverObject.makeCommand("player_slay", this.cn));
}
