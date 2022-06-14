package com.smartaccessmanager.service;

import android.nfc.Tag;
import android.util.Log;

import com.smartaccessmanager.activity.SmartAccessActivity;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.model.BLEAttributes;

import org.greenrobot.eventbus.EventBus;
import org.jetbrains.annotations.NotNull;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.zip.CRC32;

public class BLESmartAccessProtocol implements BLEProtocol {
    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_BLS";

    public final byte SETPWD_REQ = 24;
    public final byte SETPWD_RES = 25;
    public final byte ENROLL_FINGERPRINT_REQ = 26;
    public final byte ENROLL_FINGERPRINT_RES = 27;
    public final byte RECORD_NFC_REQ = 28;
    public final byte RECORD_NFC_RES = 29;
    public final byte PASS_UNLOCK_REQ = 30;
    public final byte PASS_UNLOCK_RES = 31;
    public final byte NFC_UNLOCK_REQ = 32;
    public final byte NFC_UNLOCK_RES = 33;
    public final byte UWB_UNLOCK_REQ = 34;
    public final byte UWB_UNLOCK_RES = 35;
    public final byte RECORD_UWB_REQ = 36;
    public final byte RECORD_UWB_RES = 37;

    public final String AT_PREFIX = "AT+";
    public final String AT_SUFFIX = "\r\n";
    public final String SETPWD_AT= AT_PREFIX + "PWD=";
    public final String ENROLL_FINGERPRINT_AT= AT_PREFIX + "FINGERPRINT=";
    public final String ELOCK_AT= AT_PREFIX + "ELOCK=";
    public final String RECORD_NFC_AT= AT_PREFIX + "NFC=";
    public final String RECORD_UWB_AT= AT_PREFIX + "UWB=";
    public final String PASS_UNLOCK_AT= AT_PREFIX + "UNLOCKPASS=";
    public final String NFC_UNLOCK_AT= AT_PREFIX + "UNLOCKNFC=";
    public final String UWB_UNLOCK_AT= AT_PREFIX + "UNLOCKUWB=";
    public final String AUTHENTICATION_AT = AT_PREFIX + "AUTH=";
    public final String UPDATE_PASSWORD_AT = AT_PREFIX + "UPDTUSERPASS=";
    public final String REGISTRATION_AT = AT_PREFIX + "FACERREG=";
    public final String WAKE_CALL = AT_PREFIX + "WAKEUP=";
    public final String DELETE_USER_AT = AT_PREFIX + "USERDEL=";
    public final String GET_USER_COUNT_AT = AT_PREFIX + "GETUSERNO=";
    public final String GET_USER_INFO_AT = AT_PREFIX + "GETINFO=";
    public final String UPDATE_USER_INFO_AT = AT_PREFIX + "UPDTUSER=";
    public final String REGISTRATION_CMD_AT = AT_PREFIX + "FACEREG=";
    public final String DEREGISTRATION_CMD_AT = AT_PREFIX + "FACEDREG=";
    public final String GET_APP_TYPE_AT = AT_PREFIX + "APPTYPE=";
    public final String GET_ALGO_VERSION_AT = AT_PREFIX + "ALGOVERSION=";

    public final String RESPONSE_OK = "OK";
    public final String RESPONSE_FAIL = "FAIL";
    public final String RESPONSE_DUPLICATE = "DUPLICATE";
    public final byte DUPLICATE_RESULT = 1;

    private int mPacketID = 0;
    private byte mReqPacketType;
    private int mReqPacketID;
    private byte mResPacketType;

    private int packetDataLength = 0;
    private ByteArrayOutputStream packetData;
    private int packetReserved = 0;

