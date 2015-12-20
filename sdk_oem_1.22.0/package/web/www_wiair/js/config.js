var loadingLay = "";
var isCheck = getQueryString("c");
var autoAccount = getQueryString("acc");
var autoPasswd = getQueryString("pwd");
var myAccount = "";
var myPassword = "";
var isGet = false;
var interval = 0;
var checkWanDetectLinkInterval = 0;
var checkWanDetectModeInterval = 0;
var timeout = 0;
var timeoutCount = 15;
var timeoutCheck = 0;
var timeoutCheckCount = 10;
var isCheckModel = false;
var wanConfigJsonObject;
var wanInfoJsonObject;
var currentLanIP = "";
var currentMode = "";
var myMac = "";
var myStaticIp = "";
var myStaticMask = "";
var myStaticGw = "";
var myStaticDns = "";
var myStaticDns1 = "";
var isConfiged = false;
var isConnected = false;
var responseReason = 0;
var isAccountPwdReason = false;
var isNocheckMode = false;
var accountPwdReason = 19;
var userLoginStatus = 0;
var str_md5;
var mob = getMobile();

/*
 * 获取wanConfigJson
 */
function getWanConfigJsonObject() {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=net&opt=wan_conf&function=get",
        async: false,
        dataType: "json",
        success: function(data) {
            var jsonObject = data;
            if (jsonObject.error == 0) {
                wanConfigJsonObject = jsonObject;
            } else {
                locationUrl(data.error);
            }
        }
    });
}

/*
 * 设置PPPOE账号密码
 */
function setPPPOE(ppoe) {
    isGet = false;
    var mac = (wanConfigJsonObject.mac).toUpperCase();

    if (mac == null || mac == "") {
        getMobileMsg("获取MAC失败，请刷新本页重试！");
        return;
    }
    var account = $("#acc").val();
    var passwd = $("#pwd").val();
    var mtu = 1480;
    var dns = $("#pp_dns").val();
    var dns1 = $("#pp_dns1").val();
    var modeclone = $("#macChoose1").val();
    if (modeclone == 1) {
        mac = $("#macEnter1").val().toUpperCase();
    } else {
        mac = myMac;
    }
    if ($("#pp_mtu").val() > 0) {
        mtu = $("#pp_mtu").val();
    }

    if (dns != '') {
        if (!checkIP(dns)) {
            getMobileMsg("DNS不合法！", 1, '#pp_dns');
            return;
        }
    }

    if (dns1 != '') {
        if (!checkIP(dns1)) {
            getMobileMsg("备用DNS不合法！", 1, '#pp_dns1');
            return;
        }
    }

    if (trim(account) == "") {
        getMobileMsg("请输入宽带账号", 1, "#acc");
        return;
    }
    if (account.indexOf("\"") > -1 || account.indexOf("'") > -1 || account.indexOf("\\") > -1) {
        getMobileMsg("宽带账号不能包含 单双引号、反斜杠！", 1, "#acc");
        return;
    }
    if (trim(passwd) == "") {
        getMobileMsg("请输入宽带密码", 1, "#pwd");
        return;
    }
    if (passwd.indexOf("\"") > -1 || passwd.indexOf("'") > -1 || passwd.indexOf("\\") > -1) {
        getMobileMsg("宽带密码不能包含 单双引号、反斜杠！", 1, "#pwd");
        return;
    }

    if (modeclone > 1) {
        mac = getCloneMac(modeclone);
    }

    if (!checkMac(mac)) {
        getMobileMsg("MAC地址有误！");
        return;
    }

    $("#btn_1").attr("disabled", true);
    $("#btn_1").val("拨号中...");
    account = encodeURIComponent(account);
    passwd = encodeURIComponent(passwd);
    mtu = encodeURIComponent(mtu);

    if (currentMode != 2 || mac != myMac || account != myAccount || passwd != myPassword || dns != wanConfigJsonObject.dns || dns1 != wanConfigJsonObject.dns1 || mtu != wanConfigJsonObject.mtu || wanInfoJsonObject.connected == 0) {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: "fname=net&opt=wan_conf&function=set&user=" + account + "&passwd=" + passwd + "&mode=2&mtu=" + mtu + "&dns=" + dns + "&dns1=" + dns1 + "&mac=" + mac,
            dataType: "json",
            success: function(data) {
                currentMode = 2;
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    intervalTime(1, '拨号', '宽带拨号', 2);
                } else {
                    locationUrl(data.error);
                    $("#btn_1").attr("disabled", false);
                    $("#btn_1").val("开始拨号");
                    return;
                }
            }, error: function() {
                getMobileMsg('拨号超时,请重新连接路由器！');
                return;
            }
        });
    } else {
        if (wanInfoJsonObject.connected == 0) {
            intervalTime(1, '拨号', '宽带拨号', 2);
        } else {
            toUrl();
        }
    }
}

/*
 * 设置静态IP
 */
