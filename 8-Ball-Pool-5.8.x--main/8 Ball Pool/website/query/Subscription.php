<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once 'DB.php';

class Subscription extends DB
{

    public function getAll()
    {
        $this->prepare("SELECT * FROM `core_sub_keys` ORDER BY `id` ASC");
        $this->statement->execute();
        return $this->statement->fetchAll();
    }

    public function get($key)
    {
        $this->prepare("SELECT * FROM `core_sub_keys` WHERE `key` = ? LIMIT 1");
        $this->statement->execute([$key]);
        return $this->statement->fetch();
    }

    public function delete($key)
    {
        $this->prepare("DELETE FROM `core_sub_keys` WHERE `key` = ?");
        $this->statement->execute([$key]);
    }

    public function create($days)
    {
        $this->prepare("INSERT INTO `core_sub_keys` ( `key`, `days`) VALUES (UUID(), ?)");
        $this->statement->execute([$days]);
    }

}