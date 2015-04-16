<?php
require dirname(__FILE__).'/../../lib/base.php';

function check_exists($params) {
    $name = get($params, 'name');
    $db = conn();
    $sql = "SELECT COUNT(1) FROM users WHERE name='$name'";
    list($have) = $db->query($sql)->fetch(PDO::FETCH_NUM);
    return res($have);
}

ret_http(check_exists($_REQUEST));