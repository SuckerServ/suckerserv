CREATE TABLE IF NOT EXISTS `games` (
  `id`          bigint(11) NOT NULL AUTO_INCREMENT,
  `servername`  varchar(32) COLLATE latin1_general_ci NOT NULL,
  `datetime`    datetime NOT NULL,
  `gamemode`    tinytext NOT NULL,
  `mapname`     tinytext NOT NULL,
  `duration`    int(10) unsigned NOT NULL,
  `finished`    tinyint(1) NOT NULL,
  `players`     tinyint(3) unsigned NOT NULL,
  `bots`        tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `servername` (`servername`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;

CREATE TABLE IF NOT EXISTS `teams` (
  `id`          bigint(11) NOT NULL AUTO_INCREMENT,
  `game_id`     bigint(11) NOT NULL,
  `name`        tinytext NOT NULL,
  `score`       smallint(6) NOT NULL,
  `win`         tinyint(1) NOT NULL,
  `draw`        tinyint(1) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;

CREATE TABLE IF NOT EXISTS  `players` (
  `id`              bigint(11) NOT NULL AUTO_INCREMENT,
  `game_id`         bigint(11) NOT NULL,
  `team_id`         bigint(11) NOT NULL,
  `name`            varchar(32) COLLATE latin1_general_ci NOT NULL,
  `ipaddr`          varchar(16),
  `country`         tinytext,
  `score`           smallint(6) NOT NULL,
  `frags`           smallint(5) unsigned NOT NULL,
  `deaths`          smallint(5) unsigned NOT NULL,
  `suicides`        smallint(5) unsigned NOT NULL,
  `teamkills`       smallint(6) NOT NULL,
  `hits`            smallint(5) unsigned NOT NULL,
  `misses`          smallint(5) unsigned NOT NULL,
  `shots`           smallint(5) unsigned NOT NULL,
  `damage`          smallint(5) unsigned NOT NULL,
  `damagewasted`    smallint(5) unsigned NOT NULL,
  `timeplayed`      smallint(5) unsigned NOT NULL,
  `finished`        tinyint(1) NOT NULL,
  `win`             tinyint(1) NOT NULL,
  `rank`            smallint(6) NOT NULL,
  `botskill`        smallint(6) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `name` (`name`),
  KEY `ipaddr` (`ipaddr`),
  KEY `gameid` (`game_id`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;

CREATE TABLE IF NOT EXISTS  `playertotals` (
  `id`              bigint(11) NOT NULL AUTO_INCREMENT,
  `name`            varchar(32) COLLATE latin1_general_ci NOT NULL,
  `ipaddr`          varchar(16) CHARACTER SET ascii,
  `country`         tinytext COLLATE latin1_general_ci,
  `first_game`      text,
  `last_game`       text,
  `frags`           mediumint(8) unsigned NOT NULL DEFAULT 0,
  `max_frags`       mediumint(8) unsigned NOT NULL DEFAULT 0,
  `deaths`          mediumint(8) unsigned NOT NULL DEFAULT 0,
  `suicides`        mediumint(8) unsigned NOT NULL DEFAULT 0,
  `teamkills`       mediumint(8) unsigned NOT NULL DEFAULT 0,
  `hits`            int(10) unsigned NOT NULL DEFAULT 0,
  `misses`          int(10) unsigned NOT NULL DEFAULT 0,
  `shots`           int(10) unsigned NOT NULL DEFAULT 0,
  `damage`          int(10) unsigned NOT NULL DEFAULT 0,
  `damagewasted`    int(10) unsigned NOT NULL DEFAULT 0,
  `games`           smallint(5) unsigned NOT NULL DEFAULT 0,
  `wins`            smallint(5) unsigned NOT NULL DEFAULT 0,
  `withdraws`       smallint(5) unsigned NOT NULL DEFAULT 0,
  `timeplayed`      int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE(name)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;
