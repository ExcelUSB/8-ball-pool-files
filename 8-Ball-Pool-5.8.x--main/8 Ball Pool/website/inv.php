<?php

require_once 'require.php';
require_once 'functions/auth.php';
require_once 'query/Invite.php';
require_once 'app/invite.php';

$Inv = new Invite();

?>

<html lang="en">
<head>
    <title>Welcome</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>

<?php include_once 'navbar.php' ?>

<form method="post">
    <fieldset>
        <legend>Create Inv</legend>
        <input type="submit" name="invCode" value="Create Inv"/>
    </fieldset>
</form>

<table>
    <tr>
        <th>ID</th>
        <th>Code</th>
        <th>createdBy</th>
        <th>createdAt</th>
    </tr>
    <?php foreach ($Inv->getAll() as $row) : ?>
        <tr>
            <td><?= $row->id ?></td>
            <td><?= $row->key ?></td>
            <td><?= $row->createdBy ?></td>
            <td><?= $row->createdAt ?></td>
        </tr>
    <?php endforeach; ?>

</table>
</body>
</html>
