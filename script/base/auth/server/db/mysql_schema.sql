CREATE TABLE IF NOT EXISTS `domains` (
    `id`		bigint(11) NOT NULL AUTO_INCREMENT,
    `name`		varchar(128) NOT NULL,
    `case_insensitive`	tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;

CREATE TABLE IF NOT EXISTS `users` (
    `id`	bigint(11) NOT NULL AUTO_INCREMENT,
    `name`	varchar(16) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    `pubkey`	varchar(51) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE (`name`, `pubkey`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ;

CREATE TABLE IF NOT EXISTS `domains_users` (
  `id`		bigint(11) NOT NULL AUTO_INCREMENT,
  `user_id`	bigint(11) NOT NULL,
  `domain_id`	bigint(11) NOT NULL,
  `privilege`	char(1) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=ascii ;
