<?php

require_once __DIR__.'/../require.php';

if ($_SERVER["HTTP_USER_AGENT"] !== $gUserAgent) {
    die('no');
}

require_once __DIR__.'/../query/User.php';

// Init Models
$User = new User();

if (isset($_GET['username'])) {

    $username = $_GET['username'];
    $user = $User->getByUsername($username);

    if ($user) {

        $User->resetDiscordId($username);
        echo 'valid';

    } else {
        echo 'invalid';
    }

} else {
    echo 'invalid session!';
}


