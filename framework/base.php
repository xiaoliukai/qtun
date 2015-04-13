<?php
try {
    $b = defined(__DIR__);
    if ($b) define('ROOT', __DIR__.'/../');
    else define('ROOT', dirname(__FILE__).'/../');
} catch (Exception $e) {
    define('ROOT', dirname(__FILE__).'/../');
}