<?php
require dirname(__FILE__).'/../../lib/base.php';

function change_page($params) {
    define('PERCOUNT', 25);

    $page = get($params, 'page') - 1;
    if ($page < 0) $page = 0;

    $db = conn();
    $ret = array();

    $sql = 'SELECT COUNT(1) FROM news';
    list($total) = $db->query($sql)->fetch(PDO::FETCH_NUM);

    $sql = 'SELECT id, updated, title, LEFT(content, 200) AS content FROM news ORDER BY updated DESC LIMIT '.($page * PERCOUNT).', '.PERCOUNT;
    return res(0, array(
        'rows' => $db->query($sql)->fetchAll(PDO::FETCH_ASSOC),
        'total' => $total
    ));
}

ret_http(change_page($_REQUEST));