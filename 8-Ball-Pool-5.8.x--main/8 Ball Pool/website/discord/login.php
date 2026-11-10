<?php

require_once __DIR__.'/../require.php';
require_once __DIR__.'/../functions/login.php';
require_once __DIR__.'/../functions/validator.php';
require_once __DIR__.'/../query/User.php';
require_once 'functions.php';

// Init Models
$User = new User();

// Vars
$client_id = '984141600395493427';
$client_secret = 'vdDOd_B4ZauE5TUYSuRG8ED3ZqsKj-1P';
$redirect_url = 'https://beta.aaaaaa.me/discord/login.php';
$scopes = 'identify+guilds.join';
$auth_url = url($client_id, $redirect_url, $scopes);

if (isset($_GET['username']) && isset($_GET['password'])) {
    $username = $_GET['username'];
    $password = $_GET['password'];

    // Validate data
    $error = form($username, $password);

    if (!$error) {
        $user = login($username, $password);
        if ($user) {
            $_SESSION['auth_discordId'] = $user->discordId;
            $_SESSION['auth_role'] = $user->role;
            $_SESSION['auth_username'] = $user->username;
            header("location: ${auth_url}");
        } else {
            echo 'Invalid login';
        }
    } else {
        echo $error;
    }

}

if (isset($_SESSION['state'])) {

    if (isset($_GET['code']) && isset($_GET['state'])) {
        init($redirect_url, $client_id, $client_secret);
    }

    if (isset($_SESSION['access_token'])) {
        if ($_SESSION['auth_discordId'] && $_SESSION['auth_discordId'] !== $_SESSION['user_id']) {
            die('ask for reset');
        } else {
            $User->setDiscordId($_SESSION['auth_username'], $_SESSION['user_id']);
            get_user();
            join_guild();
        }
    }
}