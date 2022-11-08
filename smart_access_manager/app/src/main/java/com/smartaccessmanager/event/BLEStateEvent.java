/**
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

/**
 * Copyright 2016 Freescale Semiconductors, Inc.
 */

package com.smartaccessmanager.event;

import static com.smartaccessmanager.activity.SmartAccessActivity.FACE;
import static com.smartaccessmanager.activity.SmartAccessActivity.FINGERPRINT;
import static com.smartaccessmanager.activity.SmartAccessActivity.NFC;
import static com.smartaccessmanager.activity.SmartAccessActivity.UWB;
import static com.smartaccessmanager.service.BLEProtocol.USER_ID_SIZE;
import static com.smartaccessmanager.service.BLEProtocol.USER_STRUCT_SIZE;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;

import androidx.annotation.NonNull;

import com.smartaccessmanager.database.User;

import java.util.ArrayList;

public class BLEStateEvent {

    /**
     * This event will be fired in global-level to inform if Bluetooth state has been changed.
     */
    public static class BluetoothStateChanged extends BaseEvent {

        public final int newState;

        public BluetoothStateChanged(int newState) {
            super();
            this.newState = newState;
        }
    }

    public static class BluetoothClientStateChanged extends BaseEvent {

        public final int newState;

        public BluetoothClientStateChanged(int newState) {
            super();
            this.newState = newState;
        }
    }
    /**
     * This event will be fired whenever a BLE device is connected.
     */
    public static class Connected extends BaseEvent {
        public int bondState;
    }

    /**
     * This event will be fired whenever phone is trying to connect to a BLE device.
     */
    public static class Connecting extends BaseEvent {
    }

    /**
     * This event will be fired whenever a current connection to BLE device has been lost.
     */
    public static class Disconnected extends BaseEvent {
    }

    /**
     * This event will be fired whenever all services of a BLE device have been discovered.
     */
    public static class ServiceDiscovered extends BaseEvent {
        public int bondState;
    }

    /**
     * This event will be fired whenever a piece of data is available through a {@link BluetoothGattCharacteristic}.
     */
    public static class DataAvailable extends BaseEvent {

        public final BluetoothGattCharacteristic characteristic;
        public boolean isNotify = false;

        public DataAvailable(@NonNull BluetoothGattCharacteristic characteristic) {
            super();
            this.characteristic = characteristic;
        }
    }

    /*
     * AUTHENTICATION_RES
     */
    public static class AuthenticationRes extends BaseEvent {
        public int mAuthenticationResult = 1;

        public AuthenticationRes(int authenticationResult) {
            super();
            this.mAuthenticationResult = authenticationResult;
        }
    }

    /*
     * UPDATE_PASSWORD_RES
     */
    public static class UpdatePasswordRes extends BaseEvent {
        public int mUpdatePasswordResult = 1;

        public UpdatePasswordRes(int updatePasswordResult) {
            super();
            this.mUpdatePasswordResult = updatePasswordResult;
        }
    }

    /*
     * SETPWD_RES
     */

    public static class SetPwdRes extends BaseEvent {
        public int mSetPwdResult = 1;
        public String mSetPwdReceivedID = null;

        public SetPwdRes(int setPwdResult, String setPwdReceivedID) {
            super();
            this.mSetPwdResult = setPwdResult;
            this.mSetPwdReceivedID = setPwdReceivedID;
        }
    }

    /*
     * ENROLL_FINGERPRINT_RES
     */
    public static class EnrollFingerprintRes extends BaseEvent {
        public int mEnrollFingerprintResult = 1;
        public int mEnrollFingerprintProgress = -1;

        public EnrollFingerprintRes(int enrollFingerprintResult, String enrollFingerprintProgress) {
            super();
            this.mEnrollFingerprintResult = enrollFingerprintResult;
            try { this.mEnrollFingerprintProgress = Integer.parseInt(enrollFingerprintProgress); }
            catch (Exception ignored) {}
        }
    }

    /*
     * RECORD_NFC_RES
     */
    public static class RecordNfcRes extends BaseEvent {
        public int mRecordNfcResult = 1;

        public RecordNfcRes(int recordNfcResult) {
            super();
            this.mRecordNfcResult = recordNfcResult;
        }
    }

    /*
     * RECORD_UWB_RES
     */
    public static class RecordUwbRes extends BaseEvent {
        public int mRecordUwbResult = 1;

