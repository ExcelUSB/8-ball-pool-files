<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once __DIR__.'/../query/User.php';

function login($username, $password)
{
    $User = new User();
    $row = $User->getByUsername($username);
    return $row && password_verify($password, $row->password) ? $row : false;
}