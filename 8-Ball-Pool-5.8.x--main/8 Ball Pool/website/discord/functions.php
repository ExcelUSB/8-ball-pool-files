<?php
/* Discord Oauth v.4.1
 * This file contains the core functions of the oauth2 script.
 * @author : MarkisDev
 * @copyright : https://markis.dev
 */

# Starting session so we can store all the variables
session_start();

# Setting the base url for API requests
$GLOBALS['base_url'] = "https://discord.com";

# Setting bot token for related requests
$GLOBALS['bot_token'] = 'OTg0MTQxNjAwMzk1NDkzNDI3.Gj6bJI.Hj6g9iFSCNc1MAQPmal_BAR9fXN4gymz52RWZ4';

# A function to generate a random string to be used as state | (protection against CSRF)
function gen_state()
{
    $_SESSION['state'] = bin2hex(openssl_random_pseudo_bytes(12));
    return $_SESSION['state'];
}

# A function to generate oAuth2 URL for logging in
function url($client_id, $redirect, $scope)
{
    $state = gen_state();
    return 'https://discordapp.com/oauth2/authorize?response_type=code&client_id='.$client_id.'&redirect_uri='.$redirect.'&scope='.$scope."&state=".$state;
}

# A function to initialize and store access token in SESSION to be used for other requests
function init($redirect_url, $client_id, $client_secret)
{
    $code = $_GET['code'];
    $state = $_GET['state'];
    # Check if $state == $_SESSION['state'] to verify if the login is legit | CHECK THE FUNCTION get_state($state) FOR MORE INFORMATION.
    $url = $GLOBALS['base_url']."/api/v10/oauth2/token";
    $data = array(
        "client_id" => $client_id,
        "client_secret" => $client_secret,
        "grant_type" => "authorization_code",
        "code" => $code,
        "redirect_uri" => $redirect_url
    );

    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_POST, true);
    curl_setopt($curl, CURLOPT_POSTFIELDS, http_build_query($data));
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false);
    $response = curl_exec($curl);
    $results = json_decode($response, true);
    curl_close($curl);

    if (array_key_exists('access_token', $results)) {
        $_SESSION['access_token'] = $results['access_token'];
    } else {
        print_r($results);
    }
}

function get_user()
{
    $url = $GLOBALS['base_url']."/api/users/@me";
    $headers = array(
        'Content-Type: application/x-www-form-urlencoded', 'Authorization: Bearer '.$_SESSION['access_token']
    );
    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($curl, CURLOPT_HTTPHEADER, $headers);
    curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false);
    $response = curl_exec($curl);
    curl_close($curl);
    $results = json_decode($response, true);
    $_SESSION['user_id'] = $results['id'];
}

# Function to make user join a guild | (guilds.join scope)
# Note : The bot has to be a member of the server with CREATE_INSTANT_INVITE permission.
#        The bot DOES NOT have to be online, just has to be a bot application and has to be a member of the server.
#        This is the basic function with no parameters, you can build on this to give the user a nickname, mute, deafen or assign a role.
function join_guild()
{
    $guild_id = '977240258133504010';
    
    $roleArray = array(
        "1" => "982997589219741726", // User
        "2" => "984390645395390484", // Beta
        "3" => "979786082184994826", // Admin
    );

    $data = json_encode(array(
        "access_token" => $_SESSION['access_token'],
        "roles" => [$roleArray[$_SESSION['auth_role']]],
        "nick" => $_SESSION['auth_username']
    ));

    $url = $GLOBALS['base_url']."/api/guilds/$guild_id/members/".$_SESSION['user_id'];
    $headers = array('Content-Type: application/json', 'Authorization: Bot '.$GLOBALS['bot_token']);

    $curl = curl_init();
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_setopt($curl, CURLOPT_HTTPHEADER, $headers);
    curl_setopt($curl, CURLOPT_POSTFIELDS, $data);
    curl_setopt($curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false);

    $response = curl_exec($curl);
    curl_close($curl);
    $results = json_decode($response, true);
}
