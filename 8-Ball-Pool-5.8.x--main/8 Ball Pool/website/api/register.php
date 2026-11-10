<?php

require_once __DIR__.'/../require.php';

if ($_SERVER["HTTP_USER_AGENT"] !== $gUserAgent) {
    die('no');
}

require_once __DIR__.'/../functions/register.php';
require_once __DIR__.'/../functions/validator.php';

if (isset($_GET['username']) && isset($_GET['password']) && isset($_GET['hwid'])) {
    $username = $_GET['username'];
    $password = $_GET['password'];
    $key = $_GET['key'];
    $hwid = $_GET['hwid'];

    // Validate data
    $error = form($username, $password);

    if (!$error) {
        register($username, $password, $key, $hwid);
    } else {
        echo $error;
    }

} else {
    echo 'Missing format!';
}