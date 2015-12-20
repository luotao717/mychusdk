var commonInterval = 4000;
var ledStatus = false;
var MODEL = new Array("其它", "华为", "小米", "魅族", "磊科", "联想", "长虹", "HTC", "苹果", "三星", "诺基亚", "索尼", "酷派", "OPPO", "LG", "中兴", "步步高VIVO", "摩托罗拉", "金立", "天语", "TCL", "飞利浦", "戴尔", "惠普", "华硕", "东芝", "宏碁", "海信", "腾达", "TPLINK", "一加", "锤子", "瑞昱", "清华同方", "触云", "创维", "康佳", "乐视", "海尔", "夏普", "VMware", "Intel", "技嘉", "海华", "其它", "其它", "其它", "其它", "仁宝", "其它", "糖葫芦", "其它", "其它", "谷歌", "天珑", "泛泰", "其它", "其它", "微软", "极路由", "奇虎360", "水星", "华勤", "安利", "富士施乐", "D-Link", "网件", "阿里巴巴", "微星", "TCT", "必联");
var actionUrl = document.domain;
if (actionUrl != null && actionUrl != "") {
    actionUrl = "http://" + actionUrl + "/protocol.csp?"
} else {
    alert("获取路由器网关IP失败！");
}

var browser = {
    versions: function () {
        var u = navigator.userAgent, app = navigator.appVersion;
        return {
            trident: u.indexOf('Trident') > -1, //IE内核 
            presto: u.indexOf('Presto') > -1, //opera内核 
            webKit: u.indexOf('AppleWebKit') > -1, //苹果、谷歌内核 
            gecko: u.indexOf('Gecko') > -1 && u.indexOf('KHTML') == -1, //火狐内核 
            mobile: !!u.match(/AppleWebKit.*Mobile.*/), //是否为移动终端 
            ios: !!u.match(/\(i[^;]+;( U;)? CPU.+Mac OS X/), //ios终端 
            android: u.indexOf('Android') > -1, //android终端或uc浏览器 
            iPhone: u.indexOf('iPhone') > -1, //是否为iPhone或者QQHD浏览器 
            iPad: u.indexOf('iPad') > -1, //是否iPad 
            webApp: u.indexOf('Safari') == -1 //是否web应该程序，没有头部与底部 
        };
    }(),
    language: (navigator.browserLanguage || navigator.language).toLowerCase()
}

/*
 * 去重字符串两边空格
 */
function trim(str) {
    regExp1 = /^ */;
    regExp2 = / *$/;
    return str.replace(regExp1, '').replace(regExp2, '');
}

/*
 * 验证MAC
 */
function checkMac(mac) {
    mac = mac.toUpperCase();
    if (mac == "" || mac.indexOf(":") == -1) {
        return false;
    } else {
        var macs = mac.split(":");
        if (macs.length != 6) {
            return false;
        }
        for (var i = 0; i < macs.length; i++) {
            if (macs[i].length != 2) {
                return false;
            }
        }
        var reg_name = /[A-F\d]{2}:[A-F\d]{2}:[A-F\d]{2}:[A-F\d]{2}:[A-F\d]{2}:[A-F\d]{2}/;
        if (!reg_name.test(mac)) {
            return false;
        }
    }
    return true;
}

/*
 * 获取URL参数值
 */
function getQueryString(name) {
    var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)");
    var r = window.location.search.substr(1).match(reg);
    if (r != null) {
        return unescape(r[2]);
    }
    return null;
}

/*
 * 检查IP是否合法
 */
function checkIP(ip) {
    obj = ip;
    var exp = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
    var reg = obj.match(exp);
    if (reg == null) {
        return false;
    } else {
        return true;
    }
}
/*
 * 检查掩码是否合法
 */