    public void dumpBytes(byte[] bytes, int len) {
        Log.d(TAG, "bytes:[" + bytes.length + "]");
        int line_len = 16;
        //int bytes_len = bytes.length;
        // only 16 header + one 128 bytes face feature as the other two 128 face feature are same as the first one
        //if (face_feature_len > (128 + 16)) {
        //face_feature_len = 128 + 16;
        //}
        int lines = len / line_len;
        int remains = len % line_len;
        int i = 0;
        for (; i < lines; i++) {
            //Log.d(TAG, bytes2HexString(faceFeature + i * line_len, line_len));
            Log.d(TAG, String.format("0x%02x ",bytes[i * line_len]) + String.format("0x%02x ",bytes[i * line_len +1]) +
                    String.format("0x%02x ",bytes[i * line_len+2]) + String.format("0x%02x ",bytes[i * line_len+3]) +
                    String.format("0x%02x ",bytes[i * line_len+4]) + String.format("0x%02x ",bytes[i * line_len+5]) +
                    String.format("0x%02x ",bytes[i * line_len+6]) + String.format("0x%02x ",bytes[i * line_len+7]) +
                    String.format("0x%02x ",bytes[i * line_len+8]) + String.format("0x%02x ",bytes[i * line_len+9]) +
                    String.format("0x%02x ",bytes[i * line_len+10]) + String.format("0x%02x ",bytes[i * line_len+11]) +
                    String.format("0x%02x ",bytes[i * line_len+12]) + String.format("0x%02x ",bytes[i * line_len+13]) +
                    String.format("0x%02x ",bytes[i * line_len+14]) + String.format("0x%02x ",bytes[i * line_len+15]));
        }

        String strRemains = "";
        for (int j = 0; j < remains; j++) {
            strRemains += String.format("0x%02x ",bytes[i * line_len + j]);
        }
        if (remains > 0) {
            Log.d(TAG, strRemains);
        }
    }

    public boolean chunkSend(byte[] data, int length)
    {
        int chunkSize = 20;
        int chunkCount = length / chunkSize;
        int remain = length % chunkSize;
        boolean ret = true;

        for (int i = 0; i < chunkCount; i++) {
            byte[] chunk = Arrays.copyOfRange(data, i * chunkSize, (i + 1)*chunkSize);
            ret = BLEService.INSTANCE.requestWrite(BLEAttributes.WUART, BLEAttributes.UART_STREAM, chunk);

            if (ret == false) {
                Log.e(TAG, "  send chunk:[" + i + "] failed");
                mReqPacketType = INVALID_PACKET;
                mReqPacketID = 0;
                return ret;
            }

            Log.e(TAG, "  send chunk:[" + i + "] ok");
            dumpBytes(chunk, chunk.length);
        }
        if (remain > 0) {
            byte[] chunk = new byte[remain];
            System.arraycopy(data, chunkCount * chunkSize, chunk, 0, remain);
            ret = BLEService.INSTANCE.requestWrite(BLEAttributes.WUART, BLEAttributes.UART_STREAM, chunk);

            if (ret == false) {
                Log.e(TAG, "  send remain failed");
                mReqPacketType = INVALID_PACKET;
                mReqPacketID = 0;
                return ret;
            }
            Log.e(TAG, "  send remain ok");
            dumpBytes(chunk, chunk.length);
        }

        return ret;
    }

    public boolean sendPacket(byte[] data, int length, byte type) {
        boolean ret = false;

        Log.d(TAG, "Sending BLE packet: " + new String(data, StandardCharsets.UTF_8));

        mReqPacketType = type;
        mResPacketType = INVALID_PACKET;

        if (data != null) {
            ret = chunkSend(data, length);
            if (ret == false) {
                Log.d(TAG, "ERROR:send pkt");
                mReqPacketType = INVALID_PACKET;
                mReqPacketID = 0;
                return ret;
            }
        }

        mResPacketType = (byte) (type + 1);
        Log.d(TAG, "-sendPacket:");
        return true;
    }

    /* send set password Req */
    public boolean sendSetPasswordReq(String name, String password){
        byte[] pkt_data = (SETPWD_AT + name + ',' + password + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, SETPWD_REQ);
    }

    /* send fingerprint Req */
    public boolean sendEnrollFingerprintReq(String userID){
        byte[] pkt_data = (ENROLL_FINGERPRINT_AT + userID + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, ENROLL_FINGERPRINT_REQ);
    }

    /* send nfc Req */
    public boolean sendRecordNfcReq(String userID){
        byte[] pkt_data = (RECORD_NFC_AT + userID + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, RECORD_NFC_REQ);
    }

    /* send RecordUwb Req*/
    public boolean sendRecordUwbReq(String userID){
        byte[] pkt_data = (RECORD_UWB_AT + userID + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, RECORD_UWB_REQ);
    }

    /* send password unlock Req */
    public boolean sendPasswordUnlockReq(String userID, String password){
        byte[] pkt_data = (PASS_UNLOCK_AT + userID + ',' + password + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, PASS_UNLOCK_REQ);
    }

