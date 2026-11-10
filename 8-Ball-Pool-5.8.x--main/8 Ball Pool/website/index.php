<?php

require_once 'require.php';
require_once 'functions/auth.php';
require_once 'query/User.php';

$User = new User;
$users = $User->getAll();

?>

<html lang="en">
<head>
    <title>Welcome</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>

<?php include_once 'navbar.php' ?>

<table>
    <tr>
        <th>UID</th>
        <th>User</th>
        <th>Actions</th>
    </tr>
    <?php foreach ($users as $row) : ?>
        <tr>
            <td><?= $row->id ?></td>
            <td><?= $row->username ?></td>
            <td><a href="user.php?id=<?= $row->id ?>">Edit</a></td>
        </tr>
    <?php endforeach; ?>

</table>
</body>
</html>
