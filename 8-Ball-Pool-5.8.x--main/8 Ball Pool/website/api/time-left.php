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

        $date = new DateTime(); // Get current date
        $currentDate = $date->format('Y-m-d'); // Format Year-Month-Day

        $currentTime = date_create($currentDate); // Convert String to date format
        $userSub = date_create($user->subEnd); // Convert String to date format
        $diff = date_diff($currentTime, $userSub);
        echo intval($diff->format("%R%a"));

    } else {
        echo 'invalid';
    }

} else {
    echo 'invalid session!';
}