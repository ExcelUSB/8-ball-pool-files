<?php

require_once 'require.php';
require_once 'functions/auth.php';
require_once 'app/login.php';

?>

<html lang="en">

<head>
    <title>Login</title>
</head>

<body>


<form method="post">

    <fieldset>
        <legend>Login</legend>

        <label for="username">Username</label>
        <input type="text" name="username" id="username">
        <br/>

        <label for="password">Password</label>
        <input type="text" name="password" id="password">
        <br/>

        <input type="submit" name="login">
    </fieldset>

</form>

<br/>
<?= isset($error) ? $error : "" ?>

</body>

</html>