function checkMask(mask) {
    obj = mask;
    var exp = /^(254|252|248|240|224|192|128|0)\.0\.0\.0|255\.(254|252|248|240|224|192|128|0)\.0\.0|255\.255\.(254|252|248|240|224|192|128|0)\.0|255\.255\.255\.(254|252|248|240|224|192|128|0)$/;
    var reg = obj.match(exp);
    if (reg == null) {
        return false;
    } else {
        return true;
    }
}

/*
 * 功能：实现IP地址，子网掩码，网关的规则验证
 * 参数：IP地址，子网掩码，网关
 * 返回值：BOOL
 */
var validateNetwork = function (ip, netmask, gateway) {
    var parseIp = function (ip) {
        return ip.split(".");
    }
    var conv = function (num) {
        var num = parseInt(num).toString(2);
        while ((8 - num.length) > 0)
            num = "0" + num;
        return num;
    }
    var bitOpera = function (ip1, ip2) {
        var result = '', binaryIp1 = '', binaryIp2 = '';
        for (var i = 0; i < 4; i++) {
            if (i != 0)
                result += ".";
            for (var j = 0; j < 8; j++) {
                result += conv(parseIp(ip1)[i]).substr(j, 1) & conv(parseIp(ip2)[i]).substr(j, 1)
            }
        }
        return result;
    }
    var ip_re = /^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$/;
    if (ip == null || netmask == null || gateway == null) {
        return false;
    }
    if (!ip_re.test(ip) || !ip_re.test(netmask) || !ip_re.test(gateway)) {
        return false
    }
    return bitOpera(ip, netmask) == bitOpera(netmask, gateway);
}

/*
 * 获取错误码
 */
function getErrorCode(type) {
    switch (type) {
        case 19 :
            return "宽带账号/密码错误！";
            break;
        case 20 :
            return "运营商拨号无响应！";
            break;
        case 10001:
            return "获取参数失败！";
            break;
        case 10002:
            return "输入参数错误！";
            break;
        case 10003:
            return "路由内存不足！";
            break;
        case 10004:
            return "添加的条目已存在！";
            break;
        case 10005:
            return "删除的条目不存在！";
            break;
        case 10006:
            return"添加条目已满！";
            break;
        case 10007:
            return "没登陆！";
            break;
        case 10008:
            return "不支持！";
            break;
        case 10009:
            return"没获取到账号！";
            break;
        case 10010:
            return "时间过期！";
            break;
        default :
            return "请求超时，错误码：" + type;
    }
}

/*
 * 检测是否是手机
 */
function getMobile() {
    var mob = 0;
    if (browser.versions.mobile || browser.versions.android || browser.versions.iPhone) {
        mob = 1;
    }
    return mob;
}

function getMobileMsg(msg, type, sid) {
    var mob = getMobile();
    if (mob == 1) {
        return tips(msg);
    } else if (type == 1 && mob != 1) {
        layer.tips(msg, sid, {
            tips: [1, '#000000']
        });
    } else {
        return layer.msg(msg);
    }
}

/*
 * 设置wifi账户密码
 */
function setWifiAp(ssid, pwd, channel, hidden_ssid, wifiBandwidth) {
    var dat = "fname=net&opt=wifi_ap&function=set&ssid=" + ssid + "&passwd=" + pwd + "&hidden=" + hidden_ssid + "&channel=" + channel + "&bw=" + wifiBandwidth;
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: dat,
        dataType: "json",
        success: function (data) {
            if (data.error == 0) {
                var mob = getMobile();
                if (mob == 1) {
                    getMobileMsg('设置成功！请关闭浏览器,重新打开wifi连接！');
                } else {
                    getMobileMsg('设置成功！');
                }
            } else {
                locationUrl(data.error);
            }
        }
    });
}

function getStrLength(str) {
    var realLength = 0, len = str.length, charCode = -1;
    for (var i = 0; i < len; i++) {
        charCode = str.charCodeAt(i);
        if (charCode > 0 && charCode <= 128) {
            realLength += 1;
        } else {
            realLength += 3;
        }
    }
    return realLength;
}


