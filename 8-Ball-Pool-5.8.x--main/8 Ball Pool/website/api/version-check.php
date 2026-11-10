<?php

require_once __DIR__.'/../require.php';

if ($_SERVER["HTTP_USER_AGENT"] !== $gUserAgent) { die('no');}

echo $version;