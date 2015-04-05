function call(url, params, callback) {
    if (url == undefined) {
        console.error('Missing url');
        return;
    }
    params = typeof(params) == 'object' ? params : {};
    callback = typeof(callback) == 'function' ? callback : function(data) {
    };
    $.ajax({
        url: url,
        type: 'POST',
        dataType: 'json',
        data: params,
        success: function(data) {
            switch (data.stat) {
            case 404:
                console.error('Undefined ' + url);
                return;
            case 500:
                console.error('Internal Server Error');
                return;
            }
            callback(data);
        }
    });
}