function setStatic(stat) {
    isGet = false;
    var mac = '';
    var static_ip = $("#static_ip").val();
    var static_mask = $("#static_mask").val();
    var static_gw = $("#static_gw").val();
    var static_dns = $("#static_dns").val();
    var static_dns1 = $("#static_dns1").val();
    var mtu = 1500;
    var modeclone = $("#macChoose2").val();
    if (modeclone == 1) {
        mac = $("#macEnter2").val().toUpperCase();
    } else {
        mac = myMac;
    }

    if ($("#static_mtu").val() > 0) {
        mtu = $("#static_mtu").val();
    }

    if (!checkIP(static_ip)) {
        getMobileMsg("IP地址不合法！", 1, '#static_ip');
        return;
    }

    if (!checkMask(static_mask)) {
        getMobileMsg("子网掩码不合法！", 1, '#static_mask');
        return;
    }

    if (!checkIP(static_gw)) {
        getMobileMsg("默认网关不合法！", 1, '#static_gw');
        return;
    }

    if (static_dns != "") {
        if (!checkIP(static_dns)) {
            getMobileMsg("DNS不合法！", 1, '#static_dns');
            return;
        }
    }

    if (static_dns1 != '') {
        if (!checkIP(static_dns1)) {
            getMobileMsg("备用DNS不合法！", 1, '#static_dns1');
            return;
        }
    }

    if (static_ip == static_gw) {
        getMobileMsg("IP地址或默认网关设置有误！");
        return;
    }

    if (!validateNetwork(static_ip, static_mask, static_gw)) {
        getMobileMsg("IP地址+子网掩码+默认网关设置错误！");
        return;
    }

    if (validateNetwork(static_ip, static_mask, currentLanIP)) {
        getMobileMsg("IP地址设置错误！");
        return;
    }

    if (modeclone > 1) {
        mac = getCloneMac(modeclone);
    }

    if (!checkMac(mac)) {
        getMobileMsg("MAC地址有误！");
        return;
    }

    mtu = encodeURIComponent(mtu);
    $("#btn_2").attr("disabled", true);
    $("#btn_2").val("连接中...");
    if (currentMode != 3 || mac != myMac || static_ip != myStaticIp || static_mask != myStaticMask || static_gw != myStaticGw || static_dns != wanConfigJsonObject.dns || static_dns1 != wanConfigJsonObject.dns1 || mtu != wanConfigJsonObject.mtu || wanInfoJsonObject.connected == 0) {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: "fname=net&opt=wan_conf&function=set&ip=" + static_ip + "&mask=" + static_mask + "&gw=" + static_gw + "&dns=" + static_dns + "&mode=3&dns1=" + static_dns1 + "&mtu=" + mtu + "&mac=" + mac,
            dataType: "json",
            success: function(data) {
                currentMode = 3;
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    intervalTime(2, '确认', '静态IP设置', 3);
                } else {
                    locationUrl(data.error);
                    $("#btn_2").attr("disabled", false);
                    $("#btn_2").val("确 认");
                    return;
                }
            }, error: function() {
                getMobileMsg('静态IP配置超时,请重新连接路由器！');
                return;
            }
        });
    } else {
        if (wanInfoJsonObject.connected == 0) {
            intervalTime(2, '确认', '静态IP设置', 3);
        } else {
            toUrl();
        }
    }
}

/*
 * 设置DHCP
 */
function setDHCP(dhcp) {
    isGet = false;
    var mac = '';
    var mtu = 1500;
    var dns = $("#dhcp_dns").val();
    var dns1 = $("#dhcp_dns1").val();
    var modeclone = $("#macChoose3").val();
    if (modeclone == 1) {
        mac = $("#macEnter3").val().toUpperCase();
    } else {
        mac = myMac;
    }

    if ($("#dhcp_mtu").val() > 0) {
        mtu = $("#dhcp_mtu").val();
    }

    if (dns != "") {
        if (!checkIP(dns)) {
            getMobileMsg("DNS不合法！", 1, '#dhcp_dns');
            return;
        }
    }

    if (dns1 != "") {
        if (!checkIP(dns1)) {
            getMobileMsg("备用DNS不合法！", 1, '#dhcp_dns1');
            return;
        }
    }

    if (modeclone > 1) {
        mac = getCloneMac(modeclone);
    }

    if (!checkMac(mac)) {
        getMobileMsg("MAC地址有误！");
        return;
    }

    mtu = encodeURIComponent(mtu);
    $("#btn_3").attr("disabled", true);
    $("#btn_3").val("连接中...");
    if (currentMode != 1 || mac != myMac || dns != wanConfigJsonObject.dns || dns1 != wanConfigJsonObject.dns1 || mtu != wanConfigJsonObject.mtu || wanInfoJsonObject.connected == 0) {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: "fname=net&opt=wan_conf&function=set&mode=1&mtu=" + mtu + "&dns=" + dns + "&dns1=" + dns1 + "&mac=" + mac,
            dataType: "json",
            success: function(data) {
                currentMode = 1;
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    intervalTime(3, '启用', 'DHCP配置', 1);
                } else {
                    locationUrl(data.error);
                    $("#btn_3").attr("disabled", false);
                    $("#btn_3").val("确 认");
                    return;
                }
            }, error: function() {
                getMobileMsg('动态IP配置超时,请重新连接路由器！');
                return;
            }
        });
    } else {
        if (wanInfoJsonObject.connected == 0) {
            intervalTime(3, '启用', 'DHCP配置', 1);
        } else {
            toUrl();
        }
    }
}

/*
 * 获取wanInfoJson
 */
function getWanInfoJsonObject() {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=net&opt=wan_info&function=get",
        async: false,
        dataType: "json",
        success: function(data) {
            var jsonObject = data;
            if (jsonObject.error == 0) {
                wanInfoJsonObject = jsonObject;
                currentMode = wanInfoJsonObject.mode;
                if (jsonObject.reason > 0) {
                    responseReason = jsonObject.reason;
                }
            } else {
                locationUrl(data.error);
            }
        }
    });
}


/*
 * 获取WanInfo
 */
function getWanInfo(choice) {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=net&opt=wan_info&function=get",
        async: false,
        dataType: "json",
        success: function(data) {
            var jsonObject = data;
            if (jsonObject.error == 0) {
                if (jsonObject.connected == 1) {
                    isGet = true;
                    clearInterval(interval);
                    timeout = 0;
                    if (choice > 0) {
                        document.location = 'index.html?tt=' + new Date().getTime();
                    } else if (choice == 0) {
                        toUrl();
                    } else if (choice < 0) {
                        wanInfoJsonObject = jsonObject;
                    }
                } else {
                    if (jsonObject.reason > 0) {
                        if (jsonObject.reason == accountPwdReason) {
                            isAccountPwdReason = true;
                        }
                        responseReason = jsonObject.reason;
                    }
                    wanInfoJsonObject = jsonObject;
                }
            } else {
                locationUrl(data.error);
            }
        }, error: function() {
            getMobileMsg('请重启路由器！');
        }
    });
}

/*
 * 检查网络类型
 */
