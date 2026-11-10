<?php

require_once __DIR__.'/../../require.php';

if ($_SERVER["HTTP_USER_AGENT"] !== $gUserAgent) { die('no');}

require_once __DIR__.'/../../functions/login.php';
require_once __DIR__.'/../../functions/validator.php';
require_once __DIR__.'/../../query/User.php';

// Init Models
$User = new User();

if (isset($_GET['username']) && isset($_GET['password']) && isset($_GET['hwid']) ) {

    $hwid     = $_GET['hwid'];
    $password = $_GET['password'];
    $username = $_GET['username'];

    $error = form($username, $password);
    if (!$error) {
        $user = login($username, $password);

        // User exists
        if ($user && (int)$user->role !== 0) {
            if ($hwid && $User->isSubActive($username)) {
                echo "good";
            }
        }
    }
}