<?php
try {
    $b = defined(__DIR__);
    if ($b) define('ROOT', __DIR__.'/../');
    else define('ROOT', dirname(__FILE__).'/../');
} catch (Exception $e) {
    define('ROOT', dirname(__FILE__).'/../');
}

require ROOT.'/lib/err.php';
require ROOT.'/conf/conf.php';
session_start();
date_default_timezone_set(TIME_ZONE);

function conn() {
    static $conn;
    if (empty($conn)) {
        $dsn = 'mysql:dbname='.DB_NAME.';host='.DB_HOST;
        $conn = new PDO($dsn, DB_USER, DB_PASS, array(
            PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
            PDO::MYSQL_ATTR_INIT_COMMAND => 'SET NAMES UTF8'
        ));
    }
    return $conn;
}

function get($params, $key, $def = null) {
    if (isset($params[$key])) return $params[$key];
    if ($def === null) throw new Exception('Missing param ['.$key.']');
    return $def;
}

function res($stat, $data = null) {
    $ret = array('stat' => $stat);
    if ($data !== null) $ret['data'] = $data;
    return $ret;
}

function uuid($len) {
    $charmap = '`1234567890~!@#$%^&*()-=_+abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $ret = '';
    for ($i = 0; $i < $len; $i++) {
        $ret .= substr($charmap, mt_rand(0, 233) % 78, 1);
    }
    return $ret;
}

function ret_http($ret) {
    echo json_encode($ret);
    exit;
}