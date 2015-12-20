/* router.js */
$(function() {
    /*************** 首页 **********************/
    $("#btn").click(function() {
        var loadingLay = layer.open({
            type: 1,
            shadeClose: false,
            content: '<div class="loading"><p><img src="../images/loading-1.gif"></p><p>正在检测上网方式...</p></div>',
            style: 'background-color: transparent;  box-shadow: none;'
        });
        setTimeout(function() {
            layer.close(loadingLay);
            $("#wrap_2").show().siblings(".indexWrap").hide();
            /*未插网线
             layer.open({
             type: 1,
             shadeClose:false,
             content: $("#lineOff").html(),
             style:'width:90%;background-color: transparent;  box-shadow: none;'
             });  */
            /*未插网线 end*/
        }, 2000)
    })
    //重新检测
    $(".img2").live("click", function() {
        layer.closeAll();
        var loadingLay2 = layer.open({
            type: 1,
            shadeClose: false,
            content: '<div class="loading"><p><img src="../images/loading-1.gif"></p><p>正在检测上网方式...</p></div>',
            style: 'background-color: transparent;  box-shadow: none;'
        });
        setTimeout(function() {
            layer.close(loadingLay2);
            //window.location.href = "internetType.html";
            /*未插网线*/
            layer.open({
                type: 1,
                shadeClose: false,
                content: $("#lineOff").html(),
                style: 'width:90%;background-color: transparent;  box-shadow: none;'
            });
            /*未插网线 end*/
        }, 2000)
    })

    /*首页登录*/
    $("#confirm").click(function() {
        if ($("#login_pwd").val() == '') {
            tips("请输入密码");
            return;
        }
        $("#wrap_2").show().siblings(".wrap").hide();
    });

    $("ul>li").find('a').click(function() {
        var _index = $("ul > li a").index(this);
        if (_index == 0) {
            document.location = 'http://' + document.domain + "/m/user/setWifi.html?tt=" + new Date().getTime();
        } else if (_index == 1) {
            document.location = 'http://' + document.domain + "/m/user/setNet.html?tt=" + new Date().getTime();
        } else if (_index == 2) {
            document.location = 'http://' + document.domain + "/m/user/online.html?tt=" + new Date().getTime();
        } else if (_index == 3) {
            document.location = 'http://' + document.domain + "/m/user/more.html?tt=" + new Date().getTime();
        }
    });

    $(".macChoose").change(function() {
        var v = $(this).val();
        var typeId = $(this).attr('id');
        var Id = '';
        if (typeId == 'macChoose1') {
            Id = 1;
        } else if (typeId == 'macChoose2') {
            Id = 2;
        } else if (typeId == 'macChoose3') {
            Id = 3;
        }

        var macInpt = $(this).parent().siblings(".macInpt");
        if (v == 1) {
            macInpt.show();
            $(this).parent().siblings(".last").hide();
            $("#mac_box" + Id).hide();
        } else if (v == 2) {
            macInpt.hide();
            getHostRaMac(2, typeId);
        } else if (v == 3) {
            macInpt.hide();
            getHostRaMac(3, typeId);
        }
    });


    /*************** 首页 end**********************/

    /*************** internetType **********************/
    //tab 切换
    $(".nav a").click(function() {
        $(this).addClass("selected").siblings("a").removeClass("selected");
        var id = $(this).attr("data-id");
        $("#" + id + "_box").show().siblings(".sub-box").hide();
        if (id == "t_4") {
            getWifiList();
        }
    })
    //输入框焦点
    $(".inpt").live("focusin", function() {
        $(this).siblings("label").hide();
    }).live("focusout", function() {
        if ($(this).val() == '') {
            $(this).siblings("label").show();
        }
    })

    //显隐密码

    var view_pwd = $(".pwd-icon");
    var flag = 1;
    view_pwd.live("click", function() {
        if (flag == 1) {
            $(this).siblings("input[type=password]").hide();
            $(this).siblings("input[type=text]").show();
            $(this).html("<img src = '../../images/pwd-icon2.png'>");
            flag = 0;
        } else {
            $(this).siblings("input[type=password]").show();
            $(this).siblings("input[type=text]").hide();
            $(this).html("<img src = '../../images/pwd-icon.png'>");
            flag = 1;
        }
    })
    $(".inpt").live("focusout", function() {
        $(this).siblings(".inpt").val($(this).val());
    })


    //取消按钮
    $(".cannel").live("click", function() {
        layer.closeAll();
    })

    //克隆mac地址
    var html = '<div class="wifiConnectLayer" style="display:block;" id="mac_addr"><div class="bg"></div><div class="con"><p>克隆MAC地址</p><ul class="list"><li><span class="sp-l">MAC地址</span><span class="sp-r"><label for="mac_inpt">请输入MAC地址</label><input type="text" name="mac_inpt" class="inpt" id="macTy" autocomplete="off"></span></li></ul><div class="box"><a href="javascript:;" class="btn layer-btn cannel">取消</a>&nbsp;&nbsp;<a href="javascript:;" class="btn layer-btn" id="mac_connect">克隆</a></div></div></div>';
    $(".mac_copy").click(function() {
        layer.open({
            type: 1,
            shadeClose: false,
            content: html,
            style: 'width:90%;background-color: transparent;  box-shadow: none;padding:0 0 1rem 0;'
        });
        router.getWanInfo();
    })
    $("#mac_connect").live("click", function() {
        var mac = $("#macTy").val();
        if (checkMac(mac) == true) {
            layer.closeAll();
            tips("克隆成功!");
        } else {
            tips("MAC地址有误！");
            return;
        }
    })

    //宽带拨号 提交
//    $("#btn_1").click(function() {
//        if ($("#acc").val() == '') {
//            tips("请输入宽带帐号!");
//            return;
//        }
//        if ($("#pwd").val() == '') {
//            tips("请输入宽带密码!");
//            return;
//        }
//        window.location.href = 'user/index.html';
//    })

    //静态IP 提交
//    var regx = /^((25[0-5]|2[0-4]\d|[01]?\d\d?)($|(?!\.$)\.)){4}$/;
//    $("#btn_2").click(function() {
//        if (!regx.exec($("#ip_addr").val())) {
//            tips("请输入正确的IP地址");
//            return;
//        }
//        if (!regx.exec($("#subnet").val())) {
//            tips("请输入正确的子网掩码");
//            return;
//        }
//        if (!regx.exec($("#wg").val())) {
//            tips("请输入正确的网关");
//            return;
//        }
//        if (!regx.exec($("#dns").val())) {
//            tips("请输入正确的DNS");
//            return;
//        }
//        window.location.href = 'user/index.html';
//    })

    //wifi连接
    $("#mCSB_1_container li a").live("click", function() {
        var name = $(this).attr('ssid');
        var channel = $(this).attr('data');
        var sec = $(this).attr('sec');
        if (name.length < 1) {
            tips("SSID被隐藏,请使用手动添加");
            return;
        }
        layer.open({
            type: 1,
            shadeClose: false,
            content: '<div class="wifiConnectLayer" id="wifi_connect" style="display:block;"><div class="bg"></div><div class="con"><p id="cyname"></p><ul class="list" id="have_pwd"><li><span class="sp-l">WIFI密码</span><span class="sp-r"><label for="wifi_pwd">请输入WIFI密码</label><input type="password" name="wifi_pwd" class="inpt" id="wifi_pwd" autocomplete="off"></span></li></ul><div class="box"><input type="hidden" id="channel"><input type="hidden" id="data_mac"><input type="hidden" id="data_sec"><a href="javascript:;" class="btn layer-btn cannel">取消</a>&nbsp;<a href="javascript:;" class="btn layer-btn" id="connect">连接</a></div></div></div>',
            style: 'width:90%;background-color: transparent;  box-shadow: none;'
        });
        $("#cyname").text(name);
        $("#wifi_pwd").val('');
        $("#channel").val('');
        $("#data_mac").val('');
        $("#data_sec").val('');
        var lock = $(this, 'span em').find('i').hasClass('lock');
        if (lock != true) {
            $("#have_pwd").addClass("hide");
            ;
        } else {
            $("#have_pwd").removeClass("hide");
        }
        var data_mac = $(this).attr('mac');
        $("#channel").val(channel);
        $("#data_mac").val(data_mac);
        $("#data_sec").val(sec);

    })

    $("#connect").live("click", function() {
        var channel = $("#channel").val();
        var name = $("#cyname").text();
        var wifi_pwd = $("#wifi_pwd").val();
        var mac = $("#data_mac").val();
        var sec = $("#data_sec").val();
        var check_wpa = '';
        if (!$("#have_pwd").hasClass("hide")) {
            if (wifi_pwd == '') {
                getMobileMsg('请输入密码');
                return;
            }
            connectWisp(name, wifi_pwd, mac, channel, check_wpa, sec);
        } else {
            wifi_pwd = null;
            connectWisp(name, wifi_pwd, mac, channel, check_wpa, sec);
        }
        var loadingLay = layer.open({
            type: 1,
            shadeClose: false,
            content: '<div class="loading"><p><img src="../../images/loading-1.gif"></p><p>正在连接中</p></div>',
            style: 'background-color: transparent;  box-shadow: none;'
        });
    })


    //添加隐藏ssid
    $("#ssid_add").live("click", function() {
        layer.open({
            type: 1,
            shadeClose: false,
            content: '<div class="wifiConnectLayer" id="ssid_connect" style="display:block;"><div class="con"><p>添加隐藏网络</p><ul class="list"><li><span class="sp-l">网络SSID</span><span class="sp-r"><label for="ssid">请输入网络SSID</label><input type="text" name="ssid" class="inpt" id="ssid" autocomplete="off"></span></li><li><span class="sp-l">信道</span><span class="sp-r"><select id="check_channel" class="check_channel"><option>1</option><option>2</option><option>3</option><option>4</option><option>5</option><option>6</option><option>7</option><option>8</option><option>9</option><option>10</option><option>11</option><option>12</option><option>13</option></select></span></li><li><span class="sp-l">安全性</span><span class="sp-r"><select id="check_wpa" class="check_channel"><option value="1">开放</option><option>WPA PSK</option><option>WPA2 PSK</option><option selected="selected">WPAWPA2 PSK</option></select></span></li><li id="pwd_li"><span class="sp-l">密码</span><span class="sp-r"><label for="ssid_pwd">请输入密码</label><input type="password" name="ssid_pwd" class="inpt" id="ssid_pwd" autocomplete="off"></span></li></ul><div class="box"><a href="javascript:;" class="btn layer-btn cannel">取消</a>&nbsp;&nbsp;<a href="javascript:;" class="btn layer-btn" id="ssid_confirm">确认</a></div></div></div>',
            style: 'width:90%;background-color: transparent;  box-shadow: none;'
        });
    })

    //确认 添加隐藏ssid
    $("#ssid_confirm").live("click", function() {
        var ssid = $("#ssid").val();
        var pwd = $("#ssid_pwd").val();
        var mac = wanConfigJsonObject.mac;
        var channel = $("#check_channel").val();
        var check_wpa = $("#check_wpa").val();
        if ($("#ssid").val() == '') {
            tips("请输入网络SSID");
            return;
        }

        if (getStrLength(ssid).length > 31) {
            tips('无线名称长度不能超过31位');
            return;
        }

        if (!$("#pwd_li").hasClass("hide")) {
            if (trim(pwd) == '') {
                tips('请输入密码!');
                return;
            }
        }

        if (pwd.length > 0) {
            if (pwd.length > 31 || pwd.length < 8) {
                tips('密码长度不能超过31位或不能少于8位');
                return;
            }
        }


        var loadingLay = layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            shadeClose: false,
            content: '<div class="loading"><p><img src="../../images/loading-1.gif"></p><p>正在连接中</p></div>',
            style: 'background-color: transparent;  box-shadow: none;'
        });
        connectWisp(ssid, pwd, mac, channel, check_wpa);
    })
    /*************** internetType end**********************/
    $("#check_wpa").live("change", function() {
        if ($(this).val() == 1) {
            $("#pwd_li").addClass("hide");
        } else {
            $("#pwd_li").removeClass('hide');
        }
    })



    /*************** getAccountInfo **********************/
    //获取宽带帐号密码
    $("#goGet").click(function() {
        layer.open({
            type: 1,
            shadeClose: false,
            content: $("#accountLayer").html(),
            style: 'width:90%;background-color: transparent;  box-shadow: none;'
        });
    })

    //获取按钮