function checkWanDetect() {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=net&opt=wan_detect&function=get",
        dataType: "json",
        success: function(data) {
            var jsonObject = data;
            if (jsonObject.error == 0) {
                if (jsonObject.wan_link == 0) {
                    checkWanDetectLinkInterval = setInterval(function() {
                        checkWanDetectLink();
                    }, 2000);
                } else {
                    clearInterval(checkWanDetectLinkInterval);
                    if (jsonObject.connected == 0) {
                        isConnected = false;
                        $("#account_login").hide();
                        layer.closeAll();
                        if (jsonObject.mode == 1) {	//DHCP
                            $("#t_3_box").show();
                            $("#t_3").addClass('selected');
                            showDHCP();
                        } else if (jsonObject.mode == 2) {	//PPPOE
                            $("#t_1_box").show();
                            $("#t_1").addClass('selected');
                            showPPOE();
                        } else if (jsonObject.mode == 3) {	//STATIC
                            $("#t_2_box").show();
                            $("#t_2").addClass('selected');
                            showStatic();
                        } else if (jsonObject.mode == 4) {
                            $("#t_4_box").show();
                            $("#t_4").addClass('selected');
                            getWifiList();
                            if (wanConfigJsonObject.oldmode == 1) {
                                showDHCP();
                            } else if (wanConfigJsonObject.oldmode == 2) {
                                showPPOE();
                            } else if (wanConfigJsonObject.oldmode == 3) {
                                showStatic();
                            }
                        } else if (jsonObject.mode == 0) {//checking
                            checkWanDetectModeInterval = setInterval(function() {
                                if (timeoutCheck > timeoutCheckCount - 1) {
                                    timeoutCheck = 0;
                                    clearInterval(checkWanDetectModeInterval);
                                    isNocheckMode = true;
                                    return;
                                }
                                if (!isCheckModel) {
                                    checkWanDetectModel();
                                    timeoutCheck++;
                                }
                            }, 1000);
                        }
                    } else {	//connected
                        isConnected = true;
                        clearInterval(checkWanDetectLinkInterval);
                        clearInterval(checkWanDetectModeInterval);
                        toUrl();
                    }
                }
            } else {
                locationUrl(data.error);
            }
        }
    });
}
/*
 * 检查网线
 */
function checkWanDetectLink() {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=net&opt=wan_detect&function=get",
        dataType: "json",
        success: function(data) {
            var jsonObject = data;
            if (jsonObject.wan_link == 1) {
                clearInterval(checkWanDetectLinkInterval);
                checkWanDetect();
            } else if (jsonObject.wan_link == 0) {
                getWanInfo(-1);
                if (wanInfoJsonObject.connected == 1) {
                    document.location = 'user/index.html?tt=' + new Date().getTime();
                } else {
                    layer.closeAll();
                    $("#account_login").hide();
                    clearInterval(checkWanDetectLinkInterval);
                    if (wanInfoJsonObject.mode == 2) {
                        $("#wrap_2").show().siblings(".wrap").hide();
                        showPPOE();
                        $("#t_1_box").show();
                        $("#t_1").addClass('selected');
                    } else {
                        $("#wrap_2").show().siblings(".wrap").hide();
                        $("#t_4").addClass('selected');
                        $("#t_4_box").show();
                        getWifiList();
                        if (wanConfigJsonObject.oldmode == 1) {
                            showDHCP();
                        } else if (wanConfigJsonObject.oldmode == 2) {
                            showPPOE();
                        } else if (wanConfigJsonObject.oldmode == 3) {
                            showStatic();
                        }
                    }
                }
            } else {
                locationUrl(data.error);
            }
        }
    });
}


/*
 * 检查网络类型
 */
function checkWanDetectModel() {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=net&opt=wan_detect&function=get",
        dataType: "json",
        success: function(data) {
            var jsonObject = data;
            if (jsonObject.mode > 0) {
                clearInterval(checkWanDetectModeInterval);
                timeout = 0;
                isCheckModel = true;
                layer.closeAll();
                $("#account_login").hide();
                $("#wrap_2").show().siblings(".wrap").hide();
                if (jsonObject.mode == 1) {	//DHCP
                    $("#t_3").addClass('selected');
                    $("#t_3_box").show();
                    showDHCP();
                } else if (jsonObject.mode == 2) {	//PPPOE
                    $("#t_1").addClass('selected');
                    $("#t_1_box").show();
                    showPPOE();
                } else if (jsonObject.mode == 3) {
                    $("#t_2").addClass('selected');
                    $("#t_2_box").show();
                    showStatic();
                } else if (jsonObject.mode == 4) {
                    $("#t_4").addClass('selected');
                    $("#t_4_box").show();
                    getWifiList();
                }
            } else if (jsonObject.mode == 0) {
                $("#t_1").addClass('selected');
                $("#t_1_box").show();
                $("#wrap_2").show().siblings(".wrap").hide();
                layer.closeAll();
            }
        }
    });
}

function toUrl() {
    if (browser.versions.android || browser.versions.iPhone || browser.versions.iPad) {
        document.location = 'http://' + document.domain + "/m/user/index.html?tt=" + new Date().getTime();
    } else {
        document.location = 'http://' + document.domain + "/user/index.html?tt=" + new Date().getTime();
    }
}

/*
 * STATIC标签
 */
function tab(type) {
    for (var i = 1; i <= 3; i++) {
        $("#set_" + i).attr("class", "sub_dt sbtn");
        $("#sub_set_" + i).attr("class", "mac_copy dl");
    }
    responseReason = 0;
    if (type == 3) {
        if (!isConfiged && !isNocheckMode) {
            $("#static_msg").html("");
            $.ajax({
                type: "POST",
                url: actionUrl,
                data: "fname=net&opt=dhcpd&function=get",
                dataType: "json",
                success: function(data) {
                    var jsonObject = data;
                    if (jsonObject.error == 0) {
                        var static_ip = jsonObject.ip == null ? "" : jsonObject.ip;
                        var static_mask = jsonObject.mask == null ? "" : jsonObject.mask;
                        $("#static_ip").val("");
                        $("#static_mask").val("");
                        $("#static_gw").val("");
                        $("#static_dns").val("");
                        currentLanIP = static_ip;
                    }
                }
            });
        }
    }
}
/*
 * 检查是否配置过
 */
