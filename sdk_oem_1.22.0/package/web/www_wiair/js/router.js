/* router.js */
$(function () {
    /*************** 首页 **********************/
    $("#login_pwd").keyup(function (event) {
        e = event ? event : (window.event ? window.event : null);
        if (e.keyCode == 13) {
            userLogin();
        }
    });




    $("#btn").click(function () {
        var loadingLay = layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            shadeClose: false,
            content: '<div class="loading"><p><img src="images/loading-1.gif" width="120"></p><p>正在检测上网方式...</p></div>',
            skin: 'cy-class'
        });
        setTimeout(function () {
            layer.close(loadingLay);
            //window.location.href = "internetType.html";
            /*未插网线*/
            layer.open({
                type: 1,
                title: false,
                shade: [0.7, '#000'],
                closeBtn: false,
                shadeClose: false,
                content: $("#lineOff"),
                skin: 'cy-class'
            });
            /*未插网线 end*/
        }, 2000)
    })
    //重新检测
    $(".img2").click(function () {
        layer.closeAll();
        var loadingLay2 = layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            shadeClose: false,
            content: '<div class="loading"><p><img src="images/loading-1.gif" width="120"></p><p>正在检测上网方式...</p></div>',
            skin: 'cy-class'
        });
        setTimeout(function () {
            layer.close(loadingLay2);
            //window.location.href = "internetType.html";
            /*未插网线*/
            layer.open({
                type: 1,
                title: false,
                shade: [0.7, '#000'],
                closeBtn: false,
                shadeClose: false,
                content: $("#lineOff"),
                skin: 'cy-class'
            });
            /*未插网线 end*/
        }, 2000)
    })
    /*************** 首页 end**********************/

    /*************** internetType **********************/
    //tab 切换
    $(".nav a").click(function () {
        $(this).addClass("selected").siblings("a").removeClass("selected");
        var id = $(this).attr("data-id");
        $("#" + id + "_box").show().siblings(".sub-box").hide();
        if (id == "t_4") {
            getWifiList();
        }
    })
    //输入框焦点
    $(".inpt").focusin(function () {
        $(this).siblings("label").hide();
    }).focusout(function () {
        if ($(this).val() == '') {
            $(this).siblings("label").show();
        }
    })
    //取消按钮
    $(".cannel").live("click", function () {
        layer.closeAll();
    })

    //克隆mac地址
    $(".mac_copy").click(function () {
        layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            content: $("#mac_addr"),
            skin: 'cy-class'
        });
    })
