<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once __DIR__.'/../query/User.php';

// Init Models
$User = new User();

if (isset($_GET['hwid'])) {

    $hwid = $_GET['hwid'];
    $User->resetHwid($hwid);

} else {
    echo 'Missing format!';
}
