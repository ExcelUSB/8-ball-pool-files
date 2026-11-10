<?php

require_once __DIR__.'/../require.php';

if ($_SERVER["HTTP_USER_AGENT"] !== $gUserAgent) {
    die('no');
}

require_once __DIR__.'/../functions/login.php';
require_once __DIR__.'/../functions/validator.php';
require_once __DIR__.'/../query/User.php';

// Init Models
$User = new User();

if (isset($_GET['username']) && isset($_GET['password']) && isset($_GET['hwid'])) {
    $username = $_GET['username'];
    $hwid = $_GET['hwid'];

    $user = $User->getByUsername($username);

    // User exists
    if ($user) {
        if ((int) $user->role === 0) {
            echo 'banned';
        } else {
            if ($user->HWID) {
                echo 'exists';
            } else {
                $User->setHwid($username, $hwid);
                echo 'set';
            }
        }
    } else {
        echo 'invalid';
    }

} else {
    echo 'Missing format!';
}