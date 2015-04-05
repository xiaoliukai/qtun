<?php
require dirname(__FILE__).'/../framework/base.php';
require ROOT.'/lib/base.php';

$module = get($_REQUEST, 'module');
$action = get($_REQUEST, 'action');

$apis = array(
    'news' => array(
        'change_page' => function($params) {
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
    )
);

if (isset($apis[$module]) and isset($apis[$module][$action])) {
    echo json_encode($apis[$module][$action]($_REQUEST));
} else {
    echo '{"stat":404}';
}