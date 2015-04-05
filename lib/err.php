<?php
function err_handler($errno, $errstr, $errfile, $errline, $errcontext) {
    var_dump($errno, $errstr, $errfile, $errline);
}

function exp_handler($e) {
    var_dump($e);
}

function shutdown_handler() {
    $err = error_get_last();
    if ($err['type'] == E_ERROR) var_dump($err);
}

register_shutdown_function('shutdown_handler');
set_error_handler('err_handler');
set_exception_handler('exp_handler');
ini_set('display_errors', 'off');
error_reporting(E_ALL);