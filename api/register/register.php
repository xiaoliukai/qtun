<?php
require dirname(__FILE__).'/../../framework/base.php';
require ROOT.'/lib/base.php';

function register($params) {
    $user = get($params, 'user');
    $pass = get($params, 'pass');
    $email = get($params, 'email', '');
    $recvmsg = get($params, 'recvmsg', 1);
    $db = conn();

    $salt1 = $_SESSION['salt'];
    $salt2 = uuid(8);
    $pass = sha1($pass.$salt2);

    $sql = "SELECT COUNT(1) FROM users WHERE name='$user'";
    list($have) = $db->query($sql)->fetch(PDO::FETCH_NUM);

    if ($have) return res(1);

    $recvmsg = empty($recvmsg) ? 0 : 1;
    $sql = "INSERT INTO `users`(`name`, `recvmsg`, `password`, `salt1`, `salt2`, `email`) VALUES('$user', b'$recvmsg', '$pass', '$salt1', '$salt2', '$email')";
    if ($db->exec($sql) === false) {
        return res(1);
    }
    return res(0);
}

ret_http(register($_REQUEST));