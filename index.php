<?php require dirname(__FILE__).'/framework/base.php'; ?>
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
        <div class="page-header">
            <h2>Welcome to the QTun Project</h2>
        </div>
        <!-- page-header -->
        <div class="row">
           <div class="col-md-12">
               <div class="well">
                   The QTun project is a light weight VPN software for <font class="system">like Unix/Linux</font>, <font class="system">Windows</font>, <font class="system">Android</font> and <font class="system">MacOS</font>. In order to protect the data, we are using <font class="protection">AES</font> and <font class="protection">DES</font> algorithm to encrypt the message, and we are using <font class="protection">GZIP</font> compression algorithm to compress the message. All of the code is open source in <a href="https://github.com/lwch/qtun" target="_blank">github.com</a> or <a href="https://coding.net/u/lwch/p/qtun/git" target="_blank">coding.net</a>.<br /><br />
                   QTun is using the <font class="system">tun/tap</font> driver to make a virtual network adapter. But in windows there was no <font class="system">tun/tap</font> driver here, so I made a <a href="https://github.com/lwch/qtun_sys" target="_blank">driver</a> using NDIS framework like <a href="https://github.com/OpenVPN/tap-windows" target="_blank">tap-windows</a> from OpenVPN. When you install the driver, it will create a virtual adapter in network connections named "local connectionX". In this time, I haven't signed the driver, so you can only using it in all of 32bit Windows, and 64bit Windows in <strong>test mode</strong>.
               </div>
           </div>
       </div>
       <!-- row -->
       <div class="page-header">
           <h2>Stable Versions</h2>
       </div>
       <!-- page-header -->
       <div class="row">
           <div class="col-md-6">
              <table class="table table-bordered">
                 <thead><tr>
                    <td width="25%">Date</td>
                    <td width="25%">Version</td>
                    <td>Description</td>
                </tr></thead>
                <tbody>
                    <tr>
                       <td>2015-04-04</td>
                       <td>1.0</td>
                       <td>Any description</td>
                   </tr>
               </tbody>
           </table>
           <!-- table -->
       </div>
   </div>
   <!-- row -->
   <div class="page-header">
       <h2>Tested</h2>
   </div>
   <!-- page-header -->
   <div class="row">
       <div class="col-md-12">
          <table id="table-status" class="table table-bordered">
             <thead><tr>
                <td width="13%">Date</td>
                <td width="20%">System</td>
                <td width="10%">Version</td>
                <td width="5%">Server</td>
                <td width="5%">Client</td>
                <td>Description</td>
            </tr></thead>
            <tbody>
                <tr>
                    <td>2015-4-4</td>
                    <td>Ubuntu 14.04(64bit)</td>
                    <td>1.0</td>
                    <td><font class="icon-coffee"></font></td>
                    <td><font class="icon-ok status-ok"></font></td>
                    <td>/</td>
                </tr>
                <!-- Ubuntu -->
                <tr>
                   <td>2015-4-4</td>
                   <td>Linux Mint 17.1(64bit)</td>
                   <td>1.0</td>
                   <td><font class="icon-coffee"></font></td>
                   <td><font class="icon-ok status-ok"></font></td>
                   <td>/</td>
               </tr>
               <!-- Linux Mint -->
               <tr>
                   <td>2015-4-4</td>
                   <td>CentOS 7(64bit)</td>
                   <td>1.0</td>
                   <td><font class="icon-ok status-ok"></font></td>
                   <td><font class="icon-coffee"></font></td>
                   <td>/</td>
               </tr>
               <!-- CentOS 7 -->
               <tr>
                   <td>2015-4-4</td>
                   <td>CentOS 6.6(64bit)</td>
                   <td>1.0</td>
                   <td><font class="icon-ok status-ok"></font></td>
                   <td><font class="icon-coffee"></font></td>
                   <td>/</td>
               </tr>
               <!-- CentOS 6.6 -->
               <tr>
                   <td>2015-4-4</td>
                   <td>MacOS 10.10.2</td>
                   <td>1.0</td>
                   <td><font class="icon-coffee"></font></td>
                   <td><font class="icon-ok status-ok"></font></td>
                   <td>/</td>
               </tr>
               <!-- MacOS -->
               <tr>
                   <td>2015-4-4</td>
                   <td>Windows Server 2003</td>
                   <td>1.0</td>
                   <td><font class="icon-ok status-ok"></font></td>
                   <td><font class="icon-ok status-ok"></font></td>
                   <td>/</td>
               </tr>
               <!-- Win2003 -->
               <tr>
                   <td>2015-4-4</td>
                   <td>Windows 7(64bit)</td>
                   <td>1.0</td>
                   <td><font class="icon-coffee"></font></td>
                   <td><font class="icon-ok status-ok"></font></td>
                   <td>/</td>
               </tr>
               <!-- CentOS 6.6 -->
               <tr>
                   <td colspan="6">
                      <font class="icon-coffee"></font> means waiting for test, <font class="icon-ok status-ok"></font> means test ok.
                  </td>
              </tr>
          </tbody>
      </table>
  </div>
</div>
<!-- row -->
</div>
<?php require ROOT.'/framework/footer.php'; ?>
<script src="/3rdParty/jquery2/jquery-2.1.3.min.js"></script>
<script src="/3rdParty/bootstrap3/js/bootstrap.min.js"></script>
</body>
</html>