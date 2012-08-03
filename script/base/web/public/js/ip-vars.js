function displayList(data, parentElement){
    
    var table = new HtmlTable();
    
    $.each(data, function(key, value){
        
    })
}

$(document).ready(function(){
    
    $.getJSON("/ip-vars", function(response, textStatus){
    
         if(textStatus != "success"){
                return;
         }
         var gbans = RegExp("[\\?&]gbans=([^&#]*)").exec(window.location.href)[1];
         if(gbans=="true") gbans=true; else gbans=false;

         $.each(response, function(key, value){
            if(value.is_gban==gbans) {
                var h2 = document.createElement("h2");
                $(h2).text(key);
                document.body.appendChild(h2);
                
                var vartable = new HtmlTable();
                
                vartable.columns([
                    {key:"name", label:"Name"},
                    {key:"value", label:"Value"},
                ]);
            
                $.each(value, function(key, value){
                    vartable.row({name:key, value:value});
                });
            
                vartable.attachTo(document.body);
           }
         });

    });
});

