CREATE DEFINER=`root`@`localhost` PROCEDURE `reg_user`(
    IN new_name VARCHAR(255),
    IN new_email VARCHAR(255),
    IN new_pwd VARCHAR(255),
    OUT result INT
)
BEGIN
    DECLARE new_uid INT DEFAULT 0;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SET result = -1;
    END;

    START TRANSACTION;

    IF EXISTS (
        SELECT 1
        FROM `user`
        WHERE `name` COLLATE utf8mb4_unicode_ci = new_name COLLATE utf8mb4_unicode_ci
    ) THEN
        SET result = 0;
        COMMIT;
    ELSEIF EXISTS (
        SELECT 1
        FROM `user`
        WHERE `email` COLLATE utf8mb4_unicode_ci = new_email COLLATE utf8mb4_unicode_ci
    ) THEN
        SET result = 0;
        COMMIT;
    ELSE
        UPDATE `user_id` SET `id` = `id` + 1;
        SELECT `id` INTO new_uid FROM `user_id` LIMIT 1;

        INSERT INTO `user` (`uid`, `name`, `email`, `pwd`)
        VALUES (new_uid, new_name, new_email, new_pwd);

        SET result = new_uid;
        COMMIT;
    END IF;
END