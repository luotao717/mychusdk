/* user.js */
$(function () {
    /*************** user/index **********************/
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
    //设置用户名 密码
    $("#user").live("click", function () {
        layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            content: $("#setAccount"),
            skin: 'cy-class'
        });
    });

    $(".menu a").click(function () {
        var _index = $(this).index();
        if (_index == 0) {
            document.location = "index.html?tt=" + new Date().getTime();
        } else if (_index == 1) {
            document.location = "setting.html?tt=" + new Date().getTime();
        } else if (_index == 2) {
            document.location = "wireless_setting.html?tt=" + new Date().getTime();
        } else if (_index == 3) {
            document.location = "vpn.html?tt=" + new Date().getTime();
        } else if (_index == 4) {
            document.location = "more.html?tt=" + new Date().getTime();
        } else if (_index == 5) {
            document.location = "version.html?tt=" + new Date().getTime();
        }
    });

    //无线设置
    $("#wifi_set").live("click", function () {
        var poplayer = layer.open({
            type: 1,
            title: false,
            shadeClose: true,
            shade: [0.7, '#000'],
            closeBtn: false,
            content: $("#wifi_set_layer"),
            skin: 'cy-class'
        });
        getSetPrice('get');
    })

    $("#close_btn").live("click", function () {
        layer.closeAll();
    })

    //开启/关闭wifi
    $('input[name=onOff]').click(function () {
        var val = $('input[name=onOff]:checked').val();
        if (val == 1) {
            $("#wifiOnOff").show();
        } else {
            $("#wifiOnOff").hide();
        }
        wifiSet(val);
    });
    //确认 无线设置
    $("#wifi_set_confirm").click(function () {
        var ssid = $("#ssid").val();
        var pwd = $("#wifi_pwd").val();
        var channel = $("#wifiChannel").val();
        var wifiBandwidth = $("#wifiBandwidth").val();
        var hidden_ssid = 0;
        if ($("#hidden_ssid").attr('checked')) {
            hidden_ssid = 1;
        }
        if (ssid == '') {
            layer.tips('请输入无线名称', '#ssid', {
                tips: [1, '#000000']
            });
            return;
        }
        if (getStrLength(ssid) > 31 || /[\':;*?~`!@#$%^&+={}\[\]\<\\(\),\.\。\，]/.test(ssid)) {
            layer.tips('无线名称长度不得超过31位字符或者包含特殊字符', '#ssid', {
                tips: [1, '#000000']
            });
            return;
        }

        if (pwd.length > 0) {
            if (pwd.length > 31 || pwd.length < 8) {
                layer.tips('密码长度不能超过31位或不能少于8位', '#wifi_pwd', {
                    tips: [1, '#000000']
                });
                return;
            }
        }
        if (escape(pwd).indexOf("%u") != -1) {
            getMobileMsg('密码不能包含中文字符！', 1, "#wifi_pwd");
            return;
        }

        if (/[\'\"{}\[\]]/.test(pwd)) {
            getMobileMsg('密码不能包含特殊字符(&、#...)！', 1, "#wifi_pwd");
            return;
        }
        pwd = encodeURIComponent(pwd);
        setWifiAp(ssid, pwd, channel, hidden_ssid, wifiBandwidth);
    })

    //显隐密码
    var view_pwd = $(".pwd-icon");
    var flag = 1;
    view_pwd.click(function () {
        if (flag == 1) {
            $(this).siblings("label").attr("for", $(this).siblings("input[type=text]").attr("id"));
            $(this).siblings("input[type=password]").hide();
            $(this).siblings("input[type=text]").show();
            $(this).html("<img src = '../images/pwd-icon2.png'>");
            flag = 0;
        } else {
            $(this).siblings("label").attr("for", $(this).siblings("input[type=password]").attr("id"));
            $(this).siblings("input[type=password]").show();
            $(this).siblings("input[type=text]").hide();
            $(this).html("<img src = '../images/pwd-icon.png'>");
            flag = 1;
        }
    })
    $(".inpt").blur(function () {
        $(this).siblings(".inpt").val($(this).val());
    })

    $(".macChoose").change(function () {
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



    //确认 设置用户名 密码
    $("#user_confirm").click(function () {
        var pwd = $("#userpwd").val();
        if (pwd == '') {
            layer.tips('请输入密码', '#tips_userpwd', {
                tips: [1, '#000000']
            });
            return;
        }
        if ($("#userpwd2").val() != pwd) {
            layer.tips('两次密码不一致', '#tips_userpwd2', {
                tips: [1, '#000000']
            });
            return;
        }

        if (pwd.length > 15) {
            layer.msg('密码长度不能超过15位！');
            return;
        }

        if (escape(pwd).indexOf("%u") != -1) {
            getMobileMsg('密码不能包含中文字符！', 1, "#tips_userpwd");
            return;
        }
        var str_md5 = $.md5('admin' + pwd);
        var flag = false;
        $.ajax({
            type: "POST",
            url: actionUrl + "fname=system&opt=login&function=set&usrid=" + str_md5,
            dataType: "JSON",
            async: false,
            success: function (data) {
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
    /*************** user/index end**********************/

    $('input[name=wifi]').click(function () {
        var val = $('input[name=wifi]:checked').val();
        getSetPrice('set', val);
    });

    //获取验证码
    $("#getValicode").click(function () {
        var param = {};
        param.code = '101';
        param.product = '0001';
        param.phone = $("#phone_num").val();
        if (param.phone == '') {
            getMobileMsg('请输入手机号码！', 1, '#phone_num');
            return;
        }

        if (!/^(((13[0-9]{1})|(15[0-9]{1})|(18[0-9]{1}))+\d{8})$/.test(param.phone)) {
            getMobileMsg('手机号码不合法！', 1, '#phone_num');
            return;
        }
        getVpn(param, 1);
    });

    //提交vpn用户注册
    $("#vpn_confirm").click(function () {
        var param = {};
        param.name = $("#realname").val();
        param.identify = $("#idcard").val();
        param.phone = $("#phone_num").val();
        param.pass = $.md5($("#vpnpass").val());
        param.message = $("#vali_code").val();
        param.product = '0001';
        param.code = '102';
        param.person = '1';
        if (param.name == '') {
            getMobileMsg('请输入真实姓名！', 1, '#realname');
            return;
        }

//        if (/^([u4E00-u9FA5])*$/.test(param.realname)) {
//            getMobileMsg('姓名不合法！');
//            return;
//        }

        if (param.identify == '') {
            getMobileMsg('请输入身份证号！', 1, '#idcard');
            return;
        }

        if (!/^(\d{6})(\d{4})(\d{2})(\d{2})(\d{3})([0-9]|X)$/.test(param.identify)) {
            getMobileMsg('身份证号不合法！', 1, '#idcard');
            return;
        }

        if (param.phone == '') {
            getMobileMsg('请输入手机号码！', 1, '#phone_num');
            return;
        }

        if (!/^(((13[0-9]{1})|(15[0-9]{1})|(18[0-9]{1}))+\d{8})$/.test(param.phone)) {
            getMobileMsg('手机号码不合法！', 1, '#phone_num');
            return;
        }

        if (param.pass == '') {
            getMobileMsg('请输入vpn密码！', 1, '#vpnpass');
            return;
        }
        if ($("#vpnpass2").val() != $("#vpnpass").val()) {
            getMobileMsg('两次输入的vpn密码不一致！', 1, '#vpnpass');
            return;
        }
        if ($("#vali_code").val() == '') {
            getMobileMsg('请输入手机验证码', 1, '#vali_code');
            return;
        }
        getVpn(param, 2);

//        layer.closeAll();
//        layer.msg("注册成功!");
    });

    $("#vpn_login_confirm").click(function () {
        var param = {};
        param.code = '103';
        param.phone = $("#phone_login").val();
        param.pass = $.md5($("#pass_login").val());
        param.product = '0001';
        if (param.phone == '') {
            getMobileMsg('请输入手机号码！', 1, '#phone_login');
            return;
        }

        if (!/^(((13[0-9]{1})|(15[0-9]{1})|(18[0-9]{1}))+\d{8})$/.test(param.phone)) {
            getMobileMsg('手机号码不合法！', 1, '#phone_login');
            return;
        }

        if ($("#pass_login").val() == '') {
            getMobileMsg('请输入vpn密码！', 1, '#pass_login');
            return;
        }
        getVpn(param, 1);
    });

    $("#confirm_buy").click(function () {
        vpnInfo('get');
        var param = {};
        param.code = '107';
        param.product = '0001';
        param.phone = vpnJsonObject.user;
        param.pass = $.md5(vpnJsonObject.password);
        param.id = $('input[name="choicepackage"]:checked').val();
        param.method = $('input[name="paymethod"]:checked').val();
        param.appid = '3';
        param.appenv = '';
        if (param.id != 1 && param.id != 2 && param.id != 3) {
            getMobileMsg('请选择套餐！');
            return;
        }

        getVpn(param, 2);
    });

    $("#pay_status").click(function () {
        var param = {};
        param.code = '108';
        param.product = '0001';
        param.order = vpnJson.order;
        getVpn(param, 1);
    });

    $('.startvpn').live('click', function () {
        var param = {};
        param.code = '110';
        param.product = '0001';
        param.phone = vpnJsonObject.user;
        param.pass = $.md5(vpnJsonObject.password);
        param.cid = dhcpdmac.replace(/:/g, '');
        param.buyid = $(this).attr('id');
        param.open = '1';
        var timestamp = JSON.stringify(Date.parse(new Date()));
        param.timestamp = timestamp.substr(0,10);
        if (vpnOnlineStatus == '1') {
            param.open = '0';
        }
        getVpn(param, 1);
    });

    $('input[name="choicepackage"]').live('click', function () {
        var vpnprice = $(this).attr('data');
        $("#orderpice").text(vpnprice);
    });

    $("#vpn_register").click(function () {
        var mob = getMobile();
        if (mob == 1) {
            $("#vpn_login").hide();
            $("#vpn_reg").show();
        } else {
            loading(1, $("#vpn_reg"), 2);
        }
    });

    /*************** 更新固件上传**********************/
    //上传并更新
//    $("#update_1,#update_2").live("click", function() {
//        layer.open({
//            type: 1,
//            title: false,
//            shade: [0.7, '#000'],
//            closeBtn: false,
//            content: $("#uploadLay"),
//            skin: 'cy-class'
//        });
//        //设置最大值
//        ProgressBar.maxValue = 100;
//        //设置当前刻度
//        var index = 0;
//        var mProgressTimer = window.setInterval(function() {
//            index += 2;
//            ProgressBar.SetValue(index);
//            if (index == 100) {
//                window.clearInterval(mProgressTimer);
//                ProgressBar.SetValue(0);
//                layer.closeAll();
//                layer.msg("更新成功!");
//            }
//        }, 100);
//        //取消更新
//        $("#update_cannel").click(function() {
//            window.clearInterval(mProgressTimer);
//            layer.closeAll();
//        })
//    })




    /*************** 更新固件上传 end**********************/

})

//var ProgressBar = {
//    maxValue: 100,
//    value: 0,
//    SetValue: function(aValue) {
//        this.value = aValue;
//        if (this.value >= this.maxValue)
//            this.value = this.maxValue;
//        if (this.value <= 0)
//            this.value = 0;
//        var mWidth = this.value / this.maxValue * $("#progressBar").width() + "px";
//        $("#progressBar_Track").css("width", mWidth);
//        $("#progressBarTxt").html(this.value + "/" + this.maxValue);
//    }
//}