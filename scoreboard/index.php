<?php
include("includes/geoip.inc");
include("includes/hopmod.php");

// Start session for session vars
session_start();
//Defaults
if (! isset($_SESSION['orderby']) ) { $_SESSION['orderby'] = "Frags"; }
if (! isset($_SESSION['querydate']) ) { $_SESSION['querydate'] = "month";}

// Check for any http GET activity
check_get();

// Start page benchmark
startbench();

// Pull Variables from Running Hopmod Server
serverDetails($serverhost, $serverport);
// Setup statsdb and assign it to an object.
$dbh = setup_pdo_statsdb($db);

$dat = explode(",",date('n,Y')); 
$year = date('Y');
$week = date('W');

$day_end = strtotime("today +1 day");
$week_end = strtotime("{$year}-W{$week}-7");
$month_end = mktime(23,59,59,$dat[0]+1,0,$dat[1]);
$year_end = strtotime("today");

$day_start = strtotime("today");
$week_start = strtotime("{$year}-W{$week}-1");
$month_start = mktime(0,0,0,$dat[0],1,$dat[1]);;
$year_start = strtotime("today -365 days");


$_SESSION['start_date'] = ${$_SESSION['querydate']."_start"};
$_SESSION['end_date'] = ${$_SESSION['querydate']."_end"};

$day = "";
$pager_query = "
        select COUNT(*)
        from
                (select name,
                        frags,
                        count(name) as TotalGames
                from players
                        inner join games on players.game_id=games.id
                where UNIX_TIMESTAMP(games.datetime) between ".$_SESSION['start_date']." and ".$_SESSION['end_date']."  and frags > 0 group by name) T
        where TotalGames >= ". $_SESSION['MinimumGames']."
";
?>
<html>
<head>
	<title><?php print $server_title; ?> scoreboard</title>
	<script type="text/javascript" src="js/overlib.js"><!-- overLIB (c) Erik Bosrup --></script>
	<script type="text/javascript" src="js/jquery-latest.js"></script>
	<script type="text/javascript" src="js/jquery.tablesorter.js"></script>
	<script type="text/javascript" src="js/jquery.uitablefilter.js"></script>
	<script type="text/javascript" src="js/hopstats.js"></script>
	<link rel="stylesheet" type="text/css" href="css/style.css" />
</head>
<body>
<div style="float: left"> <img src="images/hopmod.png" /> </div><br /><br /><br /><br />
<ul align=right id="sddm">
    <li><a href="#" 
        onmouseover="mopen('m1')" 
        onmouseout="mclosetime()">Ordered by <?php print "<font color='white'>". $_SESSION['orderby'] ."</font>";?> </a>
        <div id="m1" 
            onmouseover="mcancelclosetime()" 
            onmouseout="mclosetime()">
        <a href="?orderby=name">Name</a>
        <a href="?orderby=country">Country</a>
        <a href="?orderby=TotalScore">Total Score</a>
        <a href="?orderby=MostFrags">Frags Record</a>
        <a href="?orderby=TotalFrags">Total Frags</a>
        <a href="?orderby=TotalDeaths">Total Deaths</a>
        <a href="?orderby=Accuracy">Accuracy</a>
        <a href="?orderby=Kpd">Kpd</a>
        <a href="?orderby=TotalTeamkills">Total Teamkills</a>
        <a href="?orderby=TotalGames">Total Games</a>
        </div>
    </li>
</ul>
<noscript><div class="error">This page uses JavaScript for table column sorting and producing an enhanced tooltip display.</div></noscript>
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000"></div>
<h1><?php print "$server_title "; print "$month"; ?> Scoreboard</h1>

<div id="filter-panel">
<span class="filter-form">
Limit to this [ <a href="?querydate=day" <?php if ( $_SESSION['querydate'] == "day" ) { print "class=selected"; } ?>>DAY</a> | 
<a href="?querydate=week" <?php if ( $_SESSION['querydate'] == "week" ) { print "class=selected"; } ?>>WEEK</a> | 
<a href="?querydate=month" <?php if ( $_SESSION['querydate'] == "month" ) { print "class=selected"; } ?> >MONTH</a> | 
<a href="?querydate=year" <?php if ( $_SESSION['querydate'] == "year" ) { print "class=selected"; } ?>>YEAR</a> ]</span>

<span class="filter-form"><form id="filter-form">Name Filter: <input name="filter" id="filter" value="" maxlength="30" size="30" type="text"></form></span>

<div style="float: right " id="pagebar">
<?php build_pager($_GET['page'],$pager_query,100); //Generate Pager Bar ?>
</div>
</div>
<?php stats_table(); //Build stats table data ?> 
</tbody>
</table>
<?php stopbench(); //Stop and display benchmark.?>
</body>
</html>
