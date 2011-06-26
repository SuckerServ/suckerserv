<?php
$pagename = "game details";
include("includes/geoip.inc");
include("includes/hopmod.php");

function no_id() {
?>
<h1>Please provide a correct game ID</h1>
<?php stopbench(); ?>
</body>
</html>
<?php
exit;
}

if ( isset($_GET['showprofile']) ) {
	$profile_name = "and name='".sqlite_escape_string($_GET['showprofile'])."' ";
}
if (isset($_GET['id']) and $_GET['id'] != "") {
    $_SESSION['id'] = $_GET['id'];
} elseif (isset($_SESSION['id']) and $_SESSION['id'] != "") {
} else { no_id(); }

$day_end = strtotime("today +1 day");
$week_end = strtotime("this week today +1 week");
$month_end = strtotime("this month today +1 month");
$year_end = strtotime("today");

$day_start = strtotime("today");
$week_start = strtotime("this week today");
$month_start = strtotime("this month today");
$year_start = strtotime("-365 days");

$_SESSION['start_date'] = ${$_SESSION['querydate']."_start"};
$_SESSION['end_date'] = ${$_SESSION['querydate']."_end"};

// Start session for session vars
session_start();
        $sql = "
select 	name,
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
where game_id = '".$_SESSION['id']."' group by name order by ".$_SESSION['orderby']." desc
";



?>
<?php match_table($_SESSION['id']); //Build stats table data ?>
<?php stats_table($sql); ?>
<br />
<?php stopbench(); //Stop and display benchmark.?>
</body>
</html>












