<?php require dirname(__FILE__).'/framework/base.php'; ?>
<?php require ROOT.'/lib/base.php'; ?>
<?php
$salt = uuid(8);
setcookie('salt', $salt);
$_SESSION['salt'] = $salt;
?>
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
        <link rel="stylesheet" href="/css/loading.css">
    </head>
    <body>
        <?php require ROOT.'/framework/header.php'; ?>
        <div class="container">
            <div class="form-horizontal">
                <div class="form-group">
                    <label class="col-sm-2 control-label"><font color="red">*</font>User:</label>
                    <div class="col-sm-5">
                        <input id="user" type="text" class="form-control"></input>
                    </div>
                    <div class="col-sm-3" style="padding-left:0">
                        <div id="user-check-group" style="height:34px;display:block;padding:6px 12px;"></div>
                    </div>
                </div>
                <!-- user -->
                <div class="form-group">
                    <label class="col-sm-2 control-label"><font color="red">*</font>Password:</label>
                    <div class="col-sm-5">
                        <input id="pass1" type="password" class="form-control"></input>
                    </div>
                </div>
                <!-- pass1 -->
                <div class="form-group">
                    <label class="col-sm-2 control-label"><font color="red">*</font>Retype:</label>
                    <div class="col-sm-5">
                        <input id="pass2" type="password" class="form-control"></input>
                    </div>
                    <div class="col-sm-3" style="padding-left:0">
                        <div id="pass-check-group" style="height:34px;display:block;padding:6px 12px;"></div>
                    </div>
                </div>
                <!-- pass2 -->
                <div class="form-group">
                    <label class="col-sm-2 control-label">Email:</label>
                    <div class="col-sm-5">
                        <input id="email" type="email" class="form-control"></input>
                    </div>
                </div>
                <!-- email -->
                <div class="form-group">
                    <div class="col-sm-2"></div>
                    <div class="col-sm-5">
                        <div class="checkbox">
                            <label>
                                <input id="recvmsg" type="checkbox" checked> Receive Message
                            </label>
                        </div>
                    </div>
                </div>
                <!-- recv-message -->
                <div class="form-group">
                    <div class="col-sm-offset-2 col-sm-5">
                        <div class="center-block" style="width:200px">
                            <button id="register" class="btn btn-default">Register</button>&nbsp;
                            <a href="/login.php" class="btn btn-primary">Login</a>
                        </div>
                    </div>
                </div>
                <!-- commit -->
            </div>
        </div>
        <?php require ROOT.'/framework/footer.php'; ?>
        <script src="/3rdParty/jquery2/jquery-2.1.3.min.js"></script>
        <script src="/3rdParty/plugin/jquery.crypt.js"></script>
        <script src="/3rdParty/plugin/jquery.cookie-1.4.1.min.js"></script>
        <script src="/3rdParty/bootstrap3/js/bootstrap.min.js"></script>
        <script src="/js/lib/common.js"></script>
        <script src="/js/register.js"></script>
    </body>
</html>