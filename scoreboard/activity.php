<?php
include("includes/geoip.inc");
include("includes/hopmod.php");

// Start session for session vars
session_start();
// Start page benchmark
startbench();

// Check for any http GET activity
check_get();
if (! isset($_SESSION['days'])) { $_SESSION['days'] = 0;}
if ($_GET['select_day'] == "next") { $_SESSION['days'] = ($_SESSION['days'] + 1);header("location: activity.php");}
if ($_GET['select_day'] == "previous") { $_SESSION['days'] = ($_SESSION['days'] - 1);header("location: activity.php");}

$start_date = strtotime(($_SESSION['days']-1)." days");
$start_date = date("d F Y", $start_date);
$start_date = strtotime("$start_date");
$end_date = strtotime("+23 hours 59 minutes 59 seconds", $start_date); 


$day_games = "
select games.id as id,datetime,gamemode,mapname,duration,players,servername
        from games
        where UNIX_TIMESTAMP(games.datetime) between '$start_date' and '$end_date' and mapname != '' and gamemode != ''  and players > '1' order by datetime desc 
        limit ".$_SESSION['paging'].",$rows_per_page ;
";
        $sql = "
select *
from
        (select name,
                ipaddr,
                sum(score) as TotalScored,
                sum(teamkills) as TotalTeamkills,
                max(frags) as MostFrags,
                sum(frags) as TotalFrags,
                sum(deaths) as TotalDeaths,
                count(name) as TotalGames,
                round((0.0+sum(hits))/(sum(hits)+sum(misses))*100) as Accuracy,
                round((0.0+sum(frags))/sum(deaths),2) as Kpd
        from players
                inner join games on players.game_id=games.id
        where UNIX_TIMESTAMP(games.datetime) between $start_date and $end_date and mapname != '' group by name order by Kpd desc) T
	limit ".$_SESSION['paging'].",$rows_per_page ;

";
        $players_pager_query = "
select count(*)
from
        (select name
        from players
                inner join games on players.game_id=games.id
        where UNIX_TIMESTAMP(games.datetime) between $start_date and $end_date and games.mapname != '' group by name) T

";

        $games_pager_query = "
select count(*)
        from games
        where UNIX_TIMESTAMP(games.datetime) between '$start_date' and '$end_date' and mapname != '' and gamemode != ''  order by datetime desc
";

serverDetails($serverhost, $serverport); //Get the server configuration and name.

// Setup statsdb and assign it to an object.
$dbh = setup_pdo_statsdb($db);
$count_day_games = count_rows("
select count(*)
        from games
        where UNIX_TIMESTAMP(games.datetime) between '$start_date' and '$end_date' and mapname != '' and gamemode != ''  order by datetime desc 
");
$player_count = count_rows("
select count(*) 
from
        (select name
        from players
                inner join games on players.game_id=games.id
        where UNIX_TIMESTAMP(games.datetime) between $start_date and $end_date and mapname != '' group by name ) T

");

?>

<html>
<head>
	<title><?php print $server_title; ?> Daily Activity</title>
	<script type="text/javascript" src="js/overlib.js"><!-- overLIB (c) Erik Bosrup --></script>
	<script type="text/javascript" src="js/jquery-latest.js"></script>
	<script type="text/javascript" src="js/jquery.tablesorter.js"></script>
	<script type="text/javascript" src="js/jquery.uitablefilter.js"></script>
	<script type="text/javascript" src="js/hopstats.js"></script>
	<link rel="stylesheet" type="text/css" href="css/style.css" />
</head>
<body>
<br />
<h1>Daily Activity for <span style="font-style:italic; font-size:1.1em"><?php print date(" jS M Y",$start_date); ?></span></h1>
<div id=container>
	<div id="pagebar">
		<a href="activity.php?select_day=previous">&#171; Previous day</a>
		<a href="activity.php?select_day=next">Next day &#187;</a>
	</div>
	<div id="leftColumn">
    <h2> Games (<?php print $count_day_games; ?>)</h2><br>
    <?php build_pager($_GET['page'],$games_pager_query); //Generate Pager Bar ?>
	<?php match_player_table($dbh->query($day_games)); //Build game table data ?>

	
	</div>
	<div id="rightColumn">
		<h2> Players (<?php print $player_count; ?>)</h2><br>
	<?php build_pager($_GET['page'],$players_pager_query); //Generate Pager Bar ?>
	<?php stats_table($sql); //Build stats table data ?>
	</div>
</div>
	<br /><br /><br /><br /><br />
<?php stopbench(); //Stop and display benchmark.?>
</body>
</html>
