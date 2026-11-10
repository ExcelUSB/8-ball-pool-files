<?php
defined('SECURITY') or exit('No direct script access allowed');

session_start();

if (isset($_SESSION['login']) && basename($_SERVER['PHP_SELF']) === 'login.php') {
    header("location: /");
    exit;
} else {
    if (!isset($_SESSION['login']) && basename($_SERVER['PHP_SELF']) !== 'login.php') {
        header("location: /login.php");
        exit;
    }
}