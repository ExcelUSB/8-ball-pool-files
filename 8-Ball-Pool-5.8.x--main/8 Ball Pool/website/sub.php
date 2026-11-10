<?php

require_once 'require.php';
require_once 'functions/auth.php';
require_once 'query/Subscription.php';
require_once 'app/subscription.php';

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
        <legend>Create Code</legend>

        <label for="days">Duration</label>
        <select id="days" name="days">
            <option value="7">7</option>
            <option value="14">14</option>
            <option value="30">30</option>
            <option value="999">Lifetime</option>
        </select>

        <input type="submit" name="subCode" value="Create"/>
    </fieldset>
</form>

<table>
    <tr>
        <th>ID</th>
        <th>Code</th>
        <th>Length</th>
    </tr>
    <?php foreach ($Subscription->getAll() as $row) : ?>
        <tr>
            <td><?= $row->id ?></td>
            <td><?= $row->key ?></td>
            <td><?= $row->days ?></td>
        </tr>
    <?php endforeach; ?>

</table>
</body>
</html>
