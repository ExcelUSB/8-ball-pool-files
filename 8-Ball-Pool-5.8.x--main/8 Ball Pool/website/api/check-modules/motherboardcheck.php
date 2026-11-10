<?php

require_once __DIR__.'/../../require.php';

if ($_SERVER["HTTP_USER_AGENT"] !== $gUserAgent) { die('no');}

require_once __DIR__.'/../../functions/login.php';
require_once __DIR__.'/../../functions/validator.php';
require_once __DIR__.'/../../query/User.php';

$User = new User();

$wrongResult   = "\x1\x2\x3\x4\x5\x6\x7\x8\x9";
$correctResult = "QfTjWnZr4t7w!z%C*F-JaNdRgUkXp2s5v8x/A?D(G+KbPeShVmYq3t6w9z\$B&E)H";

if (isset($_GET['username']) && isset($_GET['password']) && isset($_GET['hwid']) ) {
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
                } 
                else if ($hwid && $User->isSubActive($username)) {
                    if (!$user->HWID) {
                        echo $wrongResult; // user must be already in db.
                    }
                    
                    if ($user->HWID == $hwid) 
                    {
                        echo $correctResult;
                    } 
                    else 
                    {
                        echo $wrongResult;
                    }
                }

            } else {
                echo $wrongResult;
            }

        } 
        else 
        {
            echo $wrongResult;
        }

} 
else 
{
    echo $wrongResult;
}