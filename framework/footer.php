<footer>
    <div class="container">
        &copy; 2015 QTun Network Organization, Powered by <a href="mailto:lwch748@gmail.com">lwch</a>
    </div>
</footer>
<script>
$(function() {
    $('#__logout__').click(function() {
        call('/api/logout.php', {}, function() {
            $('#user-menu').empty();
            $('#user-menu').append($('<li><a href="register.php">Register</a></li><li><a href="login.php">Login</a></li>'));
        });
    });
})
</script>