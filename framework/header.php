<?php
function in_page($name) {
    $config = array(
        'home' => array('index.php', 'register.php', 'login.php'),
        'documents' => array('documents.php'),
        'news' => array('news.php'),
        'download' => array('download.php'),
        'about' => array('about.php')
    );
    $file = basename($_SERVER['SCRIPT_FILENAME']);
    echo ((isset($config[$name]) and in_array($file, $config[$name])) ? 'active' : '');
}
?>

<nav class="navbar navbar-default navbar-fixed-top">
    <div class="container">
        <div class="navbar-header">
            <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target=".navbar-collapse">
                <span class="icon-bar"></span>
                <span class="icon-bar"></span>
                <span class="icon-bar"></span>
            </button>
            <a class="navbar-brand" href="http://www.qtun.org/">QTun</a>
        </div>
        <div class="collapse navbar-collapse">
            <ul class="nav navbar-nav">
                <li class="<?php in_page('home'); ?>"><a href="http://www.qtun.org/">Home</a></li>
                <li class="<?php in_page('blog'); ?>"><a href="http://blog.q-devel.com">Blog</a></li>
                <li class="<?php in_page('documents'); ?>"><a href="/documents.php">Documents</a></li>
                <li class="<?php in_page('news'); ?>"><a href="/news.php">News</a></li>
                <li class="<?php in_page('download'); ?>"><a href="/download.php">Download</a></li>
                <li class="<?php in_page('about'); ?>"><a href="/about.php">About</a></li>
            </ul>
            <ul class="nav navbar-nav navbar-right">
                <li class="dropdown">
                    <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-expanded="false"><i class="icon-user"></i> <span class="caret"></span></a>
                    <ul class="dropdown-menu" role="menu">
                    <?php
                        if (isset($_SESSION['_t'])) {
                    ?>
                        <li><a href="/index.php?logout">Logout</a></li>
                    <?php
                        } else {
                    ?>
                        <li><a href="register.php">Register</a></li>
                        <li><a href="login.php">Login</a></li>
                    <?php
                        }
                    ?>
                    </ul>
                </li>
            </ul>
        </div>
    </div>
</nav>