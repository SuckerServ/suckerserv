$(document).ready(function()
{
        $("#hopstats,#hopstats1").tablesorter({
                        headers:
                        {
                                0 : { sorter: "text" },
                                1 : { sorter: "text" },
                                2 : { sorter: "digit" },
                                3 : { sorter: "digit" },
                                4 : { sorter: "digit" },
                                5 : { sorter: "digit" },
                                6 : { sorter: "digit" },
                                7 : { sorter: "digit" },
                                8 : { sorter: "digit" },
                                9 : { sorter: "digit" },
                                10 : { sorter: "digit" },
                                11 : { sorter: "digit" }
                        },

        });
});

$(function() {
  var theTable = $('table.tablesorter')

  theTable.find("tbody > tr").find("td:eq(1)").mousedown(function(){
    $(this).prev().find(":checkbox").click()
  });

  $("#filter").keyup(function() {
    $.uiTableFilter( theTable, this.value );
  })

  $('#filter-form').submit(function(){
    theTable.find("tbody > tr:visible > td:eq(1)").mousedown();
    return false;
  }).focus();

});


// Copyright 2006-2007 javascript-array.com

var timeout     = 500;
var closetimer  = 0;
var ddmenuitem  = 0;

// open hidden layer
function mopen(id)
{       
        // cancel close timer
        mcancelclosetime();

        // close old layer
        if(ddmenuitem) ddmenuitem.style.visibility = 'hidden';

        // get new layer and show it
        ddmenuitem = document.getElementById(id);
        ddmenuitem.style.visibility = 'visible';

}
// close showed layer
function mclose()
{
        if(ddmenuitem) ddmenuitem.style.visibility = 'hidden';
}

// go close timer
function mclosetime()
{
        closetimer = window.setTimeout(mclose, timeout);
}

// cancel close timer
function mcancelclosetime()
{
        if(closetimer)
        {
                window.clearTimeout(closetimer);
                closetimer = null;
        }
}

// close layer when click-out
document.onclick = mclose; 

