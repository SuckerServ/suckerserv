CREATE TRIGGER `delete_domain_trigger` AFTER DELETE ON `domains`
FOR EACH ROW BEGIN
    DELETE FROM users WHERE domain_id = OLD.id;
END ~
