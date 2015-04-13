$(function() {
    var pagger = $('#pagger').init_pagger(
        $('#pagger').attr('page'),
        $('#pagger').attr('total'),
        $('#pagger').attr('per'),
        function (page) {
            call('/api/news/change_page.php', {
                page: page
            }, function(data) {
                $('.panel-group').empty();
                $.each(data.data.rows, function(row) {
                    var row = data.data.rows[row];
                    var str = '<div class="panel panel-default">'+
                                   '<div class="panel-heading" style="cursor:pointer" data-toggle="collapse" data-target="#news_'+row['id']+'">'+
                                       '<h4 class="panel-title message-date-parent">'
                                           +row['title']+
                                           '<div class="pull-right message-date">'+row['updated']+'</div>'+
                                       '</h4>'+
                                   '</div>'+
                                   '<div id="news_'+row['id']+'" class="panel-collapse collapse">'+
                                       '<div class="panel-body">'+
                                           '<div>'+row['content']+'</div>'+
                                           '<div class="pull-right">'+
                                               '<a href="/news.php?id='+row['id']+'">Read More</a>'+
                                           '</div>'+
                                       '</div>'+
                                   '</div>'+
                               '</div>';
                    $('.panel-group').append($(str));
                });
            });
        }
    );
});