function getLedLogin() {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=system&opt=led&function=set&act=dump",
        async: false,
        dataType: "json",
        success: function (data) {
            if (data.error == 0) {
                ledStatus = true;
            } else {
                ledStatus = false;
            }
        }, error: function () {
            ledStatus = false;
        }
    });
    return ledStatus;
}



function getSetPrice(set, price) {
    var dat = 'fname=net&opt=advert&function=get';
    if (set == 'set') {
        dat = 'fname=net&opt=advert&function=set&enable=' + price;
    }
    $.ajax({
        type: "POST",
        url: actionUrl + dat,
        async: false,
        dataType: "JSON",
        success: function (data) {
            if (data.error == 0) {
                var mob = getMobile();
                if (set == 'get') {
                    if (data.enable == 1) {
                        $("#r1").attr('checked', true);
                        $("#r2").attr('checked', false);
                    } else {
                        $("#r2").attr('checked', true);
                        $("#r1").attr('checked', false);
                    }
                } else {
                    layer.closeAll();
                    if (price == 1) {
                        getMobileMsg('开启成功！');
                    } else {
                        getMobileMsg('关闭成功！');
                    }
                }
            } else {
                locationUrl(data.error);
            }
        }
    });
}

function wifiGet() {
    $.ajax({
        type: "POST",
        url: actionUrl + "fname=&opt=wifi_lt&function=get",
        dataType: "JSON",
        success: function (data) {
            if (data.error == 0) {
                router.getWifiAp();
                if (data.enable == 1) {
                    $("#wifiOnOff").show();
                    $("#r1").attr('checked', true);
                    $("#r2").attr('checked', false);
                } else {
                    $("#wifiOnOff").hide();
                    $("#r1").attr('checked', false);
                    $("#r2").attr('checked', true);
                }
            } else {
                locationUrl(data.error);
            }
        }
    });
}

function wifiSet(enable) {
    $.ajax({
        type: "POST",
        url: actionUrl + "fname=net&opt=wifi_lt&function=set&enable=" + enable + "&time_on=0&week=0000000&sh=0&sm=0&eh=0&em=0",
        dataType: "JSON",
        success: function (data) {
            if (data.error == 0) {
                if (enable == 0) {
                    $("#wifiOnOff").hide();
                }
                getMobileMsg('设置成功！');
            } else {
                locationUrl(data.error);
            }
        }
    });
}



