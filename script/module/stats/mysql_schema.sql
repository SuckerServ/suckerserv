CREATE TABLE IF NOT EXISTS `games` (
  `id`          bigint(11) NOT NULL AUTO_INCREMENT,
  `servername`  varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
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
  `name`            varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `ipaddr`          varchar(16),
  `country`         char(3),
  `score`           smallint(6) NOT NULL,
  `frags`           smallint(5) unsigned NOT NULL,
  `deaths`          smallint(5) unsigned NOT NULL,
  `suicides`        smallint(5) unsigned NOT NULL,
  `teamkills`       smallint(6) NOT NULL,
  `hits`            int(10) unsigned NOT NULL,
  `misses`          int(10) unsigned NOT NULL,
  `shots`           int(10) unsigned NOT NULL,
  `damage`          int(10) unsigned NOT NULL,
  `damagewasted`    int(10) unsigned NOT NULL,
  `timeplayed`      smallint(5) unsigned NOT NULL,
  `finished`        tinyint(1) NOT NULL,
  `win`             tinyint(1) NOT NULL,
  `rank`            smallint(6) NOT NULL,
  `botskill`        smallint(6) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `name` (`name`),
  KEY `ipaddr` (`ipaddr`),
  KEY `gameid` (`game_id`),
  KEY `game_id` (`game_id`,`name`,`ipaddr`,`country`,`score`,`frags`,`deaths`,`teamkills`,`hits`,`misses`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;

CREATE TABLE IF NOT EXISTS  `playertotals` (
  `id`              bigint(11) NOT NULL AUTO_INCREMENT,
  `name`            varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `ipaddr`          varchar(16) CHARACTER SET ascii,
  `country`         tinytext COLLATE latin1_general_ci,
  `first_game`      text,
  `last_game`       text,
  `frags`           int(10) unsigned NOT NULL DEFAULT 0,
  `max_frags`       mediumint(8) unsigned NOT NULL DEFAULT 0,
  `deaths`          int(10) unsigned NOT NULL DEFAULT 0,
  `suicides`        int(10) unsigned NOT NULL DEFAULT 0,
  `teamkills`       int(10) unsigned NOT NULL DEFAULT 0,
  `hits`            int(10) unsigned NOT NULL DEFAULT 0,
  `misses`          int(10) unsigned NOT NULL DEFAULT 0,
  `shots`           int(10) unsigned NOT NULL DEFAULT 0,
  `damage`          bigint(11) unsigned NOT NULL DEFAULT 0,
  `damagewasted`    bigint(11) unsigned NOT NULL DEFAULT 0,
  `games`           int(10) unsigned NOT NULL DEFAULT 0,
  `wins`            int(10) unsigned NOT NULL DEFAULT 0,
  `withdraws`       int(10) unsigned NOT NULL DEFAULT 0,
  `timeplayed`      int(10) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE(name)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;
