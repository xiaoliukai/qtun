<?php
require dirname(__FILE__).'/../lib/base.php';

foreach (array_keys($_SESSION) as $key) {
    unset($_SESSION[$key]);
}
echo '{"stat":0}';