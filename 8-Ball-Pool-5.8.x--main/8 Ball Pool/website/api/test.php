<?php

require_once __DIR__.'/../require.php';

if ($_SERVER["HTTP_USER_AGENT"] !== $gUserAgent) { die('no');}

require_once __DIR__.'/../functions/login.php';
require_once __DIR__.'/../functions/validator.php';
require_once __DIR__.'/../query/User.php';

// Init Models
$User = new User();

//if (isset($_GET['username']) && isset($_GET['password']) && isset($_GET['hwid']) && isset($_GET['file']) ) {
//if (isset($_GET['uskoamn']) && isset($_GET['yreuso']) && isset($_GET['janshuw']) && isset($_GET['kdjiahs']) ) {
if (isset($_GET['username']) && isset($_GET['password']) && isset($_GET['hwid']) &&    isset($_GET['file'])) {
    // $username = $_GET['username'];
    // $password = $_GET['password'];
    // $hwid = $_GET['hwid'];
    // $file = $_GET['file'];
    
    $file     = $_GET['file'];
    $hwid     = $_GET['hwid'];
    $password = $_GET['password'];
    $username = $_GET['username'];

    // Validate data
    $error = form($username, $password);

    if (!$error) {
        $user = login($username, $password);

        // User exists
        if ($user) {
            //echo 'valid';

            if ((int) $user->role === 0) {
                echo 'banned';
            } else if ($hwid && $User->isSubActive($username)) {
                if (!$user->HWID) {
                    $User->setHwid($username, $hwid);
                }
                if (!$user->HWID || $user->HWID == $hwid) {
                    $filepath = "check-modules/" . $file;
                    if (file_exists($filepath)) {
                        echo file_get_contents($filepath);
                    }
                    else
                    {
                        echo "\x1\x2\x3\x4\x5\x6\x7\x8\x9"; // random shit
                    }
                
                } else {
                    echo 'incorrect';
                }
            }

        } else {
            echo 'invalid';
        }

    } else {
        echo $error;
    }

} else {
    echo 'Missing format!';
}