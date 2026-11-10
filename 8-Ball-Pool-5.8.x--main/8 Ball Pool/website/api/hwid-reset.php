<?php

require_once __DIR__.'/../require.php';

if ($_SERVER["HTTP_USER_AGENT"] !== $gUserAgent) {
    die('no');
}

require_once __DIR__.'/../query/User.php';

// Init Models
$User = new User();

if (isset($_GET['username']) && isset($_GET['hwid'])) {

    $hwid     = $_GET['hwid'];
    $username = $_GET['username'];
    $user = $User->getByUsername($username);

    if ($user && $User->isSubActive($username)) {

        $end_date = $user->resetAt;
        $now = date('Y-m-d H:i:s');
        $next_reset = date('Y-m-d', strtotime($end_date.' + 8 days'));
        $diff = strtotime($next_reset) - strtotime($now);
        $fullDays = floor($diff / (60 * 60 * 24));

        if ($fullDays <= 0 || $end_date == "0000-00-00") {

            $User->resetHwid($username);
            $User->setHwid($username, $hwid);
            echo 'valid';

        } else {
            echo "You have to wait $fullDays more days!";
        }

    } else {
        echo 'invalid';
    }

} else {
    echo 'invalid session!';
}