function checkConfig() {
    if ($.cookie('lstatus') == 'true') {
        var loadingLay;
        if (mob == 1) {
            loadingLay = layer.open({
                type: 1,
                shadeClose: false,
                content: '<div class="loading"><p><img src="../../images/loading-1.gif"></p><p>正在检测上网方式...</p></div>',
                style: 'background-color: transparent;  box-shadow: none;'
            });
        } else {
            loadingLay = layer.open({
                type: 1,
                title: false,
                shade: [0.7, '#000'],
                closeBtn: false,
                shadeClose: false,
                content: '<div class="loading"><p><img src="../../images/loading-1.gif" width="120"></p><p>正在检测上网方式...</p></div>',
                skin: 'cy-class'
            });
        }
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: "fname=net&opt=wan_conf&function=get",
            async: false,
            dataType: "json",
            success: function(data) {
                var jsonWanConfigObject = data;
                if (jsonWanConfigObject.error == 0) {
                    wanConfigJsonObject = jsonWanConfigObject;
                    myMac = (wanConfigJsonObject.mac).toUpperCase();
                    $('#ppoe_mac_inpt').val(myMac).siblings("label").hide();
                    $('#dhcp_mac_inpt').val(myMac).siblings("label").hide();
                    $('#static_mac_inpt').val(myMac).siblings("label").hide();
                    $("#wisp_mac_inpt").val(myMac);
                    if (autoAccount != '' && autoAccount != null) {
                        $("#acc").val(autoAccount).siblings("label").hide();
                        $("#pwd").val(autoPasswd).siblings("label").hide();
                        $("#wrap_2").show().siblings(".wrap").hide();
                        $("#t_1").addClass('selected');
                        $("#t_1_box").show();
                        layer.closeAll();
                        return;
                    } else {
                        $.ajax({
                            type: "POST",
                            url: actionUrl,
                            data: "fname=system&opt=device_check&function=get",
                            dataType: "json",
                            success: function(data) {
                                var jsonObject = data;
                                if (jsonObject.config_status == 0) {	//no config
                                    isConfiged = false;
                                    //getWanConfigJsonObject();
                                    checkWanDetect();
                                    currentMode = wanConfigJsonObject.mode;
                                    myMac = (wanConfigJsonObject.mac).toUpperCase();
                                } else {								//configed
                                    isConfiged = true;
                                    getWanInfoJsonObject();
                                    if (wanInfoJsonObject.error == 0) {
                                        if (wanInfoJsonObject.link == 0) {
                                            checkWanDetectLinkInterval = setInterval(function() {
                                                checkWanDetectLink();
                                            }, 2000);
                                        } else if (wanInfoJsonObject.connected == 0) {
                                            isConnected = false;
                                            if (wanConfigJsonObject.error == 0) {
                                                myMac = (wanConfigJsonObject.mac).toUpperCase();
                                                currentMode = wanConfigJsonObject.mode;
                                                layer.closeAll();
                                                $("#wrap_2").show().siblings(".wrap").hide();
                                                $("#account_login").hide();
                                                showNetDiv();
                                            }
                                        } else {
                                            isConnected = true;
                                            toUrl();
                                        }
                                    }
                                }
                            }
                        });
                    }
                }
            }
        });
    } else {
        $("#account_login").show();
    }
}

function showPPOE() {
    if (wanConfigJsonObject.user != '' && typeof (wanConfigJsonObject.user) != 'undefined') {
        $("#acc").val(wanConfigJsonObject.user).siblings("label").hide();
        $("#pwd").val(wanConfigJsonObject.passwd).siblings("label").hide();
        $("#pp_mtu").val(wanConfigJsonObject.mtu).siblings("label").hide();
        if (trim(wanConfigJsonObject.dns) != '') {
            $("#pp_dns").val(wanConfigJsonObject.dns).siblings("label").hide();
        }
        if (trim(wanConfigJsonObject.dns1) != '') {
            $("#pp_dns1").val(wanConfigJsonObject.dns1).siblings("label").hide();
        }
        $("#mac_box1").text(wanConfigJsonObject.mac);
        if (wanConfigJsonObject.mac == wanConfigJsonObject.hostmac) {
            $("#macChoose1 option[value=2]").attr('selected', 'selected');
        } else if (wanConfigJsonObject.mac == wanConfigJsonObject.rawmac) {
            $("#macChoose1 option[value=3]").attr('selected', 'selected');
        } else {
            $("#mac_box1").hide();
            $("#macChoose1 option[value=1]").attr('selected', 'selected');
            $("#macChoose1 option[value=1]").attr('selected', 'selected');
        }
        if (wanConfigJsonObject.oldmode == 2) {
            if (trim(wanConfigJsonObject.olddns) != '') {
                $("#pp_dns").val(wanConfigJsonObject.olddns).siblings("label").hide();
            }
            if (trim(wanConfigJsonObject.olddns1) != '') {
                $("#pp_dns1").val(wanConfigJsonObject.olddns1).siblings("label").hide();
            }
            $("#pp_mtu").val(wanConfigJsonObject.oldmtu).siblings("label").hide();
        }
    }
    var li = $("#t_1_box ul").find('li');
    if (wanConfigJsonObject.dns == '' && wanConfigJsonObject.dns1 == '') {
        if (wanConfigJsonObject.oldmode == undefined) {
            li.each(function(index) {
                if (index > 1) {
                    $(this).hide();
                    $("#PpoeHightSet").text('高级配置');
                }
            });
        } else if ((wanConfigJsonObject.olddns !== undefined && wanConfigJsonObject.olddns != '') || (wanConfigJsonObject.olddns1 !== undefined && wanConfigJsonObject.olddns1 != '')) {
            li.each(function(index) {
                if (index > 1) {
                    $(this).show();
                    $("#PpoeHightSet").text('简单配置');
                }
            });
        }
    } else {
        li.each(function(index) {
            if (index > 1) {
                $(this).show();
                $("#PpoeHightSet").text('简单配置');
            }
        });
    }
}

