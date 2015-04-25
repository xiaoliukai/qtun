<?php
define('PERCOUNT', 25);
$db = conn();

$id = get($_REQUEST, 'id', '');

if ($id === '') {
    if (isset($_SESSION['news.page'])) {
        $page = $_SESSION['news.page'];
    } else {
        $page = 0;
    }
    $header = '<h2>
                    News
                    <div class="col-lg-3 pull-right" style="padding-right:0;padding-left:0;margin-bottom:5px">
                        <div class="input-group">
                        <input type="text" class="form-control" placeholder="Search for...">
                            <span class="input-group-btn">
                                <button class="btn btn-default" type="button">Go!</button>
                            </span>
                        </div>
                    </div>
                </h2>';
    $sql = 'SELECT COUNT(1) FROM news';
    list($total) = $db->query($sql)->fetch(PDO::FETCH_NUM);
    $sql = 'SELECT id, updated, title, LEFT(content, 200) AS content FROM news ORDER BY updated DESC LIMIT '.($page * PERCOUNT).', '.PERCOUNT;
    $str = '<div class="panel-group" style="margin-bottom:0">';
    foreach ($db->query($sql) as $row) {
        $str .= '<div class="panel panel-default">
                    <div class="panel-heading" style="cursor:pointer" data-toggle="collapse" data-target="#news_'.$row['id'].'">
                        <h4 class="panel-title message-date-parent">'
                            .$row['title'].
                            '<div class="pull-right message-date">'.$row['updated'].'</div>
                        </h4>
                    </div>
                    <div id="news_'.$row['id'].'" class="panel-collapse collapse">
                        <div class="panel-body">
                            <div>'.$row['content'].'</div>
                            <div class="pull-right">
                                <a href="/news.php?id='.$row['id'].'">Read More</a>
                            </div>
                        </div>
                    </div>
                </div>';
    }
    $str .= '</div>
             <div class="pull-right">
                 <div id="pagger" page="'.($page + 1).'" total="'.$total.'" per="'.PERCOUNT.'"></div>
             </div>';
    $body = $str;
    $footer = '';
} else {
    $sql = 'SELECT title, content, updated FROM news WHERE id='.$id;
    $post = $db->query($sql)->fetch(PDO::FETCH_ASSOC);
    $header = '<h2 class="bottom-line message-date-parent">'
                  .$post['title'].
                  '<div class="pull-right message-date">'
                      .$post['updated'].
                  '</div>
               </h2>';
    $body = '<div class="bottom-line">
                 <div class="row">
                     <div class="col-lg-12">
                         <div>'.$post['content'].'</div>
                         <div class="pull-right commited-by">Commited By lwch</div>
                     </div>
                 </div>
             </div>';
    if (isset($_SESSION['_t'])) {
      $footer = '<div style="margin-top:10px;margin-bottom:10px">
                     <div class="row">
                         <div class="col-lg-6">
                             <textarea class="form-control" rows="10"></textarea>
                         </div>
                     </div>
                     <div class="row">
                         <div class="col-lg-1" style="margin-top:10px">
                             <input id="reply-news" class="btn btn-default" type="button" value="Reply"></input>
                         </div>
                     </div>
                 </div>';
    } else {
      $footer = '<div style="margin-top:10px;margin-bottom:10px">
                     <a href="/register.php">Register</a>&nbsp;
                     <a href="/login.php">Login</a>
                 </div>';
    }
}