    /* send Nfc unlock Req */
    public boolean sendNfcUnlockReq(String userID){
        byte[] pkt_data = (NFC_UNLOCK_AT + userID + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, NFC_UNLOCK_REQ);
    }

    /* send Uwb unlock Req */
    public boolean sendUwbUnlockReq(String userID){
        byte[] pkt_data = (UWB_UNLOCK_AT + userID + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, UWB_UNLOCK_REQ);
    }

    /* send authentication Req */
    public boolean sendAuthenticationReq(String password){
        byte[] pkt_data = (AUTHENTICATION_AT + password + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, AUTHENTICATION_REQ);
    }

    /* send update password Req */
    public boolean sendUpdatePasswordReq(String userID, String newPassword){
        byte[] pkt_data = (UPDATE_PASSWORD_AT + userID + ',' + newPassword + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, UPDATE_PASSWORD_REQ);
    }

    /* send registration Req */
    public boolean sendRegistrationReq(String faceID, String userName, byte[] faceFeature, boolean reRegister) {
        Log.d(TAG, "+sendRegistrationReq");

        byte[] fixedFaceID = new byte[USER_ID_SIZE];
        for (int index = 0; index < USER_ID_SIZE; index++) {
            if (index < faceID.length()) {
                fixedFaceID[index] = faceID.getBytes()[index];
            } else {
                fixedFaceID[index] = 0;
            }
        }

        byte[] fixedUserName = new byte[USER_NAME_SIZE];
        for (int index = 0; index < USER_NAME_SIZE; index++) {
            if (index < userName.length()) {
                fixedUserName[index] = userName.getBytes()[index];
            } else {
                fixedUserName[index] = 0;
            }
        }

        ByteArrayOutputStream outputStream = new ByteArrayOutputStream( );
        try {
            outputStream.write((REGISTRATION_AT).getBytes());
            outputStream.write((byte)(reRegister ? 1 : 0));
            outputStream.write(fixedFaceID);
            outputStream.write(fixedUserName);
            outputStream.write(faceFeature);
            outputStream.write(AT_SUFFIX.getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
        byte[] pkt_data = outputStream.toByteArray( );
        int pkt_len = pkt_data.length;

        Log.d(TAG, "length: " + pkt_len);

        // send the pkt
        boolean ret = sendPacket(pkt_data, pkt_len, REGISTRATION_REQ);

        Log.d(TAG, "-sendRegistrationReq:" + ret);
        return ret;
    }

    /* send wake call */
    public boolean sendWakeCall(){
        byte[] pkt_data = (WAKE_CALL + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, (byte) 100);
    }

    /* send delete user Req */
    public boolean sendDeleteUserReq(int faceId){
        byte[] pkt_data = (DELETE_USER_AT + faceId + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, DELETE_USER_REQ);
    }

    /* send get user count Req */
    public boolean sendGetUserCountReq(){
        byte[] pkt_data = (GET_USER_COUNT_AT + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, GET_USER_COUNT_REQ);
    }

    /* send user infomation Req */
    public boolean sendGetUserInfoReq(){
        byte[] pkt_data = (GET_USER_INFO_AT + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, GET_USER_INFO_REQ);
    }

    /* send update user information Req */
    public boolean sendUpdateUserInfoReq(int faceId, String userName){
        byte[] pkt_data = (UPDATE_USER_INFO_AT+ faceId + ',' + userName + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, UPDATE_USER_INFO_REQ);
    }

    /* send registration command Req */
    public boolean sendRegistrationCmdReq(String userID){
        byte[] pkt_data = (REGISTRATION_CMD_AT + userID + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, REGISTRATION_CMD_REQ);
    }

    /* send deregistration command Req */
    public boolean sendDeregistrationCmdReq(){
        byte[] pkt_data = (DEREGISTRATION_CMD_AT + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, DEREGISTRATION_CMD_REQ);
    }

    /* send preview camera switch command Req */
    public boolean sendPreviewCameraSwitchReq(){
        return false;
    }

    /* send get app type Req */
    public boolean sendGetAppTypeReq(){
        byte[] pkt_data = (GET_APP_TYPE_AT + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, GET_APP_TYPE_REQ);
    }

    /* send get algorithm version Req */
    public boolean sendGetAlgoVersionReq(){
        byte[] pkt_data = (GET_ALGO_VERSION_AT + AT_SUFFIX).getBytes();
        return sendPacket(pkt_data, pkt_data.length, GET_ALGO_VERSION_REQ);
    }

    public int CheckResPacket(@NotNull byte[] data, byte packet_type, int packet_len) {
        long tu_crc = 0;

        CRC32 crc32 = new CRC32();
        crc32.reset();

        for (int i = 0; i < (PACKET_HEADER_LEN - 4); i++) {
            crc32.update(data[i]);
        }

        tu_crc = crc32.getValue();

        /* pkt magic check */
        if ((data[0] != 0x53) || (data[1] !=  0x79) || (data[2] != 0x4c)) {
            Log.d(TAG, "  ERROR:invalid tu_type:" + data[0]);
            return -2;
        }

        byte pkt_type =   data[3];
        long pkt_len =    (long)((((data[7] & 0xff) << 24)  | ((data[6] & 0xff) << 16)  | ((data[5] & 0xff) << 8)  | (data[4] & 0xff)) & 0xFFFFFFFFL);
        long pkt_id =     (long)((((data[11] & 0xff) << 24) | ((data[10] & 0xff) << 16) | ((data[9] & 0xff) << 8)  | (data[8] & 0xff)) & 0xFFFFFFFFL);
        long pkt_crc =    (long)((((data[15] & 0xff) << 24) | ((data[14] & 0xff) << 16) | ((data[13] & 0xff) << 8) | (data[12] & 0xff)) & 0xFFFFFFFFL);
        long reserved =   (long)((((data[19] & 0xff) << 24) | ((data[18] & 0xff) << 16) | ((data[17] & 0xff) << 8) | (data[16] & 0xff)) & 0xFFFFFFFFL);
        long header_crc = (long)((((data[23] & 0xff) << 24) | ((data[22] & 0xff) << 16) | ((data[21] & 0xff) << 8) | (data[20] & 0xff)) & 0xFFFFFFFFL);

        if (pkt_type != packet_type) {
            Log.d(TAG, "  ERROR:invalid pkt_type:" + data[1]);
            return -3;
        }

        if (pkt_len != packet_len) {
            Log.d(TAG, "  ERROR:invalid pkt_len:" + pkt_len);
            return -4;
        }

        if (pkt_id != mReqPacketID) {
            Log.d(TAG, "  ERROR:invalid pkt_id:" + pkt_id + ":" + mReqPacketID);
            return -5;
        }

        if (header_crc != tu_crc) {
            Log.d(TAG, "  ERROR:invalid header_crc:" + header_crc + ":" + String.format("0x%08x", header_crc) + ":" + tu_crc);
            return -7;
        }

        return 0;
    }

    public void notifyRes(int packet_type, int result) {
        /* The received packets is OK */
        switch (packet_type) {
            case AUTHENTICATION_RES:
                EventBus.getDefault().post(new BLEStateEvent.AuthenticationRes(result));
                break;
            case UPDATE_PASSWORD_RES:
                EventBus.getDefault().post(new BLEStateEvent.UpdatePasswordRes(result));
                break;
            case REGISTRATION_RES:
                EventBus.getDefault().post(new BLEStateEvent.RegistrationRes(
                        result, new String(packetData.toByteArray())));
                break;
            case DELETE_USER_RES:
                EventBus.getDefault().post(new BLEStateEvent.DeleteUserRes(result));
                break;
            case GET_USER_COUNT_RES:
                EventBus.getDefault().post(new BLEStateEvent.GetUserCountRes(result, Integer.parseInt(packetData.toString())));
                break;
            case GET_USER_INFO_RES:
                EventBus.getDefault().post(new BLEStateEvent.GetUserInfoRes(
                        result, SmartAccessActivity.userCount, packetData.toString()));
                break;
            case UPDATE_USER_INFO_RES:
                EventBus.getDefault().post(new BLEStateEvent.UpdateUserInfoRes(result));
                break;
            case REGISTRATION_CMD_RES:
                EventBus.getDefault().post(new BLEStateEvent.RegistrationCMDRes(result));
                break;
            case DEREGISTRATION_CMD_RES:
                EventBus.getDefault().post(new BLEStateEvent.DeregistrationCMDRes(result));
                break;
            case PREV_CAMERA_SWITCH_CMD_RES:
                EventBus.getDefault().post(new BLEStateEvent.PrevCameraSwitchCMDRes(result));
                break;
            case GET_APP_TYPE_RES:
                EventBus.getDefault().post(new BLEStateEvent.GetAppTypeRes(result, packetData.toString()));
                break;
            case GET_ALGO_VERSION_RES:
                EventBus.getDefault().post(new BLEStateEvent.GetAlgoVersionRes(result));
                break;
            case SETPWD_RES:
                EventBus.getDefault().post(new BLEStateEvent.SetPwdRes(result, packetData.toString()));
                break;
            case ENROLL_FINGERPRINT_RES:
                EventBus.getDefault().post(new BLEStateEvent.EnrollFingerprintRes(result));
                break;
            case RECORD_NFC_RES:
                EventBus.getDefault().post(new BLEStateEvent.RecordNfcRes(result));
                break;
            case RECORD_UWB_RES:
                EventBus.getDefault().post(new BLEStateEvent.RecordUwbRes(result));
                break;
            case PASS_UNLOCK_RES:
                EventBus.getDefault().post(new BLEStateEvent.PasswordUnlockRes(result));
                break;
            case NFC_UNLOCK_RES:
                EventBus.getDefault().post(new BLEStateEvent.NfcUnlockRes(result));
                break;
            case UWB_UNLOCK_RES:
                EventBus.getDefault().post(new BLEStateEvent.UwbUnlockRes(result));
                break;
            default:
                break;
        }
    }

    private int bytes_expected = 0;

    public void handleRes(byte[] data) {
        Log.d(TAG, "+handleRes:[" + mResPacketType+ "]");

        int result = 0;
        String response = new String(data, StandardCharsets.UTF_8);

        Log.d(TAG, "Receiving BLE packet: " + response);

        if (response == null){
            Log.d(TAG, "-handleRes:check packet error:" + result);
            result = INVALID_PACKET;
            notifyRes(mResPacketType, result);
            return;
        }

        String response_data = response.substring(response.indexOf("=") + 1).replace("\r\n","");

        if (response.contains(AT_PREFIX)){
            packetData = new ByteArrayOutputStream();

            if (response.contains(GET_USER_INFO_AT)) {
                // its the first getuserinfo packet, prepare all variables
                bytes_expected = USER_STRUCT_SIZE * SmartAccessActivity.userCount;

                Log.d(TAG, "bytes_expected " + bytes_expected);
            } else {
                // this resets the bytes_expected in case of hanging packets
                bytes_expected = 0;
            }

            try {
                /* save data into buffer */
                packetData.write(response_data.getBytes());
                if (response.contains(GET_USER_INFO_AT))
                    bytes_expected -= response_data.length();
            } catch (IOException e) {
                Log.e(TAG,"extra packets data write error: " + e.getMessage());
                result = INVALID_PACKET;
                notifyRes(mResPacketType, result);
                return;
            }

            if(response_data.startsWith(RESPONSE_FAIL)) {
                Log.e(TAG,"-handleRes: commands failed with: " + response_data + " response");
                result = VALID_PACKET_FAILED;
                notifyRes(mResPacketType, result);
                return;
            } else if(response_data.startsWith(RESPONSE_DUPLICATE)) {
                Log.e(TAG,"-handleRes: commands failed with: " + response_data + " response");

                String name = response_data.substring(RESPONSE_DUPLICATE.length()).replace("\r\n","");
                packetData.reset();
                packetData.write(name.getBytes(),0, name.getBytes().length);

                result = DUPLICATE_RESULT;
                notifyRes(mResPacketType, result);
                return;
            }
        } else {
            // here, check if we still have users to receive
            if (bytes_expected > 0) {
                try {
                    /* save data into buffer */
                    packetData.write(response_data.getBytes());
                    bytes_expected -= response_data.length();
                } catch (IOException e) {
                    Log.e(TAG,"extra packets data write error: " + e.getMessage());
                    result = INVALID_PACKET;
                    notifyRes(mResPacketType, result);
                }

            } else {
                Log.d(TAG, "-handleRes:packets AT commands issue: " + packetDataLength);
                result = INVALID_PACKET;
                notifyRes(mResPacketType, result);
                return;
            }
        }

        if (bytes_expected == 0) {
            notifyRes(mResPacketType, result);
            Log.d(TAG, "-handleRes:[" + packetReserved + "]");
        }
    }
}
