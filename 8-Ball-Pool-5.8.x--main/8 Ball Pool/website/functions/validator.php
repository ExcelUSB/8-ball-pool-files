<?php
defined('SECURITY') or exit('No direct script access allowed');

function validateUsername($username)
{
    $usernameSchema = "/^[a-zA-Z0-9]*$/";
    $error = false;
    if (empty($username)) {
        $error = "Please enter a username.";
    } elseif (strlen($username) < 3) {
        $error = "Username is too short.";
    } elseif (strlen($username) > 14) {
        $error = "Username is too long.";
    } elseif (!preg_match($usernameSchema, $username)) {
        $error = "Username must only contain alphanumerical!";
    }
    return $error;
}

function validatePassword($password)
{
    $error = false;
    if (empty($password)) {
        $error = "Please enter a password.";
    } elseif (strlen($password) < 4) {
        $error = "Password is too short.";
    } elseif (strlen($password) > 50) {
        $error = "Password is too long.";
    }
    return $error;
}

function form($username, $password)
{
    $validateUsername = validateUsername($username); // Validate username
    $validatePassword = validatePassword($password); // Validate password

    if ($validateUsername) {
        return $validateUsername;
    }
    if ($validatePassword) {
        return $validatePassword;
    }

    return false;
}