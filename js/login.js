$(function() {
    page.init();
});

var page = {
    init: function() {
        $('#login').click(function() {
            if (!page.check_input()) return;
            var md5 = $().crypt({
                method: "md5",
                source: $('#password').val()
            });
            call('/api/login/login.php', {
                user: $('#user').val(),
                pass: md5
            }, function(data) {
                if (data.stat == 0) {
                    window.location.href = '/index.php';
                } else /* if (data.stat == 1) */ {
                    show_alert('Error', 'Invalid username or password');
                }
            });
        });
    },
    check_input: function() {
        if ($('#user').val().length == 0) {
            show_alert('Error', 'Please input your username');
            return false;
        }
        if ($('#password').val().length == 0) {
            show_alert('Error', 'Please input your password');
            return false;
        }
        return true;
    }
};