var router = {
    //获取系统版本
    getVersion: function () {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: "fname=system&opt=firmstatus&function=get&flag=local",
            dataType: "json",
            success: function (data) {
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    $("#version").html(jsonObject.localfirm_version);
                } else {
                    $("#version").html("获取系统版本失败！");
                }
            }
        });
    },
    //获取网络类型
    getWanInfo: function () {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: "fname=net&opt=wan_info&function=get",
            dataType: "json",
            success: function (data) {
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    $("#netType").html(getWanNetType(jsonObject.mode));
                    $("#wanIP").html(jsonObject.ip);
                    var mob = getMobile();
                    if (mob == 1) {
                        $('#macTy').val(jsonObject.mac.toUpperCase()).siblings("label").hide();
                    } else {
                        $('#macTy').html(jsonObject.mac);
                    }
                } else {
                    $("#netType").html("获取网络类型失败！");
                    $("#wanIP").html("获取外网IP失败！");
                }
            }
        });
    },
    //获取model.js
    getModel: function () {
        $.getScript('http://static.wiair.com/conf/model.js', function () {
            MODEL = MODEL;
        });
    },
    //获取LAN IP
    getLanIP: function () {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: "fname=net&opt=dhcpd&function=get",
            dataType: "json",
            success: function (data) {
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    $("#lanIP").html(jsonObject.ip);
                } else {
                    $("#lanIP").html("获取本机IP失败！");
                }
            }
        });
    },
    //获取QOS信息
    getQos: function () {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: "fname=net&opt=qos&function=get",
            dataType: "json",
            success: function (data) {
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    if (jsonObject.enable == 0) {
                        $("#totalSpeed").html("未知");
                    } else {
                        $("#totalSpeed").html((jsonObject.down / 1024).toFixed(0) + "Mb");
                    }
                }
            }
        });
    },
    //获取路由器动态信息
    getDynamicInfo: function () {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: "fname=system&opt=main&function=get",
            dataType: "json",
            success: function (data) {
                router.getWanInfo();
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    if (jsonObject.connected == true) {
                        $("#netStatus").html('已连接');
                    } else {
                        $("#netStatus").html('未连接');
                    }
                    if (jsonObject.cur_speed > 1024) {
                        $("#curSpeed").html((jsonObject.cur_speed / 1024).toFixed(2) + "M/S");
                    } else {
                        $("#curSpeed").html(jsonObject.cur_speed + "KB/S");
                    }
                    //$("#totalSpeed").html((jsonObject.total_speed / 1024 * 8).toFixed(0) + "M");
                    var devices = "";
                    var terminals = jsonObject.terminals;
                    var name;
                    var moble = getMobile();
                    var sort = 0;
                    var mac_mark = '';
                    if (terminals != null && terminals.length > 0) {
                        for (var i = 0; i < terminals.length; i++) {
                            var flag = terminals[i].flag;
                            if (flag.charCodeAt(1) == 84) {	//T
                                var model = "";
                                if (MODEL != null) {
                                    if (MODEL[terminals[i].vendor] === undefined) {
                                    } else {
                                        model = MODEL[terminals[i].vendor] + " - ";
                                    }
                                }
                                if (terminals[i].speed > 1024) {
                                    terminals[i].speed = (terminals[i].speed / 1024).toFixed(2) + 'm/s';
                                } else {
                                    terminals[i].speed = terminals[i].speed + 'KB/s';
                                }
                                sort++;
                                var used_time = (terminals[i].ontime / 3600).toFixed(2) + "小时";
                                var device_name;
                                if (terminals[i].flag.substr(6, 1) == 'T') {
                                    device_name = terminals[i].name;
                                } else if (terminals[i].vendor == 34) {
                                    mac_mark = terminals[i].mac.replace(/:/g, "");
                                    device_name = '触云-' + mac_mark.substr(-4, 4);
                                } else {
                                    device_name = model + terminals[i].name;
                                }
                                device_name = device_name.length > 17 ? device_name.substr(0, 15) + '...' : device_name;

                                if (moble == 1) {
                                    name = "<li><span class='s-1'>" + sort + "</span>" + "<span class='s-2'>" + device_name + "</span><span class='s-3'>" + terminals[i].speed + "</span><span>" + used_time + "</span></li>";
                                } else {
                                    name = "<tr><td class=first width=32>" + sort + "</td>" + "<td width=130>" + device_name + "</td><td>" + terminals[i].speed + "</td></tr>";
                                }
                                devices += name;
                            }
                        }
                    }
                    $("#count_devices").html('设备连接总数：' + sort);
                    $("#devices").html(devices);
                } else {
                    $("#devices").html("");
                    locationUrl(data.error);
                    //$("#totalSpeed").html("--");
                }
            }, complete: function (XHR, TS) {
                XHR = null;
            }, error: function (XHRequest, status, data) {
                XHRequest.abort();
                clearPlotChart(g_typePlot);
                typeChartTimer();
            }
        });
    },
    //获取设备SSID
    getWifiAp: function () {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: 'fname=net&opt=wifi_ap&function=get',
            dataType: 'JSON',
            success: function (data) {
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    $("#channel").val(jsonObject.channel);
                    var mob = getMobile();
                    if (jsonObject.hidden == 1) {
                        $("#hidden_ssid").attr("checked", true);
                    }
                    if (mob == 1) {
                        $('#wifi_name').val(jsonObject.ssid).siblings("label").hide();
                        if (trim(jsonObject.passwd) != '') {
                            $('#wifi_password').val(jsonObject.passwd).siblings("label").hide();
                        }
                    } else {
                        $('#ssid').val(jsonObject.ssid).siblings("label").hide();
                        if (trim(jsonObject.passwd) != '') {
                            $('#wifi_pwd').val(jsonObject.passwd).siblings("label").hide();
                        }
                        $('#wifi_name').text(jsonObject.ssid).siblings("label").hide();
                        $('#wifi_password').text(jsonObject.passwd).siblings("label").hide();
                    }
                    $("#wifiChannel option[value=" + jsonObject.channel + "]").attr('selected', 'selected');
                    $("#wifiBandwidth option[value=" + jsonObject.bw + "]").attr('selected', 'selected');
                } else {
                    if (mob == 1) {
                        $('#wifi_name').val('');
                        $('#wifi_password').val('');
                    } else {
                        $('#wifi_name').val('');
                        $('#wifi_password').val('');
                    }
                }
            }
        });
    },
    getLedInfo: function () {
        $.ajax({
            type: "POST",
            url: actionUrl,
            data: 'fname=system&opt=led&function=set&act=dump',
            dataType: 'JSON',
            success: function (data) {
                var jsonObject = data;
                if (jsonObject.error == 0) {
                    if (jsonObject.led == 1) {
                        $("#led_on_off").html("<div class='light l-2'><img src=../images/light.png></div><div class='light l-3'><img src=../images/light.png></div><div class='light l-4'><img src=../images/light.png></div><div class='light l-5'><img src=../images/light.png></div>");
                    } else if (jsonObject.led == 0) {
                        $("#led_on_off").empty();
                    }
                }
            }
        });
    }
}
//获取登录信息
function getLoginInfo() {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=system&opt=led&function=set&act=dump",
        dataType: 'JSON',
        success: function (data) {
            var jsonObject = data;
            if (jsonObject.error != 0) {
                var mob = getMobile();
                if (mob == 1) {
                    document.location = 'http://wiair.cc/m/index.html?tt=' + new Date().getTime();
                } else {
                    document.location = 'http://wiair.cc/index.html?tt=' + new Date().getTime();
                }
            }
        }

    });
}

