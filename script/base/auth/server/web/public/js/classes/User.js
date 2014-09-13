function User(server){

    this.serverObject = server;

    this.id = 0;
    this.name = "";

    this.pubkey = "";
    this.domain = "";
    this.priv = "";
}

User.prototype.deluser = function(){
    this.serverObject.executeCommand(this.serverObject.makeCommand("del_user", this.name, this.domain), function(){});
}
