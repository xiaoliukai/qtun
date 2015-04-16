<?php
require dirname(__FILE__).'/../../lib/base.php';

function login($params) {
    $user = get($params, 'user');
    $pass = get($params, 'pass');
    $pdo = conn();

    $sql = "SELECT `salt` FROM `users` WHERE `name` = '$user'";
    $ret = $pdo->query($sql);
    if ($ret->rowCount() == 0) return res(1);
    list($salt) = $ret->fetch(PDO::FETCH_NUM);
    $pass = sha1($pass.$salt);

    $sql = "UPDATE `users` SET `login` = CURRENT_TIMESTAMP WHERE `name` = '$user' AND `password` = '$pass'";
    if (!$pdo->exec($sql)) return res(1);
    $_SESSION['_t'] = uuid(8);

    return res(0);
}

ret_http(login($_REQUEST));