//    $("#getAccount").live("click", function() {
//        $(".dot").hide();
//        $(".txt").removeClass("hide").text("路由器正在获取中...");
//        $(".d2").removeClass("error");
//        $(this).val("获取中...")
//        $(this).attr("disabled", "disabled");
//        var sec = 3;
//        var interval = window.setInterval(function() {
//            sec--;
//            if (sec == 0) {
//                window.clearInterval(interval);
//
//                /*获取失败
//                 $(".dot").show();
//                 $(".d2").addClass("error");
//                 $(".txt").text("获取失败!");
//                 $(".inptBtn").removeAttr("disabled");
//                 $(".inptBtn").val("重新获取");
//                 */
//
//
//                /* 获取成功*/
//                layer.closeAll();
//                layer.open({
//                    type: 1,
//                    shadeClose: false,
//                    content: $("#sucLayer").html(),
//                    style: 'width:90%;background-color: transparent;  box-shadow: none;'
//                });
//
//
//            } else {
//                $(".d1").delay(100).fadeIn();
//                $(".d2").delay(400).fadeIn();
//                $(".d3").delay(700).fadeIn();
//                setTimeout(function() {
//                    $(".dot").fadeOut();
//                }, 1200)
//            }
//
//        }, 2000)
//    })
    /*************** getAccountInfo end**********************/
    $("#priceSet").live("click", function() {
        var priceLayer = layer.open({
            type: 1,
            content: '<div class="wifiConnectLayer" id="setAccount" style="display:block;"><div class="bg"></div><div class="con"><div class="closebtn" id="close_btn">x</div><p>购物比价设置</p><p class="newtxt">逛京东、一号店等网站时，可以自动同款比价、展示180天历史价格走势、推荐相关优惠券和促销活动消息，商品降价后还可以获得降价提醒通知。电脑、手机、平板都支持比价！</p><p><span class="navsp"><label for="r1">开启</label><input class="navsp_rad" type="radio" id="r1" name="wifi" value="1" checked></span><span class="navsp"><label for="r2">关闭</label><input class="navsp_rad" type="radio" id="r2" name="wifi" value="0"></span></p></div></div>',
            style: 'width:90%;background-color: transparent;  box-shadow: none;'
        });
        $("#close_btn").live("click", function() {
            layer.close(priceLayer);
        });
        getSetPrice('get');
    })

    //开启/关闭wifi
    $('input[name=onOff]').click(function() {
        var val = $('input[name=onOff]:checked').val();
        if (val == 1) {
            $("#wifiOnOff").show();
        } else {
            $("#wifiOnOff").hide();
        }
        wifiSet(val);
    });

    $('input[name=wifi]').live('click', function() {
        var val = $('input[name=wifi]:checked').val();
        getSetPrice('set', val);
    });


    //设置用户名 密码
    $("#user").on("click", function() {
        layer.open({
            type: 1,
            shadeClose: false,
            content: '<div class="wifiConnectLayer" id="setAccount" style="display:block;"><div class="bg"></div><div class="con"><p>管理密码设置</p><ul class="list"><li><span class="sp-l">密码</span><span class="sp-r"><label for="userpwd">请输入密码</label><input type="password" name="userpwd" class="inpt" id="userpwd" autocomplete="off"><input type="text" name="userpwd_1" id="userpwd_1" class="inpt hide" autocomplete="off"><a href="javascript:;" class="pwd-icon" id="pwd-icon1"><img src="../../images/pwd-icon.png"></a></span></li><li><span class="sp-l">确认密码</span><span class="sp-r"><label for="userpwd2">请再次输入密码</label><input type="password" name="userpwd2" class="inpt" id="userpwd2" autocomplete="off"><input type="text" name="userpwd_2" id="userpwd_2" class="inpt hide" autocomplete="off"><a href="javascript:;" class="pwd-icon" id="pwd-icon2"><img src="../../images/pwd-icon.png"></a></span></li></ul><div class="box"><a href="javascript:;" class="btn layer-btn cannel">取消</a>&nbsp;<a href="javascript:;" class="btn layer-btn" id="user_confirm">确认</a></div></div></div>',
            style: 'width:90%;background-color: transparent;  box-shadow: none;'
        });
    })

    //确认 设置用户名 密码
    $("#user_confirm").live("click", function() {
        var pwd = $("#userpwd").val();
        if (pwd == '') {
            tips("请输入密码");
            return;
        }
        if ($("#userpwd2").val() == '') {
            tips("请再次输入密码");
            return;
        }
        if ($("#userpwd2").val() != pwd) {
            tips("两次密码不一致");
            return;
        }
        if (pwd.length > 15) {
            tips('密码长度不能超过15位！');
            return;
        }

        if (escape(pwd).indexOf("%u") != -1) {
            getMobileMsg('密码不能包含中文字符！');
            return;
        }
        var str_md5 = $.md5('admin' + pwd);
        var flag = false;
        $.ajax({
            type: "POST",
            url: actionUrl + "fname=system&opt=login&function=set&usrid=" + str_md5,
            dataType: "JSON",
            async: false,
            success: function(data) {
                if (data.error == 0) {
                    flag = true;
                }
            }
        });
        if (flag == true) {
            getMobileMsg('新密码和旧密码相同！', 1, '#tips_userpwd');
            return;
        }
        layer.closeAll();
        setUserAccount(pwd);
    })

    //修改密码
    $("#changePwd").live("click", function() {
        layer.closeAll();
        layer.open({
            type: 1,
            shadeClose: false,
            content: '<div class="wifiConnectLayer" id="changePwdLay" style="display:block;"><div class="bg"></div><div class="con"><p>修改密码</p><ul class="list"><li><span class="sp-l">旧密码</span><span class="sp-r"><label for="oldpwd">请输入旧密码</label><input type="password" name="oldpwd" class="inpt" id="oldpwd" autocomplete="off"><input type="text" name="oldpwd_1" id="oldpwd_1" class="inpt" style="display: none;" autocomplete="off"><em class="pwd-icon" id="pwd-icon3"><img src="../../images/pwd-icon.png"></em></span></li><li><span class="sp-l">新密码</span><span class="sp-r"><label for="newpwd">请输入新密码</label><input type="password" name="newpwd" class="inpt" id="newpwd" autocomplete="off"><input type="text" name="newpwd_1" id="newpwd_1" class="inpt" style="display: none;" autocomplete="off"><em class="pwd-icon" id="pwd-icon4"><img src="../../images/pwd-icon.png"></em></span></li><li><span class="sp-l">确认密码</span><span class="sp-r"><label for="newpwd2">请再次输入新密码</label><input type="password" name="newpwd2" class="inpt" id="newpwd2" autocomplete="off"><input type="text" name="newpwd_2" id="newpwd_2" class="inpt" style="display: none;" autocomplete="off"><em class="pwd-icon" id="pwd-icon5"><img src="../../images/pwd-icon.png"></em></span></li></ul><div class="box"><a href="javascript:;" class="btn layer-btn cannel">取消</a>&nbsp;<a href="javascript:;" class="btn layer-btn" id="change_pwd_confirm">确认</a></div></div></div>',
            style: 'width:90%;background-color: transparent;  box-shadow: none;'
        });
    })

    $("#change_pwd_confirm").live("click", function() {
        if ($("#oldpwd").val() == '') {
            tips("请输入旧密码");
            return;
        }
        if ($("#newpwd").val() == '') {
            tips("请输入新密码");
            return;
        }
        if ($("#newpwd2").val() == '') {
            tips("请再次输入新密码");
            return;
        }
        if ($("#newpwd2").val() != $("#newpwd").val()) {
            tips("两次密码不一致");
            return;
        }
        layer.closeAll();
        tips("修改成功!");
    })

    $("#net_work").live('click', function() {
        var account = $("#account_get").text();
        var pwd = $("#pwd_get").text();
        var url = document.URL;
        if (url.indexOf('user') > 0) {
            location.href = 'setNet.html?acc=' + account + "&pwd=" + pwd;
        } else {
            location.href = 'index.html?acc=' + account + "&pwd=" + pwd;
        }
    });

    $('#PpoeHightSet').click(function() {
        $("#t_1_box ul").find('li').each(function(index) {
            if (index > 1) {
                if ($(this).is(':visible') == false) {
                    $(this).show();
                    $("#PpoeHightSet").text('简单配置');
                } else {
                    $(this).hide();
                    $(this).find('input').val('').siblings("label").show();
                    $("#PpoeHightSet").text('高级配置');
                }
            }
        });
    })

    $('#StaticHightSet').click(function() {
        $("#t_2_box ul").find('li').each(function(index) {
            if (index > 2) {
                if ($(this).is(':visible') == false) {
                    $(this).show();
                    $("#StaticHightSet").text('简单配置');
                } else {
                    $(this).hide();
                    $(this).find('input').val('').siblings("label").show();
                    $("#StaticHightSet").text('高级配置');
                }
            }
        });
    })

    $('#DhcpHightSet').click(function() {
        $("#t_3_box ul").find('li').each(function(index) {
            if ($(this).is(':visible') == false) {
                $(this).show();
                $("#DhcpHightSet").text('简单配置');
            } else {
                $(this).hide();
                $(this).find('input').val('').siblings("label").show();
                $("#DhcpHightSet").text('高级配置');
            }
        });
    })

})

function tips(content) {
    layer.open({
        title: false,
        shadeClose: false,
        content: content,
        style: 'background-color:#000;opacity:.75; box-shadow: none; text-align:center;color:#fff;',
        time: 2
    });
}

//function mobile_url() {
//    var account = $("#account_get").text();
//    var pwd = $("#pwd_get").text();
//    var url = document.URL;
//    if (url.indexOf('user') > 0) {
//        location.href = 'setNet.html?acc=' + account + "&pwd=" + pwd;
//    } else {
//        location.href = 'index.html?acc=' + account + "&pwd=" + pwd;
//    }
//}