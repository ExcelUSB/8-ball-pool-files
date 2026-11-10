<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once 'DB.php';

class User extends DB
{

    public function getAll()
    {
        $this->prepare("SELECT * FROM `core_members` ORDER BY id ASC");
        $this->statement->execute();
        return $this->statement->fetchAll();
    }

    public function getByUsername($username)
    {
        $this->prepare("SELECT * FROM `core_members` WHERE username = ? LIMIT 1");
        $this->statement->execute([$username]);
        return $this->statement->fetch();
    }

    public function getById($id)
    {
        $this->prepare("SELECT * FROM `core_members` WHERE id = ? LIMIT 1");
        $this->statement->execute([$id]);
        return $this->statement->fetch();
    }

    public function getByHWID($hwid)
    {
        $this->prepare("SELECT * FROM `core_members` WHERE HWid = ? LIMIT 1");
        $this->statement->execute([$hwid]);
        return $this->statement->fetch();
    }

    public function resetHWID($username)
    {
        $this->prepare("UPDATE `core_members` SET HWid = '' WHERE username = ?");
        $this->statement->execute([$username]);

        $this->prepare("UPDATE `core_members` SET resetAt = CURDATE() where username = ?");
        $this->statement->execute([$username]);
    }

    public function resetDiscordId($username)
    {
        $this->prepare("UPDATE `core_members` SET discordId = NULL WHERE username = ?");
        $this->statement->execute([$username]);
    }

    public function setHWID($username, $hwid)
    {
        $this->prepare("UPDATE `core_members` SET HWid = ? WHERE username = ?");
        $this->statement->execute([$hwid, $username]);
    }

    public function setDiscordId($username, $discordId)
    {
        $this->prepare("UPDATE `core_members` SET discordId = ? WHERE username = ?");
        $this->statement->execute([$discordId, $username]);
    }

    public function createUser($username, $password, $hwid, $invitedBy)
    {
        $today = date('Y-m-d');

        $this->prepare("INSERT INTO `core_members` (`username`, `password`, `HWID`, `invitedBy`, `resetAt`) VALUES (?, ?, ?, ?, ?)");
        $this->statement->execute([$username, $password, $hwid, $invitedBy, $today]);
    }

    public function role($role, $id)
    {
        $this->prepare("UPDATE `core_members` SET role = ? WHERE id = ?");
        $this->statement->execute([$role, $id]);
    }

    public function isSubActive($username)
    {
        $this->prepare("SELECT * FROM `core_members` WHERE `username` = ? AND `subEnd` > CURRENT_DATE()");
        $this->statement->execute([$username]);
        return $this->statement->fetch();
    }

    public function updatePassword($password, $id)
    {
        $this->prepare("UPDATE `core_members` SET `password` = ? WHERE `id` = ?");
        $this->statement->execute([$password, $id]);
    }

    public function updateSubscription($duration, $username)
    {
        $this->prepare("UPDATE `core_members` SET `subEnd` = ? WHERE `username` = ?");
        $this->statement->execute([$duration, $username]);
    }
}