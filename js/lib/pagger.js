$(function() {
    var pagger_obj = {
        max_pages: 5, // 最多显示5页
        init: function($this) {
            var str = '';
            str += '<nav><ul class="pagination">';
            var pages = Math.ceil($this.total / $this.per);
            var half = Math.floor(pagger_obj.max_pages / 2);
            var start = $this.page - half;
            if (start + half > pages) start = pages - pagger_obj.max_pages;
            if (start < 1) start = 1;
            str += '<li id="head" title="move to head"><span>&laquo;</span></li>';
            for (var i = start; i <= pages; ++i) {
                var cls = '';
                if ($this.page == i) cls = 'active';
                str += '<li class="' + cls + '"><span type="page">' + i + '</span></li>';
            }
            str += '<li id="tail" title="move to tail"><span>&raquo;</span></li>';
            str += '</ul></nav>';
            var page = $(str);
            page.find('#head').click(function(event) {
                pagger_obj.move_head($this)
            });
            page.find('#tail').click(function(event) {
                pagger_obj.move_tail($this)
            });
            page.on('click', 'li:not(.active)>span[type=page]', function(event) {
                pagger_obj.on_changed($this, this);
            });
            page.appendTo($this.obj);
            pagger_obj.update_changes($this);
        },
        on_changed: function($this, a) {
            var to = parseInt($(a).text());
            var li = $this.obj.find('li.active');

            $this.page = to;
            li.attr('class', '');
            $(a).parent().attr('class', 'active');
            pagger_obj.update_changes($this);

            $this.changed(to);
        },
        update_changes: function($this) {
            $this.obj.find('#head').removeClass('disabled');
            $this.obj.find('#tail').removeClass('disabled');
            if ($this.page == 1) $this.obj.find('#head').addClass('disabled');
            var pages = Math.ceil($this.total / $this.per);
            if ($this.page == pages) $this.obj.find('#tail').addClass('disabled');
        },
        move_head: function($this) {
            if ($this.page == 1) return;
            $this.page = 1;
            $this.obj.empty();
            pagger_obj.init($this);
            $this.changed(1);
        },
        move_tail: function($this) {
            var pages = Math.ceil($this.total / $this.per);
            if ($this.page == pages) return;
            $this.page = pages;
            $this.obj.empty();
            pagger_obj.init($this);
            $this.changed(pages);
        }
    };
    $.fn.init_pagger = function(page, total, per, changed) {
        $this = this;
        var ret = $.extend(true, {
            obj: $this,
            page: parseInt(page),
            total: parseInt(total),
            per: parseInt(per),
            changed: typeof(changed) == 'function' ? changed : function(page) {
            }
        }, pagger_obj);
        ret.init(ret);
        return ret;
    };
});