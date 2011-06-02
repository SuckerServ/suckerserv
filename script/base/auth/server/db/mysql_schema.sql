CREATE TABLE IF NOT EXISTS `domains` (
    `id`		bigint(11) NOT NULL AUTO_INCREMENT,
    `name`		varchar(128) NOT NULL,
    `case_insensitive`	tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;

CREATE TABLE IF NOT EXISTS `users` (
    `id`	bigint(11) NOT NULL AUTO_INCREMENT,
    `domain_id`	bigint(11) NOT NULL,
    `name`	varchar(16) NOT NULL,
    `pubkey`	varchar(51) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `domain_id` (`domain_id`),
  UNIQUE (`name`, `domain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;
