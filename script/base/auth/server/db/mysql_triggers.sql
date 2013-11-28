CREATE TRIGGER `delete_domain_trigger` AFTER DELETE ON `domains`
FOR EACH ROW BEGIN
    DELETE FROM domains_users WHERE domain_id = OLD.id;
END ~

CREATE TRIGGER `delete_user_trigger` AFTER DELETE ON `users`
FOR EACH ROW BEGIN
    DELETE FROM domains_users WHERE user_id = OLD.id;
END ~

CREATE TRIGGER `delete_domains_user_trigger` AFTER DELETE ON `domains_users`
FOR EACH ROW BEGIN
    IF (SELECT COUNT(*) FROM users_domains WHERE user_id = OLD.user_id) = 0 THEN
        DELETE FROM users WHERE id = OLD.user_id;
    END IF;
END ~

