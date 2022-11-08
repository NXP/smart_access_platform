package com.smartaccessmanager.service;

public interface BLEProtocol {
    /*
     * transmission protocol
     */
    public final byte AUTHENTICATION_REQ = 0;
    public final byte AUTHENTICATION_RES = 1;
    public final byte UPDATE_PASSWORD_REQ = 2;
    public final byte UPDATE_PASSWORD_RES = 3;
    public final byte REGISTRATION_REQ = 4;
    public final byte REGISTRATION_RES = 5;
    public final byte DELETE_USER_REQ = 6;
    public final byte DELETE_USER_RES = 7;
    public final byte GET_USER_COUNT_REQ = 8;
    public final byte GET_USER_COUNT_RES = 9;
    public final byte GET_USER_INFO_REQ = 10;
    public final byte GET_USER_INFO_RES = 11;
    public final byte UPDATE_USER_INFO_REQ = 12;
    public final byte UPDATE_USER_INFO_RES = 13;
    public final byte REGISTRATION_CMD_REQ = 14;
    public final byte REGISTRATION_CMD_RES = 15;
    public final byte DEREGISTRATION_CMD_REQ = 16;
    public final byte DEREGISTRATION_CMD_RES = 17;
    public final byte PREV_CAMERA_SWITCH_CMD_REQ = 18;
    public final byte PREV_CAMERA_SWITCH_CMD_RES = 19;
    public final byte GET_APP_TYPE_REQ = 20;
    public final byte GET_APP_TYPE_RES = 21;
    public final byte GET_ALGO_VERSION_REQ = 22;
    public final byte GET_ALGO_VERSION_RES = 23;

    public final int VALID_PACKET = 0;
    public final int VALID_PACKET_FAILED = -1;
    public final int INVALID_PACKET = -2;

    public static final int REGISTRATION_RESULT_DUPLICATE = 1;

    public static final int USER_ID_SIZE = 4;
    public static final int USER_NAME_SIZE = 32;
    public static final int USER_STRUCT_SIZE = USER_ID_SIZE + USER_NAME_SIZE;

    final int packetLength[] = {
        6,      // AUTHENTICATION_REQ_PKT_LEN
        0,      // AUTHENTICATION_RES_PKT_LEN
        6,      // UPDATE_PASSWORD_REQ_PKT_LEN
        0,      // UPDATE_PASSWORD_RES_PKT_LEN
        432,    // REGISTRATION_REQ_PKT_LEN
        0,      // REGISTRATION_RES_PKT_LEN
        4,      // DELETE_USER_REQ_PKT_LEN
        0,      // DELETE_USER_RES_PKT_LEN
        0,      // GET_USER_COUNT_REQ_PKT_LEN
        0,      // GET_USER_COUNT_RES_PKT_LEN
        0,      // GET_USER_INFO_REQ_PKT_LEN
        0,     // GET_USER_INFO_RES_PKT_LEN
        36,     // UPDATE_USER_INFO_REQ_PKT_LEN
        0,      // UPDATE_USER_INFO_RES_PKT_LEN
        0,      // REGISTRATION_CMD_REQ_PKT_LEN
        0,      // REGISTRATION_CMD_RES_PKT_LEN
        0,      // DE-REGISTRATION_CMD_REQ_PKT_LEN
        0,      // DE-REGISTRATION_CMD_RES_PKT_LEN
        0,      // PREV_CAMERA_SWITCH_CMD_REQ_PKT_LEN
        0,      // PREV_CAMERA_SWITCH_CMD_RES_PKT_LEN
        0,      // GET_APP_TYPE_REQ
        0,      // GET_APP_TYPE_RES
        0,      // GET_ALGO_VERSION_REQ
        0,      // GET_ALGO_VERSION_RES
    };

    static final byte PACKET_HEADER_LEN = 24;

    /* send authentication Req */
    boolean sendAuthenticationReq(String password);

    /* send update password Req */
    boolean sendUpdatePasswordReq(String userID, String newPassword);

    /* send registration Req */
    boolean sendRegistrationReq(String faceID, String userName, byte[] faceFeature, boolean reRegister);

    /* send SetPWD Req */
    boolean sendSetPasswordReq(String name, String password);

    /* send EnrollFingerprint Req */
    boolean sendEnrollFingerprintReq(String userID);

    /* send RecordNfc Req*/
    boolean sendRecordNfcReq(String userID);

    /* send RecordUwb Req*/
    boolean sendRecordUwbReq(String userID);

    /* send delete user Req */
    boolean sendDeleteUserReq(int faceId);

    /* send get user count Req */
    boolean sendGetUserCountReq();

    /* send user infomation Req */
    boolean sendGetUserInfoReq();

    /* send update user information Req */
    boolean sendUpdateUserInfoReq(int faceId, String userName);

    /* send registration command Req */
    boolean sendRegistrationCmdReq(String userID);

    /* send deregistration command Req */
    boolean sendDeregistrationCmdReq();

    /* send preview camera switch command Req */
    boolean sendPreviewCameraSwitchReq();

    /* send get app type Req */
    boolean sendGetAppTypeReq();

    /* send get algorithm version Req */
    boolean sendGetAlgoVersionReq();

    /* wakeup call to keep alive */
    boolean sendWakeCall();

    /* send password unlock Req */
    boolean sendPasswordUnlockReq(String userID, String password);

    /* send Nfc unlock Req */
    boolean sendNfcUnlockReq(String userID);

    /* send Uwb unlock Req */
    boolean sendUwbUnlockReq(String userID);

    void handleRes(byte[] data);
}
