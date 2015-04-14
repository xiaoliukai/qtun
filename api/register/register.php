<?php
require dirname(__FILE__).'/../../framework/base.php';
require ROOT.'/lib/base.php';

function register($params) {
    $user = get($params, 'user');
    $pass = get($params, 'pass');
    $email = get($params, 'email', '');
    $recvmsg = get($params, 'recvmsg', 1);
    $db = conn();

    if (strlen($pass) != 32) return res(1);
    $salt = uuid(8);
    $pass = sha1($pass.$salt);

    $sql = "SELECT COUNT(1) FROM users WHERE name='$user'";
    list($have) = $db->query($sql)->fetch(PDO::FETCH_NUM);

    if ($have) return res(1);

    $recvmsg = empty($recvmsg) ? 0 : 1;
    $sql = "INSERT INTO `users`(`name`, `recvmsg`, `password`, `salt`, `email`) VALUES('$user', b'$recvmsg', '$pass', '$salt', '$email')";
    if ($db->exec($sql) === false) {
        return res(1);
    }
    return res(0);
}

ret_http(register($_REQUEST));