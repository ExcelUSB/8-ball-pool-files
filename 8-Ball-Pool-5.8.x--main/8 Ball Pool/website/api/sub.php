<?php

require_once __DIR__.'/../require.php';

if ($_SERVER["HTTP_USER_AGENT"] !== $gUserAgent) {
    die('no');
}

require_once __DIR__.'/../query/User.php';
require_once __DIR__.'/../query/Subscription.php';

// Init Models
$User = new User();
$Subscription = new Subscription();

if (isset($_GET['username']) && isset($_GET['subCode'])) {
    $username = $_GET['username'];
    $subCode = $_GET['subCode'];

    if (empty($subCode)) {
        echo 'Please enter a code.';
    } else {
        $sub = $Subscription->get($subCode);
        $days = 'P'.$sub->days.'D';

        if ($User->isSubActive($username)) {
            echo 'Sub is already active.';
        } else {
            if ($sub) {
                $date = new DateTime(); // Get current date
                $date->add(new DateInterval($days)); // Adds 32 days
                $duration = $date->format('Y-m-d'); // Format Year-Month-Day

                $User->updateSubscription($duration, $username);
                $Subscription->delete($subCode);
                echo 'Sub activated.';
            } else {
                echo 'Invalid code.';
            }
        }
    }

} else {
    echo 'invalid session!';
}