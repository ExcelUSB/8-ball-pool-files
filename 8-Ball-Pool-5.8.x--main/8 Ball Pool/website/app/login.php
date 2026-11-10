<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once __DIR__.'/../functions/login.php';
require_once __DIR__.'/../functions/validator.php';
require_once __DIR__.'/../query/User.php';

// Init Models
$User = new User();

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['login'])) {

    // Get data
    $username = trim($_POST['username']);
    $password = (string) $_POST['password'];

    // Validate data
    $error = form($username, $password);

    if (!$error) {
        $user = login($username, $password);

        if ($user && (int) $user->role === 3) {

            $_SESSION["login"] = true;
            $_SESSION["username"] = (string) $user->username;
            $_SESSION["role"] = $user->role;

            header("location: /");
            exit;

        } else {
            $error = 'Username/Password is wrong.';
        }
    }

}