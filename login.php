<?php require dirname(__FILE__).'/lib/base.php'; ?>
<html>
    <head>
        <meta charset="utf-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="width=device-width, user-scalable=no">

        <title>QTun Project</title>

        <link rel="stylesheet" href="/3rdParty/bootstrap3/css/bootstrap.min.css">
        <link rel="stylesheet" href="/3rdParty/bootstrap3/css/bootstrap-theme.min.css">
        <link rel="stylesheet" href="/3rdParty/Font-Awesome-3.2.1/css/font-awesome.min.css">
        <link rel="stylesheet" href="/css/common.css">
    </head>
    <body>
        <?php require ROOT.'/framework/header.php'; ?>
        <div class="container">
            <div class="panel panel-default">
                <div class="panel-heading">Register</div>
                <div class="panel-body">
                    <div class="row">
                        <div class="col-sm-9">
                            <div class="form-horizontal">
                                <div class="form-group">
                                    <label class="col-sm-4 control-label">User:</label>
                                    <div class="col-sm-5">
                                        <input id="user" type="text" class="form-control"></input>
                                    </div>
                                </div>
                                <!-- user -->
                                <div class="form-group">
                                    <label class="col-sm-4 control-label">Password:</label>
                                    <div class="col-sm-5">
                                        <input id="password" type="password" class="form-control"></input>
                                    </div>
                                    <div class="col-sm-3" style="padding-left:0;width:200px">
                                        <div style="height:34px;display:block;padding-top:6px;">
                                            <a href="#">Forgot Password?</a>
                                        </div>
                                    </div>
                                </div>
                                <!-- password -->
                                <div class="form-group">
                                    <div class="col-sm-offset-4 col-sm-5">
                                        <div class="center-block" style="width:200px">
                                            <button id="login" class="btn btn-default">Login</button>&nbsp;
                                            <a href="/register.php" class="btn btn-primary">Register</a>
                                        </div>
                                    </div>
                                </div>
                                <!-- commit -->
                            </div>
                            <!-- form -->
                        </div>
                        <!-- left -->
                        <div class="col-sm-3">
                            <img src="/img/reg_code.png" style="width:150px"></img>
                            <div style="padding-top:5px">&nbsp;&nbsp;Thanks for using qtun</div>
                        </div>
                        <!-- right -->
                    </div>
                    <!-- row -->
                </div>
                <!-- panel-body -->
            </div>
            <!-- panel -->
        </div>
        <script src="/3rdParty/jquery2/jquery-2.1.3.min.js"></script>
        <script src="/3rdParty/bootstrap3/js/bootstrap.min.js"></script>
        <script src="/3rdParty/plugin/jquery.crypt.js"></script>
        <script src="/js/lib/common.js"></script>
        <script src="/js/login.js"></script>
        <?php require ROOT.'/framework/footer.php'; ?>
    </body>
</html>