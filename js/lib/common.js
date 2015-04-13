function call(url, params, callback, error, loading) {
    if (url == undefined) {
        console.error('Missing url');
        return;
    }
    params = typeof(params) == 'object' ? params : {};
    callback = typeof(callback) == 'function' ? callback : function(data) {
    };
    error = typeof(error) == 'function' ? error : function(data) {
    };
    loading = loading === undefined ? true : loading;
    if (loading) show_loading();
    $.ajax({
        url: url,
        type: 'POST',
        dataType: 'json',
        data: params,
        success: function(data) {
            switch (data.stat) {
            case 404:
                if (loading) hide_loading();
                show_alert('Error', 'Undefined api');
                error(data);
                return;
            case 500:
                if (loading) hide_loading();
                show_alert('Error', 'Internal Server Error');
                error(data);
                return;
            }
            if (loading) hide_loading();
            callback(data);
        }
    });
}

function show_alert(title, content, callback) {
    title = (title === undefined) ? 'Message' : title;
    content = (content === undefined) ? '' : content;
    callback = (callback === undefined) ? function() {
    } : callback;
    var str = '<div class="modal fade" id="__show__alert__">'+
              '<div class="modal-dialog">'+
                '<div class="modal-content">'+
                  '<div class="modal-header">'+
                    '<button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>'+
                    '<h4 class="modal-title">' + title + '</h4>'+
                  '</div>'+
                  '<div class="modal-body">' + content + '</div>'+
                  '<div class="modal-footer">'+
                    '<button type="button" class="btn btn-default" data-dismiss="modal">Close</button>'+
                  '</div>'+
                '</div>'+
              '</div>'+
            '</div>';
    $(document.body).append($(str));
    $('#__show__alert__').on('show.bs.modal', function(e) {
        var $clone = $(this).clone().css('display', 'block').appendTo('body');
        var top = Math.round(($clone.height() - $clone.find('.modal-content').height()) / 2);
        top = top > 0 ? top : 0;
        $clone.remove();
        $(this).find('.modal-content').css('margin-top', top);
    });
    $('#__show__alert__').on('hidden.bs.modal', function(e) {
        $('#__show__alert__').remove();
        callback();
    });
    $('#__show__alert__').modal();
}

function show_loading() {
    var str = '<div class="__loading__"><div class="group">'+
                  '<div class="icon1"></div>'+
                  '<div class="icon2"></div>'+
              '</div></div>'+
              '<div class="global-mask"></div>';
    var obj = $(str);
    var $clone = obj.clone().css('display', 'block').appendTo('body');
    var top = Math.round(($clone.height() - $clone.find('.group').height()) / 2);
    var left = Math.round(($clone.width() - $clone.find('.group').width()) / 2);
    top = top > 0 ? top : 0;
    $clone.remove();
    obj.find('.group').css({
        'margin-left': left,
        'margin-top': top
    });
    $(document.body).append(obj);
}

function hide_loading() {
    $(document.body).find('.__loading__').remove();
    $(document.body).find('.global-mask').remove();
}