        public RecordUwbRes(int recordUwbResult) {
            super();
            this.mRecordUwbResult = recordUwbResult;
        }
    }

    /*
     * REGISTRATION_RES
     */
    public static class RegistrationRes extends BaseEvent {
        public int mRegistrationResult = 1;
        public String mRegistrationDuplicateName = null;

        public RegistrationRes(int registrationResult, String registrationData) {
            super();
            this.mRegistrationResult = registrationResult;
            this.mRegistrationDuplicateName = registrationData;
        }
    }

    /*
     * DELETE_USER_RES
     */
    public static class DeleteUserRes extends BaseEvent {
        public int mDeleteUserResult = 1;

        public DeleteUserRes(int deleteUserResult) {
            super();
            this.mDeleteUserResult = deleteUserResult;
        }
    }

    /*
     * GET_USER_COUNT_RES
     */
    public static class GetUserCountRes extends BaseEvent {
        public int mGetUserCountResult = 1;
        public int mGetUserCountNumber = 0;

        public GetUserCountRes(int getUserCountResult, int getUserCountNumber) {
            super();
            this.mGetUserCountResult = getUserCountResult;
            this.mGetUserCountNumber = getUserCountNumber;
        }
    }

    /*
     * GET_USER_INFO_RES
     */
    public static class GetUserInfoRes extends BaseEvent {

        public int mGetUserInfoResult = 1;
        public int mGetUserInfoCount = 1;
        public ArrayList<User> mGetUserInfoData = null;

        private void parseUserData(int userCount, String userData){
            mGetUserInfoData = new ArrayList<>();

            System.out.println("+GetUserInfoRes " + userData);

            for (int i = 0; i < userCount; i += 1){
                String temp_user = userData.substring(USER_STRUCT_SIZE * i, USER_STRUCT_SIZE * (i+1));

                System.out.println("+GetUserInfoRes " + temp_user);

                int id = Integer.parseInt(temp_user.substring(0, USER_ID_SIZE)) ;
                String name = temp_user.substring(USER_ID_SIZE).trim();

                mGetUserInfoData.add(new User(id, name, null));
            }
        }

        public GetUserInfoRes(int getUserInfoResult, int getUserInfoCount, String getUserInfoData) {
            super();
            this.mGetUserInfoResult = getUserInfoResult;
            this.mGetUserInfoCount = getUserInfoCount;
            if (mGetUserInfoCount >= 0)
                parseUserData(getUserInfoCount, getUserInfoData);
        }
    }

    public static class ConnectionTimeout extends BaseEvent {
        public ConnectionTimeout() {
            super();
        }
    }

    /*
     * PASSWORD_UNLOCK_RES
     */
    public static class PasswordUnlockRes extends BaseEvent {
        public int mPasswordUnlockRes = 1;

        public PasswordUnlockRes(int passwordUnlockRes) {
            super();
            this.mPasswordUnlockRes = passwordUnlockRes;
        }
    }

    /*
     * NFC_UNLOCK_RES
     */
    public static class NfcUnlockRes extends BaseEvent {
        public int mNfcUnlockRes = 1;

        public NfcUnlockRes(int nfcUnlockRes) {
            super();
            this.mNfcUnlockRes = nfcUnlockRes;
        }
    }

    /*
     * UWB_UNLOCK_RES
     */
    public static class UwbUnlockRes extends BaseEvent {
        public int mUwbUnlockRes = 1;

        public UwbUnlockRes(int uwbUnlockRes) {
            super();
            this.mUwbUnlockRes = uwbUnlockRes;
        }
    }

    /*
     * UPDATE_USER_INFO_RES
     */
    public static class UpdateUserInfoRes extends BaseEvent {
        public int mUpdateUserInfoResult = 1;

        public UpdateUserInfoRes(int updateUserInfoResult) {
            super();
            this.mUpdateUserInfoResult = updateUserInfoResult;
        }
    }

    /*
     * REGISTRATION_CMD_RES
     */
    public static class RegistrationCMDRes extends BaseEvent {
        public int mRegistrationCMDResult = 1;

        public RegistrationCMDRes(int registrationCMDResult) {
            super();
            this.mRegistrationCMDResult = registrationCMDResult;
        }
    }

    /*
     * DEREGISTRATION_CMD_RES
     */
    public static class DeregistrationCMDRes extends BaseEvent {
        public int mDeregistrationCMDResult = 1;

