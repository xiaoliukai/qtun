<?php require dirname(__FILE__).'/framework/base.php'; ?>
<?php require ROOT.'/lib/base.php'; ?>
<html>
    <head>
        <meta charset="utf-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="width=device-width, user-scalable=no">

        <title>QTun Project</title>

        <link rel="stylesheet" href="/3rdParty/bootstrap3/css/bootstrap.min.css">
        <link rel="stylesheet" href="/3rdParty/bootstrap3/css/bootstrap-theme.min.css">
        <link rel="stylesheet" href="/css/common.css">
    </head>
    <body>
        <?php require ROOT.'/framework/header.php'; ?>
        <div class="container">
            <?php require ROOT.'/modules/news/load.php'; ?>
            <div class="row">
                <?php global $header;echo $header; ?>
            </div>
            <div class="row">
                <?php global $body;echo $body; ?>
            </div>
            <div class="row">
                <?php global $footer;echo $footer; ?>
            </div>
        </div>
        <?php require ROOT.'/framework/footer.php'; ?>
        <script src="/3rdParty/jquery2/jquery-2.1.3.min.js"></script>
        <script src="/3rdParty/bootstrap3/js/bootstrap.min.js"></script>
        <script src="/js/lib/common.js"></script>
        <script src="/js/lib/pagger.js"></script>
        <script src="/js/news.js"></script>
    </body>
</html>