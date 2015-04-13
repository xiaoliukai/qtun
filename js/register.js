$(function() {
    page.init();
});

var page = {
    init: function() {
        $('#user').blur(function(event) {
            $('#user-check-group').empty();
            if ($('#user').val().length == 0) {
                $('#user-check-group').append('<font style="margin-top:8px">please input your username</font>');
                return;
            }
            $('#user-check-group').append($('<i id="user-loading" class="icon-spinner icon-spin"></i>'))
            call('/api/register/check_exists.php', {
                name: $('#user').val()
            }, function(data) {
                $('#user-loading').remove();
                if (data.stat == 0) {
                    $('#user-check-group').append('<i class="icon-ok" style="color:green"></i>')
                } else {
                    $('#user-check-group').append('<font color="red" style="margin-top:8px">user exists</font>');
                }
            }, function(data) {
                $('#user-loading').remove();
                $('#user-check-group').append('<font color="red" style="margin-top:8px">user exists</font>');
            }, false);
        });
        $('#pass2').blur(function(event) {
            $('#pass-check-group').empty();
            if ($('#pass2').val() != $('#pass1').val()) {
                $('#pass-check-group').append('<font color="red" style="margin-top:8px">two password are not same</font>');
            }
        });
        $('#register').click(function(event) {
            if (!page.check_input()) return;
            var md5 = $().crypt({
                method: "md5",
                source: $('#pass1').val() + $.cookie('salt')
            });
            call('/api/register/register.php', {
                user: $('#user').val(),
                pass: md5,
                email: $('#email').val(),
                recvmsg: $('#recvmsg:checkbox:checked').val() == 'on' ? 1 : 0
            }, function(data) {
                if (data.stat == 0) {
                    show_alert('Success', 'Thank you for register', function() {
                        window.location.href = '/login.php';
                    });
                    return;
                }
                if (data.stat == 1) {
                    $('#user-check-group').empty();
                    $('#user-check-group').append('<font color="red" style="margin-top:8px">user exists</font>');
                }
            });
        });
    },
    check_input: function() {
        if ($('#user').val().length == 0) {
            show_alert('Error', 'Please input username');
            return false;
        }
        if ($('#pass1').val().length == 0 || $('#pass2').val().length == 0) {
            show_alert('Error', 'Please input password');
            return false;
        }
        if ($('#email').val().length) {
            if (!/^[a-z]([a-z0-9]*[-_]?[a-z0-9]+)*@([a-z0-9]*[-_]?[a-z0-9]+)+[\.][a-z]{2,3}([\.][a-z]{2})?$/.test($('#email').val())) {
                show_alert('Error', 'Please input a valid email address');
                return false;
            }
        }
        return true;
    }
};