//    $("#mac_connect").click(function() {
//        var mac = $('input[name=mac_inpt]').val();
//        if (checkMac(mac) == true) {
//            layer.closeAll();
//            layer.msg("克隆成功!");
//        } else {
//            layer.tips("MAC地址有误！", '#mac_inpt', {
//                tips: [1, '#000000']
//            });
//            return;
//        }
//    });

    //wifi连接
    $("#wifi_box li").live("click", function () {
        var name = $(this).attr('ssid');
        $("#cyname").text(name);
        $("#wifi_pwd").val('');
        $("#channel").val('');
        $("#data_mac").val('');
        $("#data_sec").val('');
        var channel = $(this).attr('data');
        var sec = $(this).attr('sec');
        if (name.length < 1) {
            layer.msg("SSID被隐藏,请使用手动添加");
            return;
        }
        layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            content: $("#wifi_connect"),
            skin: 'cy-class'
        });
        var lock = $(this, 'span em').find('i').hasClass('lock');
        $("#channel").val(channel);
        $("#data_sec").val(sec);
        if (lock != true) {
            $("#have_pwd").addClass("hide");
        } else {
            $("#have_pwd").removeClass("hide");
        }
    });

    $("#connect").live("click", function () {
        var channel = $("#channel").val();
        var name = $("#cyname").text();
        var wifi_pwd = $("#wifi_pwd").val();
        var mac = $('#wisp_mac_inpt').val();
        var sec = $("#data_sec").val();
        var check_wpa = '';
        name = encodeURIComponent(name);
        wifi_pwd = encodeURIComponent(wifi_pwd);
        if (!$("#have_pwd").hasClass("hide")) {
            if (trim(wifi_pwd) == '') {
                layer.tips('请输入密码', '#wifi_pwd', {
                    tips: [1, '#000000']
                });
                return;
            }
            connectWisp(name, wifi_pwd, mac, channel, check_wpa, sec);
        } else {
            wifi_pwd = null;
            connectWisp(name, wifi_pwd, mac, channel, check_wpa, sec);
        }
        var loadingLay = layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            shadeClose: false,
            content: '<div class="loading"><p><img src="../images/loading-2.gif" width="120"></p><p>正在连接中</p></div>',
            skin: 'cy-class'
        });
    })


    //添加隐藏ssid
    $("#ssid_add").live("click", function () {
        layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            content: $("#ssid_connect"),
            skin: 'cy-class'
        });
    })

    //确认 添加隐藏ssid
    $("#ssid_confirm").click(function () {
        var ssid = $("#ssid").val();
        var pwd = $("#ssid_pwd").val();
        var mac = $('#wisp_mac_inpt').val();
        var channel = $("#check_channel").val();
        var check_wpa = $("#check_wpa").val();
        if (ssid == '') {
            layer.tips('请输入无线名称', '#ssid', {
                tips: [1, '#000000']
            });
            return;
        }

        if (getStrLength(ssid) > 31) {
            layer.tips('无线名称不能超过31位', '#ssid', {
                tips: [1, '#000000']
            });
            return;
        }

        if (!$("#pwd_li").hasClass("hide") && pwd.length < 1) {
            layer.tips('请输入无线密码', '#ssid_pwd', {
                tips: [1, '#000000']
            });
            return;
        }

        if (pwd.length > 0 && pwd.length < 8) {
            layer.tips('无线密码不能少于8位', '#ssid_pwd', {
                tips: [1, '#000000']
            });
            return;
        }

        var loadingLay = layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            shadeClose: false,
            content: '<div class="loading"><p><img src="../images/loading-2.gif" width="120"></p><p>正在连接中</p></div>',
            skin: 'cy-class'
        });
        connectWisp(ssid, pwd, mac, channel, check_wpa);
    })

    $("#check_wpa").live("change", function () {
        if ($(this).val() == 1) {
            $("#pwd_li").addClass("hide");
        } else {
            $("#pwd_li").removeClass("hide");
        }
    })
    /*************** internetType end**********************/



    /*************** getAccountInfo **********************/
    //获取宽带帐号密码
    $("#goGet").click(function () {
        layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            content: $("#accountLayer"),
            skin: 'cy-class'
        });
    })
    //取消按钮
    $("#cannel").click(function () {
        layer.closeAll();
    })

    $("#net_work").click(function () {
        var account = $("#account_get").text();
        var pwd = $("#pwd_get").text();
        var url = document.URL;
        if (url.indexOf('user') > 0) {
            window.location.href = 'setting.html?acc=' + account + "&pwd=" + pwd;
        } else {
            window.location.href = 'index.html?acc=' + account + "&pwd=" + pwd;
        }
    });

    /*************** getAccountInfo end**********************/

    $('#PpoeHightSet').click(function () {
        $("#t_1_box ul").find('li').each(function (index) {
            if (index > 1) {
                if ($(this).is(':visible') == false) {
                    $(this).show();
                    $("#PpoeHightSet").text('简单配置');
                } else {
                    $(this).hide();
                    $(this).find('input').val('').siblings("label").show();
                    $(this).find('select').val(3);
                    $(this).find("option:selected").text('出厂MAC');
                    if ($(this).find("span").length == 3) {
                        $(this).find("span").last().hide();
                    }
                    $("#PpoeHightSet").text('高级配置');
                }
            }
        });
    })

    $('#StaticHightSet').click(function () {
        $("#t_2_box ul").find('li').each(function (index) {
            if (index > 2) {
                if ($(this).is(':visible') == false) {
                    $(this).show();
                    $("#StaticHightSet").text('简单配置');
                } else {
                    $(this).hide();
                    $(this).find('input').val('').siblings("label").show();
                    $(this).find('select').val(3);
                    $(this).find("option:selected").text('出厂MAC');
                    if ($(this).find("span").length == 3) {
                        $(this).find("span").last().hide();
                    }
                    $("#StaticHightSet").text('高级配置');
                }
            }
        });
    })

    $('#DhcpHightSet').click(function () {
        $("#t_3_box ul").find('li').each(function (index) {
            if ($(this).is(':visible') == false) {
                $(this).show();
                $("#DhcpHightSet").text('简单配置');
            } else {
                $(this).hide();
                $(this).find('input').val('').siblings("label").show();
                $(this).find('select').val(3);
                $(this).find("option:selected").text('出厂MAC');
                if ($(this).find("span").length == 3) {
                    $(this).find("span").last().hide();
                }
                $("#DhcpHightSet").text('高级配置');
            }
        });
    })

})