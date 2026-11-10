<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once __DIR__.'/../query/User.php';

// Init Models
$User = new User();

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['setRole'])) {

    $id = (int) $_GET['id'];
    $role = (int) $_POST['role'];

    $User->role($role, $id);
    header("location: user.php?id=${id}");

}
