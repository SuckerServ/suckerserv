CREATE TABLE IF NOT EXISTS domains (
    id          	INTEGER PRIMARY KEY,
    name	        TEXT UNIQUE,
    case_insensitive	INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS users (
    id          INTEGER PRIMARY KEY,
    domain_id   INTEGER REFERENCES domains(id),
    name        TEXT UNIQUE,
    pubkey      TEXT
);

CREATE TRIGGER IF NOT EXISTS delete_domain_trigger AFTER DELETE ON domains
BEGIN
    DELETE FROM users WHERE domain_id = OLD.id;
END;