function showDHCP() {
    if (trim(wanConfigJsonObject.dns) != '') {
        $("#dhcp_dns").val(wanConfigJsonObject.dns).siblings("label").hide();
    }
    if (trim(wanConfigJsonObject.dns1) != '') {
        $("#dhcp_dns1").val(wanConfigJsonObject.dns1).siblings("label").hide();
    }
    $("#dhcp_mtu").val(wanConfigJsonObject.mtu).siblings("label").hide();
    $("#mac_box3").text(wanConfigJsonObject.mac);
    if (wanConfigJsonObject.mac == wanConfigJsonObject.hostmac) {
        $("#macChoose3 option[value=2]").attr('selected', 'selected');
    } else if (wanConfigJsonObject.mac == wanConfigJsonObject.rawmac) {
        $("#macChoose3 option[value=3]").attr('selected', 'selected');
    } else {
        $("#mac_box3").hide();
        $("#macChoose3 option[value=1]").attr('selected', 'selected');
        $("#macChoose3 option[value=1]").attr('selected', 'selected');
    }

    if (wanConfigJsonObject.oldmode == 1) {
        if (trim(wanConfigJsonObject.olddns) != '') {
            $("#dhcp_dns").val(wanConfigJsonObject.olddns).siblings("label").hide();
        }
        if (trim(wanConfigJsonObject.olddns1) != '') {
            $("#dhcp_dns1").val(wanConfigJsonObject.olddns1).siblings("label").hide();
        }
        $("#dhcp_mtu").val(wanConfigJsonObject.oldmtu).siblings("label").hide();
    }

    var li = $("#t_3_box ul").find('li');
    if (wanConfigJsonObject.dns == '' && wanConfigJsonObject.dns1 == '') {
        if (wanConfigJsonObject.oldmode == undefined) {
            li.each(function() {
                $(this).hide();
                $("#DhcpHightSet").text('高级配置');
            });
        } else if ((wanConfigJsonObject.olddns !== undefined && wanConfigJsonObject.olddns != '') || (wanConfigJsonObject.olddns1 !== undefined && wanConfigJsonObject.olddns1 != '')) {
            li.each(function() {
                $(this).show();
                $("#DhcpHightSet").text('简单配置');
            });
        }
    } else {
        li.each(function() {
            $(this).show();
            $("#DhcpHightSet").text('简单配置');
        });
    }
}

function showStatic() {
    if (wanConfigJsonObject.ip != '' && typeof (wanConfigJsonObject.ip) != 'undefined') {
        $("#static_ip").val(wanConfigJsonObject.ip).siblings("label").hide();
        $("#static_mask").val(wanConfigJsonObject.mask).siblings("label").hide();
        $("#static_gw").val(wanConfigJsonObject.gw).siblings("label").hide();
        if (trim(wanConfigJsonObject.dns) != '') {
            $("#static_dns").val(wanConfigJsonObject.dns).siblings("label").hide();
        }
        if (trim(wanConfigJsonObject.dns1) != '') {
            $("#static_dns1").val(wanConfigJsonObject.dns1).siblings("label").hide();
        }
        $("#static_mtu").val(wanConfigJsonObject.mtu).siblings("label").hide();
        $("#mac_box2").text(wanConfigJsonObject.mac);
        if (wanConfigJsonObject.mac == wanConfigJsonObject.hostmac) {
            $("#macChoose2 option[value=2]").attr('selected', 'selected');
        } else if (wanConfigJsonObject.mac == wanConfigJsonObject.rawmac) {
            $("#macChoose2 option[value=3]").attr('selected', 'selected');
        } else {
            $("#mac_box2").hide();
            $("#macChoose2 option[value=1]").attr('selected', 'selected');
            $("#macChoose2 option[value=1]").attr('selected', 'selected');
        }

        if (wanConfigJsonObject.oldmode == 3) {
            if (trim(wanConfigJsonObject.olddns) != '') {
                $("#static_dns").val(wanConfigJsonObject.olddns).siblings("label").hide();
            }
            if (trim(wanConfigJsonObject.olddns1) != '') {
                $("#static_dns1").val(wanConfigJsonObject.olddns1).siblings("label").hide();
            }
            $("#static_mtu").val(wanConfigJsonObject.oldmtu).siblings("label").hide();
        }


        myStaticIp = wanConfigJsonObject.ip;
        myStaticMask = wanConfigJsonObject.mask;
        myStaticGw = wanConfigJsonObject.gw;
        myStaticDns = wanConfigJsonObject.dns;
    }
    var li = $("#t_2_box ul").find('li');
    if (wanConfigJsonObject.dns == '' && wanConfigJsonObject.dns1 == '') {
        if (wanConfigJsonObject.oldmode == undefined) {
            li.each(function(index) {
                if (index > 2) {
                    $(this).hide();
                    $("#StaticHightSet").text('高级配置');
                }
            });
        } else if ((wanConfigJsonObject.olddns !== undefined && wanConfigJsonObject.olddns != '') || (wanConfigJsonObject.olddns1 !== undefined && wanConfigJsonObject.olddns1 != '')) {
            li.each(function(index) {
                if (index > 2) {
                    $(this).show();
                    $("#StaticHightSet").text('简单配置');
                }
            });
        }
    } else {
        li.each(function(index) {
            if (index > 2) {
                $(this).show();
                $("#StaticHightSet").text('简单配置');
            }
        });
    }
}

