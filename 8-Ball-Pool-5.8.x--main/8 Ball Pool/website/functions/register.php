<?php
defined('SECURITY') or exit('No direct script access allowed');

require_once __DIR__.'/../query/User.php';
require_once __DIR__.'/../query/Invite.php';

function register($username, $password, $key, $hwid)
{
    $User = new User();
    $Invite = new Invite();

    $invitee = $Invite->get($key);
    if ($invitee) {

        // ( Username || HWID ) is taken
        if ($User->getByUsername($username) || $User->getByHWID($hwid)) {
            echo 'found';
        } else {
            $hashedPassword = password_hash($password, PASSWORD_DEFAULT);
            $User->createUser($username, $hashedPassword, $hwid, $invitee->createdBy);
            $Invite->delete($key);
        }

    } else {
        echo 'invalid-key';
    }

}