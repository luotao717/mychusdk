/* user.js */
$(function() {
    /*************** user/index **********************/
    //输入框焦点
    $(".inpt").focusin(function() {
        $(this).siblings("label").hide();
    }).focusout(function() {
        if ($(this).val() == '') {
            $(this).siblings("label").show();
        }
    })
    //取消按钮
    $(".cannel").live("click", function() {
        layer.closeAll();
    })
    //设置用户名 密码
    $("#user").live("click", function() {
        layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            content: $("#setAccount"),
            skin: 'cy-class'
        });
    });

    $(".menu a").click(function() {
        var _index = $(this).index();
        if (_index == 0) {
            document.location = "index.html?tt=" + new Date().getTime();
        } else if (_index == 1) {
            document.location = "setting.html?tt=" + new Date().getTime();
        } else if (_index == 2) {
            document.location = "wireless_setting.html?tt=" + new Date().getTime();
        } else if (_index == 3) {
            document.location = "more.html?tt=" + new Date().getTime();
        } else if (_index == 4) {
            document.location = "version.html?tt=" + new Date().getTime();
        }
    });

    //无线设置
    $("#wifi_set").live("click", function() {
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
        $("#close_btn").live("click", function() {
            layer.close(poplayer);
        })
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
    //确认 无线设置
    $("#wifi_set_confirm").click(function() {
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
    view_pwd.click(function() {
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
    $(".inpt").blur(function() {
        $(this).siblings(".inpt").val($(this).val());
    })

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



    //确认 设置用户名 密码
    $("#user_confirm").click(function() {
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
    /*************** user/index end**********************/

    $('input[name=wifi]').click(function() {
        var val = $('input[name=wifi]:checked').val();
        getSetPrice('set', val);
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