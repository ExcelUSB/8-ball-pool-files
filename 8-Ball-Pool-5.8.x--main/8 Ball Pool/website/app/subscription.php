<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once __DIR__.'/../query/Subscription.php';

// Init Models
$Subscription = new Subscription();

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['subCode'])) {

    $days = (int) $_POST['days'];
    $Subscription->create($days);

    header("location: sub.php");

}