<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once __DIR__.'/../query/Invite.php';

// Init Models
$Inv = new Invite();

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['invCode'])) {

    $Inv->create($_SESSION['username']);

    header("location: inv.php");
}