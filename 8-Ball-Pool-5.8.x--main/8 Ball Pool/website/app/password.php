<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once __DIR__.'/../functions/validator.php';
require_once __DIR__.'/../query/User.php';

// Init Models
$User = new User();

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['updatePassword'])) {

    $password = $_POST['password'];
    $cPassword = $_POST['cPassword'];
    $id = $_GET['id'];

    // Validate data
    $error = validatePassword($password);

    if (!$error) {
        if ($password === $cPassword) {

            $hashedPassword = password_hash($password, PASSWORD_DEFAULT);
            $User->updatePassword($hashedPassword, $id);
            echo 'Updated Password.';
            header("location: user.php?id=${id}");

        } else {
            echo 'Password doesnt match';
        }
    } else {
        echo $error;
    }

}
