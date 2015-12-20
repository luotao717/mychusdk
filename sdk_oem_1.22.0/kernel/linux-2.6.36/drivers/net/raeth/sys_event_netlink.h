#ifndef __SYS_EVENT_NETLINK_H
#define __SYS_EVENT_NETLINK_H

#define NETLINK_SYS_MSG 25
#define U_PID 1
#define K_MSG 2
#define D_GRP 1

#define	 FALSE		0
#define	 TRUE		1

#define PROBE_SSID_MAX_LEN 32
#define MAC_STR_LEN         18

typedef enum
{
    BAND_TYPE_AUTO = 0,
    BAND_TYPE_5_8_G,
    BAND_TYPE_2_4_G,
    BAND_TYPE_ALL
}BAND_TYPE;

struct STA_INFO
{
    int Rssi;
    BAND_TYPE BandType;
    char SSID[PROBE_SSID_MAX_LEN];
};

typedef enum
{
    ACCESS_TYPE_WIRE = 1,
    ACCESS_TYPE_WIRELESS  
}ACCESS_TYPE;

typedef enum
{
    MSG_TYPE_ONLINE = 1,
    MSG_TYPE_OFFLINE,
    MSG_TYPE_PROBE,
    MSG_TYPE_NEW_MAC,
    MSG_TYPE_REMOVE_MAC
}NETWORK_STATUS_MSG;

typedef struct _NDIS_AP_802_11_STA_DEV_MSG
{
    NETWORK_STATUS_MSG MsgType;           // 1:online 2:offline 3:probe 4:mac increase 5:mac decrease
    ACCESS_TYPE AccessType;      // 1:wire;   2:wireless
    char ClientMac[MAC_STR_LEN];

    union
    {
        int Port;
        struct STA_INFO Info;
    }u;

#define MsgRssi u.Info.Rssi
#define MsgBandType u.Info.BandType
#define MsgSSID u.Info.SSID
}NDIS_AP_802_11_STA_DEV_MSG, PNDIS_AP_802_11_STA_DEV_MSG;

typedef enum
{
    /* kernel msg */
    ASEC_MSG_REPLY_ATTACK_QUERY = 1,    
    ASEC_MSG_ADD_PHISHING_WHITELIST,  
    ASEC_MSG_FLUSH_ACL_TABLE,               
    ASEC_MSG_FLUSH_ACCE_CACHE,             
    ASEC_MSG_SET_SWITCH_STATUS,            
    ASEC_MSG_SET_REDIRECT_URL,              

    /* user??????*/
    ASEC_MSG_REQUEST_ATTACK_QUERY,    
    ASEC_MSG_REPORT_ATTACK_EVENT,       
    ASEC_MSG_SET_DEBUG_LEVEL,                

    /* wireless tpsk & ie ???? */
    NL_MSG_TPSK_UPGRADE,                         
    NL_MSG_IE_REPORT,                               

    /* dev online status ???? */
    NL_MSG_DEV_ONLINE_STATUS               
}NETLINK_MSG_TYPE;

typedef struct
{
    NETLINK_MSG_TYPE MsgType;
    int MsgLen;
    void *Body;
}NDIS_AP_802_11_NETLINK_MSG_INFO, PNDIS_AP_802_11_NETLINK_MSG_INFO;

struct sock * init_sys_nl(void);
int mtk_nl_send_msg(void *msg, int msg_size);
int str2Mac (unsigned char *str, unsigned char *mac);
int mac2Str (unsigned char *mac, unsigned char *str, int bColon);

#endif