//获取网络类型字符串
function getWanNetType(type) {
    switch (type) {
        case 1 :
            return "动态IP";
            break;
        case 2 :
            return "PPPOE";
            break;
        case 3 :
            return "静态IP";
            break;
        case 4 :
            return "WISP";
            break;
        default :
            return "获取网络类型失败";
    }
}

//设置路由登录账户
function setUserAccount(pwd) {
    $.ajax({
        type: "POST",
        url: actionUrl,
        data: "fname=system&opt=login_account&function=set&user=admin" + "&password=" + pwd,
        dataType: "JSON",
        success: function (data) {
            if (data.error == 0) {
                var mob = getMobile();
                if (mob == 1) {
                    $.cookie('lstatus', false, {path: '/m'});
                } else {
                    $.cookie('lstatus', false, {path: '/'});
                }
                document.location = 'http://' + document.domain + '/index.html?tt=' + new Date().getTime();
            } else {
                locationUrl(data.error);
            }
        }
    });
}

function locationUrl(error) {
    if (error == 10007) {
        getMobileMsg('请重新登录！');
        setTimeout(function () {
            if (mob == 1) {
                $.cookie('lstatus', false, {path: '/m'});
            } else {
                $.cookie('lstatus', false, {path: '/'});
            }
            document.location = 'http://' + document.domain + "/index.html?tt=" + new Date().getTime();
        }, 3000);
    } else {
        getMobileMsg(getErrorCode(error));
    }
}




//获取Router Info
function init() {
    router.getDynamicInfo();
    router.getModel();
    router.getVersion();
    router.getLanIP();
    router.getQos();
    router.getWifiAp();
    router.getLedInfo();
    setInterval(function () {
        router.getDynamicInfo();
    }, commonInterval);
}