function userLogin() {
    var name = 'admin';
    var pwd = $("#login_pwd").val();
    var mob = getMobile();

    if (pwd == '') {
        getMobileMsg("请输入密码", 1, "#login_pwd");
        return;
    }

    $("#user_login").val('正在登录...');
    $("#user_login").attr("disabled", true);
    str_md5 = $.md5(name + pwd);
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=system&opt=login&function=set&usrid=" + str_md5,
        dataType: "JSON",
        success: function(data) {
            if (data.error == 0) {
                userLoginStatus = 1;
                $.cookie('lstatus', true);
                if ($.cookie('lstatus') == null) {
                    getMobileMsg('登录失败,错误码21000！');
                    $("#user_login").val('登 录');
                    $("#user_login").attr("disabled", false);
                    return;
                }
                checkConfig();
            } else if (data.error == 10001) {
                getMobileMsg('密码错误！');
                $("#user_login").val('登 录');
                $("#user_login").attr("disabled", false);
            } else {
                getMobileMsg('登录失败！');
                $("#user_login").val('登 录');
                $("#user_login").attr("disabled", false);
            }
        }
    });
}

function getConfig() {
    if (typeof (wanInfoJsonObject) == 'undefined') {
        getWanInfoJsonObject();
    }

    if (typeof (wanConfigJsonObject) == 'undefined') {
        getWanConfigJsonObject();
    }
    myMac = (wanConfigJsonObject.mac).toUpperCase();

    $('#ppoe_mac_inpt').val(myMac).siblings("label").hide();
    $('#dhcp_mac_inpt').val(myMac).siblings("label").hide();
    $('#static_mac_inpt').val(myMac).siblings("label").hide();
    $("#wisp_mac_inpt").val(myMac);
    if (autoAccount != '' && autoAccount != null) {
        $("#acc").val(autoAccount).siblings("label").hide();
        $("#pwd").val(autoPasswd).siblings("label").hide();
        $("#t_1").addClass('selected');
        $("#t_1_box").show();
        layer.closeAll();
        return;
    }
    showNetDiv();
}

function showNetDiv() {
    if (wanConfigJsonObject.mode == 1) {
        $("#t_3").addClass('selected');
        $("#t_3_box").show();
        showDHCP();
    } else if (wanConfigJsonObject.mode == 2) {
        $("#t_1").addClass('selected');
        $("#t_1_box").show();
        showPPOE();
    } else if (wanConfigJsonObject.mode == 3) {
        $("#t_2").addClass('selected');
        $("#t_2_box").show();
        $("#wrap_2").show().siblings(".wrap").hide();
        showStatic();
    } else if (wanConfigJsonObject.mode == 4) {
        $("#t_4").addClass('selected');
        $("#t_4_box").show();
        $("#wrap_2").show().siblings(".wrap").hide();
        getWifiList();
        if (wanConfigJsonObject.oldmode == 1) {
            showDHCP();
        }
        else if (wanConfigJsonObject.oldmode == 2) {
            showPPOE();
        } else if (wanConfigJsonObject.oldmode == 3) {
            showStatic();
        }
    } else if (wanConfigJsonObject.mode == 0) {
        layer.closeAll();
        $("#t_1").addClass('selected');
        $("#t_1_box").show();
        $("#wrap_2").show().siblings(".wrap").hide();
    }
}

function getWifiList() {
    var loadingLay;
    var mob = getMobile();
    var conn_ssid = wanConfigJsonObject.ssid;
    if (mob == 1) {
        loadingLay = layer.open({
            type: 1,
            shadeClose: false,
            content: '<div class="loading"><p><img src="../../images/loading-1.gif"></p><p>正在加载...</p></div>',
            style: 'background-color: transparent;  box-shadow: none;'
        });
    } else {
        loadingLay = layer.open({
            type: 1,
            title: false,
            shade: [0.7, '#000'],
            closeBtn: false,
            shadeClose: false,
            content: '<div class="loading"><p><img src="../../images/loading-1.gif" width="120"></p><p>正在加载...</p></div>',
            skin: 'cy-class'
        });
    }
    $.ajax({
        type: 'POST',
        url: actionUrl,
        data: "fname=net&opt=ap_list&function=get",
        dataType: "JSON",
        success: function(data) {
            var jsonWifiListObject = data;
            if (jsonWifiListObject.error == 0) {
                layer.close(loadingLay);
                var aplist = jsonWifiListObject.aplist;
                var wifi = "";
                var list = "";
                var mac;
                if (mob == 1) {
                    mac = $("#mac_wisp").val();
                } else {
                    mac = $("#mac_wisp").val();
                }
                var mac_equal;
                var data_ssid = new Array();
                if (aplist.length > 0) {
                    for (var i = 0; i < aplist.length; i++) {
                        if (aplist[i].dbm >= 0 && aplist[i].dbm <= 33) {
                            aplist[i].dbm = '"lvl lvl-4"';
                        } else if (aplist[i].dbm >= 34 && aplist[i].dbm <= 66) {
                            aplist[i].dbm = '"lvl lvl-2"';
                        } else if (aplist[i].dbm >= 67 && aplist[i].dbm <= 100) {
                            aplist[i].dbm = '"lvl lvl-1"';
                        }
                        if (aplist[i].security == 'NONE') {
                            aplist[i].security_lock = "<em class=" + aplist[i].dbm + "></em>";
                        } else {
                            aplist[i].security_lock = "<em class=" + aplist[i].dbm + "><i class='lock'></i></em>";
                        }
                        mac_equal = aplist[i].bssid.toUpperCase();
                        if (aplist[i].ssid.length > 15) {
                            data_ssid[i] = aplist[i].ssid.substr(0, 15) + '...';
                        } else {
                            if (aplist[i].ssid.length < 2) {
                                data_ssid[i] = "SSID被隐藏 当前信道:" + aplist[i].channel;
                            } else {
                                data_ssid[i] = aplist[i].ssid;
                            }
                        }

                        if (mob == 1) {
                            if (aplist[i].security != 'NONE')
                            {
                                list = "<li><a href='javascript:;' sec=" + aplist[i].security + " data=" + aplist[i].channel + " mac=" + aplist[i].bssid + " ssid=" + aplist[i].ssid + "><span class='fl name'>" + data_ssid[i] + "</span><span class='fr'>" + aplist[i].security_lock + "</span></a></li>";
                            } else {
                                list = "<li><a href='javascript:;' data=" + aplist[i].channel + " mac=" + aplist[i].bssid + " ssid=" + aplist[i].ssid + "><span class='fl name'>" + data_ssid[i] + "</span><span class='fr'>" + aplist[i].security_lock + "</span></a></li>";
                            }
                        } else {
                            if (aplist[i].security != 'NONE')
                            {
                                list = "<li sec=" + aplist[i].security + " data=" + aplist[i].channel + " mac=" + aplist[i].bssid + " ssid=" + aplist[i].ssid + "><span class='fl name'>" + data_ssid[i] + "</span><span class='fr'>" + aplist[i].security_lock + "</span></li>";
                            } else {
                                list = "<li data=" + aplist[i].channel + " mac=" + aplist[i].bssid + " ssid=" + aplist[i].ssid + "><span class='fl name'>" + data_ssid[i] + "</span><span class='fr'>" + aplist[i].security_lock + "</span></li>";
                            }
                        }
                        wifi += list;
                    }
                }
                getWanInfo(-1);
                if (wanInfoJsonObject.connected == 1 && wanInfoJsonObject.mode == 4) {
                    wifi = '<li style="text-align:left;font-size:12px;">' + conn_ssid + ' 已连接</li>' + wifi;
                }
                $("#mCSB_1_container").empty().html(wifi);
            } else {
                locationUrl(data.error);
                $("#mCSB_1_container").html('');
            }
        }
    });
}
/*
 * 无线中继连接
 * @param {type} ssid
 * @param {type} pwd
 * @param {type} mac
 * @param {type} channel
 */
