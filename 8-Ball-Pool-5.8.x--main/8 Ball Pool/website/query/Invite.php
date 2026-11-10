<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once 'DB.php';

class Invite extends DB
{

    public function getAll()
    {
        $this->prepare("SELECT * FROM core_invite_keys ORDER BY ID ASC");
        $this->statement->execute();
        return $this->statement->fetchAll();
    }

    public function get($key)
    {
        $this->prepare("SELECT * FROM core_invite_keys WHERE `key` = ? LIMIT 1");
        $this->statement->execute([$key]);
        return $this->statement->fetch();
    }

    public function delete($key)
    {
        $this->prepare("DELETE FROM core_invite_keys WHERE `key` = ?");
        $this->statement->execute([$key]);
    }

    public function create($username)
    {
        $this->prepare("INSERT INTO `core_invite_keys` (`key`, `createdBy`) VALUES (?, ?)");
        $this->statement->execute([uniqid(), $username]);
    }

}