CREATE SEQUENCE gamesid;

CREATE TABLE games  (
   id           bigint DEFAULT nextval('gamesid') NOT NULL,
   servername   varchar(32) NOT NULL,
   datetime     TIMESTAMP NOT NULL,
   gamemode     TEXT NOT NULL,
   mapname      TEXT NOT NULL,
   duration    INTEGER    NOT NULL,
   finished     SMALLINT  NOT NULL,
   players      SMALLINT    NOT NULL,
   bots         SMALLINT    NOT NULL,
  PRIMARY KEY ( id )
);

CREATE INDEX iservername ON games (servername);

CREATE SEQUENCE teamsid;

CREATE TABLE teams  (
   id           bigint DEFAULT nextval('teamsid') NOT NULL,
   game_id      bigint NOT NULL,
   name         TEXT NOT NULL,
   score        smallint  NOT NULL,
   win          SMALLINT  NOT NULL,
   draw         SMALLINT  NOT NULL,
  PRIMARY KEY ( id )
);

CREATE SEQUENCE playersid;

CREATE TABLE players  (
   id               bigint DEFAULT nextval('playersid') NOT NULL,
   game_id          bigint NOT NULL,
   team_id          bigint NOT NULL,
   name             varchar(32) NOT NULL,
   ipaddr           varchar(16),
   country          TEXT,
   score            INTEGER  NOT NULL,
   frags            INTEGER    NOT NULL,
   deaths           INTEGER    NOT NULL,
   suicides         INTEGER    NOT NULL,
   teamkills        INTEGER  NOT NULL,
   hits             INTEGER    NOT NULL,
   misses           INTEGER    NOT NULL,
   shots            INTEGER    NOT NULL,
   damage           INTEGER    NOT NULL,
   damagewasted     INTEGER    NOT NULL,
   timeplayed       INTEGER    NOT NULL,
   finished         SMALLINT  NOT NULL,
   win              SMALLINT  NOT NULL,
   rank             smallint  NOT NULL,
   botskill         smallint  NOT NULL,
  PRIMARY KEY ( id )
);

CREATE INDEX gin ON players (name,ipaddr,game_id);

CREATE SEQUENCE playertotalsid;

CREATE TABLE playertotals  (
   id               bigint DEFAULT nextval('playertotalsid') NOT NULL,
   name             varchar(32) NOT NULL,
   ipaddr           varchar(16),
   country          TEXT ,
   first_game       timestamp with time zone,
   last_game        timestamp with time zone,
   frags            INTEGER    NOT NULL DEFAULT 0,
   max_frags        INTEGER    NOT NULL DEFAULT 0,
   deaths           INTEGER    NOT NULL DEFAULT 0,
   suicides         INTEGER    NOT NULL DEFAULT 0,
   teamkills        INTEGER    NOT NULL DEFAULT 0,
   hits            INTEGER    NOT NULL DEFAULT 0,
   misses          INTEGER    NOT NULL DEFAULT 0,
   shots           INTEGER    NOT NULL DEFAULT 0,
   damage          INTEGER    NOT NULL DEFAULT 0,
   damagewasted    INTEGER    NOT NULL DEFAULT 0,
   games            smallint    NOT NULL DEFAULT 0,
   wins             smallint    NOT NULL DEFAULT 0,
   withdraws        smallint    NOT NULL DEFAULT 0,
   timeplayed      INTEGER    NOT NULL DEFAULT 0,
  PRIMARY KEY ( id ),
  CONSTRAINT cname UNIQUE (name)
);