function connectWisp(ssid, pwd, mac, channel, check_wpa, sec) {
    var dat, dat2;
    if (mac == '' || mac == 'undefined' || typeof (mac) == 'undefined') {
        mac = myMac;
    }
    mac = mac.toUpperCase();

    if (pwd == null || pwd == '') {
        dat = "fname=net&opt=wan_conf&function=set" + "&mac=" + mac + "&mode=4&ssid=" + ssid + "&security=NONE&key=NONE&channel=" + channel;
        dat2 = "fname=net&opt=wan_conf&function=get" + "&mac=" + mac + "&mode=4&ssid=" + ssid + "&security=NONE&key=NONE&channel=" + channel;
    } else {
        if (typeof (check_wpa) != 'undefined' && check_wpa != '') {
            check_wpa = check_wpa.replace(' ', '') + '/TKIPAES';
            if (check_wpa == 'WPAWPA2PSK/TKIPAES') {
                check_wpa = 'WPAPSKWPA2PSK/TKIPAES';
            }
            dat = "fname=net&opt=wan_conf&function=set" + "&mac=" + mac + "&mode=4&ssid=" + ssid + "&security=" + check_wpa + "&key=" + pwd + "&channel=" + channel;
            dat2 = "fname=net&opt=wan_conf&function=get" + "&mac=" + mac + "&mode=4&ssid=" + ssid + "&security=" + check_wpa + "&key=" + pwd + "&channel=" + channel;
        } else {
            dat = "fname=net&opt=wan_conf&function=set" + "&mac=" + mac + "&mode=4&ssid=" + ssid + "&security=" + sec + "&key=" + pwd + "&channel=" + channel;
            dat2 = "fname=net&opt=wan_conf&function=get" + "&mac=" + mac + "&mode=4&ssid=" + ssid + "&security=" + sec + "&key=" + pwd + "&channel=" + channel;
        }
    }
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: dat,
        dataType: "JSON",
        success: function(data) {
            if (data.error == 0) {
                $.ajax({
                    type: "POST",
                    url: actionUrl,
                    data: dat2,
                    dataType: "JSON",
                    success: function(jsonWisp) {
                        if (jsonWisp.error == 0 && jsonWisp.mode == 4) {
                            var sec = 60;
                            var interval = setInterval(function() {
                                sec--;
                                if (sec == 0) {
                                    window.clearInterval(interval);
                                    layer.closeAll();
                                    getMobileMsg('连接超时！');
                                } else {
                                    $.ajax({
                                        type: "POST",
                                        url: actionUrl,
                                        data: "fname=net&opt=wan_info&function=get",
                                        dataType: "JSON",
                                        success: function(wanObject) {
                                            if (wanObject.error == 0) {
                                                if (wanObject.connected == 1) {
                                                    window.clearInterval(interval);
                                                    layer.closeAll();
                                                    getMobileMsg('中继成功！');
                                                    var mob = getMobile();
                                                    if (mob == 1) {
                                                        document.location = 'http://wiair.cc/m/user/index.html?tt=' + new Date().getTime();
                                                    } else {
                                                        document.location = 'http://wiair.cc/user/index.html?tt=' + new Date().getTime();
                                                    }
                                                } else if (wanObject.connected == 0 && sec < 1) {
                                                    window.clearInterval(interval);
                                                    layer.closeAll();
                                                    getMobileMsg('中继成功,联网失败！');
                                                }
                                            } else {
                                                locationUrl(wanObject.error);
                                            }
                                        }, error: function() {
                                            window.clearInterval(interval);
                                            layer.closeAll();
                                            getMobileMsg('连接超时,请重新连接路由器！');
                                            return;
                                        }
                                    });
                                }
                            }, 1000);
                        } else {
                            locationUrl(jsonWisp.error);
                        }
                    }
                });
            } else if (data.error != 0) {
                locationUrl(data.error);
                layer.closeAll();
            }
        }
    });
}

/*
 * 获取宽带账号
 */
