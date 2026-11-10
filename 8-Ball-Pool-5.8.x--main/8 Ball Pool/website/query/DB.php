<?php
defined('SECURITY') or exit('No direct script access allowed');

class DB
{

    protected $statement;
    private $dbHost = "localhost";
    private $dbUser = "aaaaaame_beta";
    private $dbPass = "Calculator098#";
    private $dbName = "aaaaaame_beta";

    protected function prepare($sql)
    {
        $this->statement = $this->connect()->prepare($sql);
    }

    protected function connect()
    {

        try {
            $dsn = 'mysql:host='.$this->dbHost.';dbname='.$this->dbName;
            $pdo = new PDO($dsn, $this->dbUser, $this->dbPass);
            $pdo->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_OBJ);
            return $pdo;
        } catch (PDOException $e) {
            print ("Error!: ".$e->getMessage()."<br/>");
            die();
        }

    }

}