        public DeregistrationCMDRes(int deregistrationCMDResult) {
            super();
            this.mDeregistrationCMDResult = deregistrationCMDResult;
        }
    }

    /*
     * PREV_CAMERA_SWITCH_CMD_RES
     */
    public static class PrevCameraSwitchCMDRes extends BaseEvent {
        public int mPrevCameraSwitchCMDResult = 1;

        public PrevCameraSwitchCMDRes(int prevCameraSwitchCMDResult) {
            super();
            this.mPrevCameraSwitchCMDResult = prevCameraSwitchCMDResult;
        }
    }

    /*
     * GET_APP_TYPE_RES
     */
    public static class GetAppTypeRes extends BaseEvent {
        public int mGetAppTypeResult = 1;
        public ArrayList<Integer> mAppFeatures;

        private void parseAppTypeResponse(String getAppTypeResponse) {
            mAppFeatures = new ArrayList<>();

            if (getAppTypeResponse.contains("FACE")) {
                mAppFeatures.add(FACE);
            }
            if (getAppTypeResponse.contains("UWB")) {
                mAppFeatures.add(UWB);
            }
            if (getAppTypeResponse.contains("NFC")) {
                mAppFeatures.add(NFC);
            }
            if (getAppTypeResponse.contains("FINGERPRINT")) {
                mAppFeatures.add(FINGERPRINT);
            }
        }

        public GetAppTypeRes(int getAppTypeResult, String getAppTypeResponse) {
            super();
            this.mGetAppTypeResult = getAppTypeResult;
            parseAppTypeResponse(getAppTypeResponse);
        }
    }

    /*
     * GET_ALGO_VERSION_RES
     */
    public static class GetAlgoVersionRes extends BaseEvent {
        public int mGetAlgoVersionResult = 1;

        public GetAlgoVersionRes(int getAlgoVersionResult) {
            super();
            this.mGetAlgoVersionResult = getAlgoVersionResult;
        }
    }

    /**
     * This event will be fired whenever a piece of data is available through a {@link BluetoothGattCharacteristic}.
            */
    public static class DataAvailableFRMD extends BaseEvent {

        public final BluetoothGatt characteristic;

        public DataAvailableFRMD(@NonNull BluetoothGatt characteristic) {
            super();
            this.characteristic = characteristic;
        }
    }


    public static class DataWritenFromClient extends BaseEvent {

        public final BluetoothDevice device;
        public final int requestId;
        public final BluetoothGattCharacteristic characteristic;
        public final boolean preparedWrite;
        public final boolean responseNeeded;
        public final int offset;
        public final byte[] value;

        public DataWritenFromClient(BluetoothDevice device, int requestId,
                                    BluetoothGattCharacteristic characteristic, boolean preparedWrite,
                                    boolean responseNeeded, int offset, byte[] value) {
            this.device = device;
            this.requestId = requestId;
            this.characteristic = characteristic;
            this.preparedWrite = preparedWrite;
            this.responseNeeded = responseNeeded;
            this.offset = offset;
            this.value = value;
        }
    }

    /**
     * Global-level event, fired whenever bonding state of a bluetooth device is changed.
     */
    public static class DeviceBondStateChanged extends BaseEvent {

        public final BluetoothDevice device;
        public final int bondState;

        public DeviceBondStateChanged(BluetoothDevice device, int bondState) {
            super();
            this.device = device;
            this.bondState = bondState;
        }
    }

    public static class DeviceRssiUpdated extends BaseEvent {

        public final String device;
        public final int rssi;

        public DeviceRssiUpdated(int rssi, String device) {
            super();
            this.rssi = rssi;
            this.device = device;
        }
    }

    public static class MTUUpdated extends BaseEvent {

        public final String device;
        public final int mtuSize;
        public final boolean success;

        public MTUUpdated(String device, int mtuSize, boolean success) {
            super();
            this.device = device;
            this.mtuSize = mtuSize;
            this.success = success;
        }
    }

    public static class PHYUpdated extends BaseEvent {

        public final int txPhy;
        public final int rxPhy;
        public final int status;

        public PHYUpdated(int tx, int rx, int sts) {
            super();
            this.txPhy = tx;
            this.rxPhy = rx;
            this.status = sts;
        }
    }

    public static  class PHYReaded extends BaseEvent{
        public  final  int txPhy;
        public final  int rxPhy;
        public PHYReaded(int tx,int rx){
            this.txPhy=tx;
            this.rxPhy=rx;
        }
    }
}