function getPPPOEAccount() {
    $(".dot").hide();
    $(".txt").removeClass("hide").text("路由器正在获取中...");
    $(".d2").removeClass("error");
    $('#getAccount').val("获取中...");
    $('#getAccount').attr("disabled", true);
    timeoutCount = 30;
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=net&opt=wan_account&function=set",
        dataType: "json",
        success: function(data) {
            var jsonObject = data;
            if (jsonObject.error == 0) {
                $.ajax({
                    type: "POST",
                    url: actionUrl,
                    data: "fname=net&opt=wan_account&function=get",
                    dataType: "json",
                    success: function(data) {
                        var jsonObject = data;
                        if (jsonObject.error == 0) {
                            var mob = getMobile();
                            layer.closeAll();
                            if (mob == 1) {
                                layer.open({
                                    type: 1,
                                    title: false,
                                    shade: [0.7, '#000'],
                                    closeBtn: false,
                                    content: "<div class='wifiConnectLayer' style='height:auto;display:block;' id='sucLayer'><div class='bg'></div><div class='con con2'><div class='layer-suc png_ie'></div><p>获取成功</p><p>账号：<em id='account_get'></em></p><p>密码：<em id='pwd_get'></em></p><div class='box'><a id='net_work' class='btn layer-btn'>去拨号</a></div></div></div>",
                                    style: 'width:90%;background-color: transparent;  box-shadow: none;'
                                });
                            } else {
                                layer.open({
                                    type: 1,
                                    title: false,
                                    shade: [0.7, '#000'],
                                    closeBtn: false,
                                    content: $('#sucLayer'),
                                    skin: 'cy-class'
                                });
                            }
                            $('#account_get').empty().text(jsonObject.account);
                            $('#pwd_get').empty().text(jsonObject.passwd);
                        } else {
                            interval = setInterval(function() {
                                if (timeout > timeoutCount - 1) {
                                    timeout = 0;
                                    $(".dot").show();
                                    $(".d2").addClass("error");
                                    $(".txt").text("获取失败!");
                                    clearInterval(interval);
                                    $("#getAccount").removeAttr("disabled");
                                    $("#getAccount").val("重新获取");
                                }
                                if (!isGet) {
                                    $(".d1").delay(100).fadeIn();
                                    $(".d2").delay(400).fadeIn();
                                    $(".d3").delay(700).fadeIn();
                                    if (timeout != 0) {
                                        setTimeout(function() {
                                            $(".dot").fadeOut();
                                        }, 1200)
                                    }
                                    getAccountTimer();
                                    timeout++;
                                }
                            }, 3000);
                        }
                    }
                });
            }
        }
    });
}

function getAccountTimer() {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=net&opt=wan_account&function=get",
        dataType: "json",
        success: function(data) {
            var jsonObject = data;
            if (jsonObject.error == 0) {
                var mob = getMobile();
                clearInterval(interval);
                isGet = true;
                timeout = 0;
                layer.closeAll();
                if (mob == 1) {
                    layer.open({
                        type: 1,
                        title: false,
                        shade: [0.7, '#000'],
                        closeBtn: false,
                        content: "<div class='wifiConnectLayer' style='height:auto;display:block;' id='sucLayer'><div class='bg'></div><div class='con con2'><div class='layer-suc png_ie'></div><p>获取成功</p><p>账号：<em id='account_get'></em></p><p>密码：<em id='pwd_get'></em></p><div class='box'><a id='net_work' class='btn layer-btn'>去拨号</a></div></div></div>",
                        style: 'width:90%;background-color: transparent;  box-shadow: none;'
                    });

                } else {
                    layer.open({
                        type: 1,
                        title: false,
                        shade: [0.7, '#000'],
                        closeBtn: false,
                        content: $('#sucLayer'),
                        skin: 'cy-class'
                    });
                }
                $('#account_get').empty().text(jsonObject.account);
                $('#pwd_get').empty().text(jsonObject.passwd);
            }
        }
    });
}

function intervalTime(btn, btnText, msgText, mode) {
    responseReason = 0;
    interval = setInterval(function() {
        getWanInfoJsonObject();
        if ((wanInfoJsonObject.link == 0 && mode != 4) || responseReason == 19) {
            timeout = 15;
        }
        if (timeout > timeoutCount - 1) {
            if (wanInfoJsonObject.error == 0) {
                if (wanInfoJsonObject.connected == 0) {
                    if (wanInfoJsonObject.link == 0 && mode != 4) {
                        getMobileMsg("请插入网线！");
                    } else {
                        if (responseReason > 0) {
                            getMobileMsg(getErrorCode(responseReason));
                        } else {
                            getMobileMsg(msgText + '超时！');
                        }
                    }
                } else if (wanInfoJsonObject.connected == 1 && wanInfoJsonObject.mode == mode) {
                    toUrl();
                }
            } else {
                getMobileMsg(getErrorCode(wanInfoJsonObject.error));
            }
            if (mode == 4) {
                layer.closeAll();
                getMobileMsg(getErrorCode(wanInfoJsonObject.error));
            }
            $("#btn_" + btn).attr("disabled", false);
            $("#btn_" + btn).val("重新" + btnText);
            timeout = 0;
            clearInterval(interval);
            return;
        }
        if (wanInfoJsonObject.connected == 0 || wanInfoJsonObject.mode != mode) {
            timeout++;
        } else if (wanInfoJsonObject.connected == 1 && wanInfoJsonObject.mode == mode) {
            toUrl();
        }
    }, 4000);
}

function getHostRaMac(type, Id) {
    if (Id == 'macChoose1') {
        Id = 1;
    } else if (Id == 'macChoose2') {
        Id = 2
    } else if (Id == 'macChoose3') {
        Id = 3;
    }
    if (wanConfigJsonObject.mode != '' || wanConfigJsonObject.mode !== undefined) {
        if (type == 2) {
            $("#mac_box" + Id).show().text(wanConfigJsonObject.hostmac);
        } else if (type == 3) {
            $("#mac_box" + Id).show().text(wanConfigJsonObject.rawmac);
        }
    }
}

function getCloneMac(clone) {
    var mac;
    if (clone == 2) {
        mac = wanConfigJsonObject.hostmac;
    } else if (clone == 3) {
        mac = wanConfigJsonObject.rawmac;
    }
    return mac;
}