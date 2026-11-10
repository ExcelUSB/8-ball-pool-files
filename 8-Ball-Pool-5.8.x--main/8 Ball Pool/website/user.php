<?php

require_once 'require.php';
require_once 'functions/auth.php';
require_once 'query/User.php';

$User = new User;

if (isset($_GET['id'])) {

    $id = $_GET['id'];
    $user = $User->getById($id);

    if (!$user) {
        die('invalid user');
    } else {
        require_once 'app/role.php';
        require_once 'app/password.php';
    }

} else {
    die('invalid user');
}
?>

<html lang="en">
<head>
    <title>User: <?= $user->id ?></title>
    <link rel="stylesheet" href="style.css">
</head>
<body>

<?php include_once 'navbar.php' ?>

<table>
    <tr>
        <th>UID</th>
        <th>User</th>
        <th>Role</th>
        <th>subEnd</th>
        <th>HWID</th>
        <th>invitedBy</th>
        <th>resetAt</th>
        <th>createdAt</th>
    </tr>

    <tr>
        <td><?= $user->id ?></td>
        <td><?= $user->username ?></td>
        <td><?= $user->role ?></td>
        <td><?= $user->subEnd ?></td>
        <td><?= $user->HWID ?></td>
        <td><?= $user->invitedBy ?></td>
        <td><?= $user->resetAt ?></td>
        <td><?= $user->createdAt ?></td>
    </tr>
</table>

<br/>

<form method="post">
    <fieldset>
        <legend>Update Password</legend>

        <label for="password">Password</label>
        <input type="text" id="password" name="password"/>
        <br/>

        <label for="cPassword">Confirm Password</label>
        <input type="text" id="cPassword" name="cPassword"/>
        <br/>

        <input type="submit" name="updatePassword" value="Submit">
    </fieldset>
</form>


<form method="post">
    <fieldset>
        <legend>Set Role</legend>

        <label for="role">Role</label>
        <select id="role" name="role">
            <option value="0">Ban</option>
            <option value="1">User</option>
            <option value="2">Beta</option>
            <option value="3">Admin</option>
        </select>

        <input type="submit" name="setRole" value="Submit"/>
    </fieldset>
</form>


</body>
</html>


