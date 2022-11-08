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
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.smartaccessmanager.service;

import android.annotation.TargetApi;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.smartaccessmanager.AppConfig;
import com.smartaccessmanager.activity.NoConnectionActivity;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.model.BLEAttributes;
import com.smartaccessmanager.model.Task;
import com.smartaccessmanager.utility.BLEConverter;
import com.smartaccessmanager.utility.SdkUtils;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

public enum BLEService {

    /**
     * Please refer to http://stackoverflow.com/questions/70689.
     */
    INSTANCE;

    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_BLS";

    private BLEProtocol protocol;

    /**
     * States of BLE connection.
     */
    public interface State {

        int STATE_DISCONNECTED = 0;
        int STATE_CONNECTING = 1;
        int STATE_CONNECTED = 2;
    }

    /**
     * Request types which can be made to BLE devices.
     */
    public interface Request {

        int READ = 0;
        int WRITE = 1;
        int NOTIFY = 2;
        int INDICATE = 3;
        int WRITE_NO_RESPONSE = 4;
        int WRITE_WITH_AUTHEN = 5;
        int DISABLE_NOTIFY_INDICATE = 6;
    }

    private BluetoothManager mBluetoothManager; // centralized BluetoothManager
    private BluetoothAdapter mBluetoothAdapter; // corresponding Adapter
    private BluetoothGatt mBluetoothGatt; // GATT connection
    private BluetoothGattServer mBluetoothGattServer; // for UART, we need a server callback

    /**
     * Always use main thread to perform BLE operation to avoid issue with many Samsung devices.
     */
    private final Handler mMainLoop = new Handler(Looper.getMainLooper());

    /**
     * All awaiting operations need a timeout.
     */
    private final Runnable mTimeOutRunnable = new Runnable() {

        @Override
        public void run() {
            Log.v("ota","mTimeOutRunnable");
            continueTaskExecution(false);
        }
    };

    /**
     * Also, service discovery need a timeout too.
     */
    private final Runnable mDiscoveryTimeOut = new Runnable() {

        @Override
        public void run() {
            disconnect();
        }
    };

    /**
     * We hold reference to application-level context.
     */
    private Context mContext;

    /**
     * All registered tasks are stored and release when disconnect.
     */
    private List<Task> mTaskList;

    /**
     * Internal variable to track connection state.
     */
    private int mConnectionState;

    /**
     * Sometimes, connect to a BLE device need a few tries :(
     */
    private int tryTime;

    /**
     * Internal MAC address of BLE device.
     */
    private String address;

    /**
     * We need to create a Gatt Server with UART support (service and characteristic).
     */
    private boolean needUartSupport;


    private boolean isThroughput;

    private int mCurrentPhy;

    /**
     * This callback should be used whenever a GATT event is fired.
     * It will, in turn, broadcast EventBus events to other application components.
     */
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            Log.e(TAG, "onConnectionStateChange:device state" + gatt.toString()+newState);
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                isThroughput = false;
                mConnectionState = State.STATE_CONNECTED;
                BLEStateEvent.Connected connectedEvent = new BLEStateEvent.Connected();
                connectedEvent.bondState = gatt.getDevice().getBondState();
                EventBus.getDefault().post(new BLEStateEvent.Connected());
                // after connected, ask executors to dicover BLE services
                registerTask(new Task() {

                    @Override
                    public void run() {
                        mMainLoop.removeCallbacks(mDiscoveryTimeOut);
                        if (mBluetoothGatt != null) {
                            mBluetoothGatt.discoverServices();
                            mMainLoop.postDelayed(mDiscoveryTimeOut, AppConfig.DEFAULT_REQUEST_TIMEOUT);
                        }
                    }
                });
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                mServiceDiscovered = null;
                if (status == 133 && !TextUtils.isEmpty(address) && tryTime < 10) {
                    mMainLoop.postDelayed(new Runnable() {

                        @Override
                        public void run() {
                            connect(address, needUartSupport,mCurrentPhy);
                        }
                    }, 1000);
                } else {
                    disconnect();
                    bluetoothReleaseResource();
                    EventBus.getDefault().post(new BLEStateEvent.Disconnected());
                }
            }
        }

        BLEStateEvent.ServiceDiscovered mServiceDiscovered;
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "onServicesDiscovered:dicovery all service" + "SUCCESS");

                if (BLEService.this.needUartSupport == false) {
                    request(BLEAttributes.WUART, BLEAttributes.UART_NOTIFY, Request.NOTIFY);
                }

                mMainLoop.removeCallbacks(mDiscoveryTimeOut);
                if (mServiceDiscovered ==null){
                    mServiceDiscovered = new BLEStateEvent.ServiceDiscovered();
                    mServiceDiscovered.bondState = gatt.getDevice().getBondState();
                    EventBus.getDefault().post(mServiceDiscovered);
                }
                // post to get all FRDM's services
                //EventBus.getDefault().post(gatt.getServices());
                EventBus.getDefault().post(new BLEStateEvent.DataAvailableFRMD(gatt));
                continueTaskExecution(false);
            } else if (status == BluetoothGatt.GATT_INSUFFICIENT_AUTHENTICATION || status == BluetoothGatt.GATT_INSUFFICIENT_ENCRYPTION) {
                changeDevicePairing(gatt.getDevice(), true);
                Log.e(TAG, "onServicesDiscovered:dicovery all service" +"discovery failed and remove device");

            } else {
                Log.e(TAG, ":dicovery all service" +"discovery failed and remove device disconnect");

                disconnect();
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            Log.e(TAG, "onCharacteristicRead");
            if (status == BluetoothGatt.GATT_SUCCESS) {
                EventBus.getDefault().post(new BLEStateEvent.DataAvailable(characteristic));
                continueTaskExecution(false);
            } else if (status == BluetoothGatt.GATT_INSUFFICIENT_AUTHENTICATION || status == BluetoothGatt.GATT_INSUFFICIENT_ENCRYPTION) {
                changeDevicePairing(gatt.getDevice(), true);
            } else {
                disconnect();
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            Log.d(TAG, "+onCharacteristicChanged");
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            Log.d(TAG, "onCharacteristicWrite");
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (isThroughput) {
                    Log.d(TAG, "  writeCharacteristic");
                    gatt.writeCharacteristic(characteristic);
                } else {
                    BLEStateEvent.DataAvailable e =new BLEStateEvent.DataAvailable(characteristic);
                    EventBus.getDefault().post(e);
                    continueTaskExecution(false);
                }

            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            Log.d(TAG, "onDescriptorWrite");
            if (status == BluetoothGatt.GATT_SUCCESS) {
                continueTaskExecution(false);
            } else if (status == BluetoothGatt.GATT_INSUFFICIENT_AUTHENTICATION || status == BluetoothGatt.GATT_INSUFFICIENT_ENCRYPTION) {
                changeDevicePairing(gatt.getDevice(), true);
            } else {
                disconnect();
            }
        }

        @Override
        public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                EventBus.getDefault().post(new BLEStateEvent.DeviceRssiUpdated(rssi, address));
            }
        }

        @Override
        public void onMtuChanged(BluetoothGatt gatt, int mtuSize, int status) {
//            super.onMtuChanged(gatt, mtuSize, status);
            Log.e(TAG, "onMtuChanged : \nmtuSize : " + mtuSize + "\nstatus : " + status);
            EventBus.getDefault().post(new BLEStateEvent.MTUUpdated(address, mtuSize, (status == BluetoothGatt.GATT_SUCCESS)));
        }

        @TargetApi(Build.VERSION_CODES.O)
        @Override
        public void onPhyUpdate(BluetoothGatt gatt, int txPhy, int rxPhy, int status) {
            EventBus.getDefault().post(new BLEStateEvent.PHYUpdated(txPhy,rxPhy,status));
        }

        @TargetApi(Build.VERSION_CODES.O)
        @Override
        public void onPhyRead(BluetoothGatt gatt, int txPhy, int rxPhy, int status) {
            EventBus.getDefault().post(new BLEStateEvent.PHYReaded(txPhy,rxPhy));
        }

    };





    /**
     * This callback is used for Wireless UART custom profile.
     */
    private final BluetoothGattServerCallback mGattServerCallback = new BluetoothGattServerCallback() {

        @Override
        public void onConnectionStateChange(BluetoothDevice device, int status, int newState) {
            Log.d(TAG, "gatt server" + "gatt server state changed: "+status+" ==" + newState);
            EventBus.getDefault().post(new BLEStateEvent.BluetoothClientStateChanged(newState));
        }

        @Override
        public void onServiceAdded(int status, BluetoothGattService service) {
            Log.d(TAG, "gatt server" + "gatt server added: "+status);

        }

        @Override
        public void onCharacteristicWriteRequest(BluetoothDevice device, int requestId,
                                                 BluetoothGattCharacteristic characteristic,
                                                 boolean preparedWrite,
                                                 boolean responseNeeded,
                                                 int offset,
                                                 byte[] value) {

            Log.d(TAG, "gatt server" + "onCharacteristicWriteRequest");
            Log.d(TAG, "gatt_server: " + new String(value));

            final String charaterUuid = characteristic.getUuid().toString();
            Log.d(TAG, "  uuid = " + charaterUuid);

            if (BLEAttributes.CHAR_WUART_NOTIFY.equalsIgnoreCase(charaterUuid)) {
                Log.d(TAG, "  CHAR_WUART_NOTIFY uuid = " + charaterUuid);
                final byte[] data = value;
                if (null != data && 0 < data.length) {

                    final StringBuilder stringBuilder = new StringBuilder(data.length);
                    for (byte byteChar : data)
                        stringBuilder.append(String.format("%02x ", byteChar));
                    Log.d(TAG, "  Notify data len:" + data.length + ":[" + stringBuilder.toString() + "]");
                }else{
                    Log.d(TAG, "  Notify empty packet");
                }

                BLEService.INSTANCE.getProtocol().handleRes(data);
            }
        }
    };

    /**
     * Init singleton instance using application-level context.
     * We ensure that BluetoothManager and Adapter are always available.
     *
     * @param context
     */
    public void init(@NonNull Context context) {
        if (this.mContext != null) {
            return;
        }
        this.mContext = context.getApplicationContext();
        if (mBluetoothManager == null) {
            mBluetoothManager = (BluetoothManager) mContext.getSystemService(Context.BLUETOOTH_SERVICE);
        }

        if (mBluetoothAdapter == null) {
            mBluetoothAdapter = mBluetoothManager.getAdapter();
        }

//        protocol = new BLEVisionProtocol();
        protocol = new BLESmartAccessProtocol();

        EventBus.getDefault().register(this);

    }

    /**
     * This instance handles bonding state event to correctly execute last failed task.
     *
     * @param e
     */
    @Subscribe
    public void onEvent(BLEStateEvent.DeviceBondStateChanged e) {
        Log.e("Bond", e.device.getAddress() + " " + e.bondState);
        if (mBluetoothGatt != null && e.device.equals(mBluetoothGatt.getDevice())) {
            // if device has bonding removed, retry last task so it can request bonding again
            if (e.bondState == BluetoothDevice.BOND_NONE) {
                continueTaskExecution(true);
            } else if (e.bondState == BluetoothDevice.BOND_BONDING) {
                mMainLoop.removeCallbacks(mTimeOutRunnable);
            } else if (e.bondState == BluetoothDevice.BOND_BONDED) {
                // do nothing because request will return result to callback now
                continueTaskExecution(true);
            }
        }
    }

    /**
     * For display purpose only.
     *
     * @return
     */
    public int getConnectionState() {
        return mConnectionState;
    }

    /**
     * Check if Bluetooth of device is enabled.
     *
     * @return
     */
    public boolean isBluetoothAvailable() {
        return mBluetoothAdapter != null && mBluetoothAdapter.isEnabled();
    }

    BluetoothGattService mPhoneSideService = null;
    /**
     * Connects to the GATT server hosted on the Bluetooth LE device.
     *
     * @param address The device address of the destination device.
     * @return Return true DataWritenFromClientif the connection is initiated successfully. The connection result
     * is reported asynchronously through the
     * {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     * callback.
     */
    public void connect(@NonNull final String address, final boolean needUartSupport,final int phy) {
        mCurrentPhy = phy;
        this.address = address;
        this.tryTime++;
        mMainLoop.post(new Runnable() {

            @Override
            public void run() {
                BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
                Log.e(TAG, "initialize: mBluetoothGatt isn't null");

                if(SdkUtils.hasO()){
                    if(mBluetoothGatt !=  null){
                        mBluetoothGatt.connect();
                    }else{
                        mBluetoothGatt = device.connectGatt(mContext, false, mGattCallback,BluetoothDevice.TRANSPORT_LE,phy);
                    }
                }else{
                    if(mBluetoothGatt !=  null){
                        mBluetoothGatt.connect();
                    }else{
                        mBluetoothGatt = device.connectGatt(mContext, false, mGattCallback);
                    }
                }
                Log.e(TAG, "initialize successfully: mBluetoothGatt isn't null");

                mConnectionState = State.STATE_CONNECTING;
                EventBus.getDefault().post(new BLEStateEvent.Connecting());
                BLEService.this.needUartSupport = needUartSupport;

                if (needUartSupport) {
                    Log.e(TAG, "needUartSupport");

                    mBluetoothGattServer = mBluetoothManager.openGattServer(mContext, mGattServerCallback);
                    final BluetoothGattCharacteristic characteristic = new BluetoothGattCharacteristic(
                            BLEConverter.uuidFromAssignedNumber(BLEAttributes.UART_STREAM),
                            BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE,
                            BluetoothGattCharacteristic.PERMISSION_WRITE);

                    mPhoneSideService = new BluetoothGattService(
                            BLEConverter.uuidFromAssignedNumber(BLEAttributes.WUART), BluetoothGattService.SERVICE_TYPE_PRIMARY);

                    mPhoneSideService.addCharacteristic(characteristic);
                    if(null != mBluetoothGattServer && null != mPhoneSideService){
                        mBluetoothGattServer.addService(mPhoneSideService);
                    }

                }
            }
        });
    }

    /**
     * Checks if bluetooth is manually turned OFF in order to go back to scan activity
     */
    public final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            System.out.println(action);
            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                if (mBluetoothAdapter.getState() == BluetoothAdapter.STATE_TURNING_OFF ||
                        mBluetoothAdapter.getState() == BluetoothAdapter.STATE_OFF) {
                    NoConnectionActivity.jumpToDisconnectActivity((Activity) context);
                }
            }
        }
    };

    /**
     * Disconnects an existing connection or cancel a pending connection. The disconnection result
     * is reported asynchronously through the
     * {@code BluetoothGattCallback#onConnectionStateChange(android.bluetooth.BluetoothGatt, int, int)}
     * callback.
     */
    public void disconnect() {

        if (mBluetoothGatt != null) {
            try {

                mBluetoothGatt.disconnect();
//                        mBluetoothGatt.close();
            } catch (Throwable ignored) {
            }
            mBluetoothGatt = null;
        }
        if (mBluetoothGattServer != null) {
            try {
                mBluetoothGattServer.close();
            } catch (Throwable ignored) {
            }
            mBluetoothGattServer = null;
        }

        mConnectionState = State.STATE_DISCONNECTED;
        tryTime = 0;
    }
private void bluetoothReleaseResource(){
    try {
        clearTasks();
        mBluetoothGatt.close();
    } catch (Throwable ignored) {
    }
    mBluetoothGatt = null;
    Log.e(TAG, "disconnect:BluetoothGatt is null");

    if (mBluetoothGattServer != null) {
        try {
            if(needUartSupport){
                mBluetoothGattServer.removeService(mPhoneSideService);
            }
            mBluetoothGattServer.close();
        } catch (Throwable ignored) {
        }
        mBluetoothGattServer = null;
    }
}
    /**
     * Check and get if a service with UUID is available within device.
     *
     * @param uuid assigned-number of a service
     * @return available service
     */
    public BluetoothGattService getService(int uuid) {
        return mBluetoothGatt == null ? null : mBluetoothGatt.getService(BLEConverter.uuidFromAssignedNumber(uuid));
    }


    public void readCustomCharacteristic() {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {

            return;
        }
        /*check if the service is available on the device*/
        BluetoothGattService mCustomService = mBluetoothGatt.getService(UUID.fromString("02ff5600-ba5e-f4ee-5ca1-eb1e5e4b1ce0"));
        if(mCustomService == null){

            return;
        }
        /*get the read characteristic from the service*/
        BluetoothGattCharacteristic mReadCharacteristic = mCustomService.getCharacteristic(UUID.fromString("02ff5700-ba5e-f4ee-5ca1-eb1e5e4b1ce0"));
        if(mBluetoothGatt.readCharacteristic(mReadCharacteristic) == false){

        }
    }




    public boolean request(String serviceUUID, int characteristicUUID, final int requestType) {
        if (mBluetoothGatt == null) {
            return false;
        }

        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_READ) > 0) {
            if (requestType == Request.READ) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        readCharacteristic(characteristic);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_INDICATE) > 0) {
            if (requestType == Request.INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicIndication(characteristic);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.NOTIFY) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.DISABLE_NOTIFY_INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicDisableNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        return true;
    }


    public boolean request(String serviceUUID, String characteristicUUID, final int requestType) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
       final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_READ) > 0) {
            if (requestType == Request.READ) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        readCharacteristic(characteristic);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_INDICATE) > 0) {
            if (requestType == Request.INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicIndication(characteristic);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.NOTIFY) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.DISABLE_NOTIFY_INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicDisableNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        return true;
    }


    /**
     * Request READ action using predefined service UUID and characteristic UUID, can allow notification if needed.
     *
     * @param serviceUUID
     * @param characteristicUUID
     * @param requestType        value must be of {@link com.freescale.bletoolbox.service.BLEService.Request}
     * @return true if both service and characteristic are found, false if otherwise.
     */
    public boolean request(int serviceUUID, int characteristicUUID, final int requestType) {
        Log.d(TAG, "+request:" + String.format("0x%x ", serviceUUID) + ":" + String.format("0x%x ", characteristicUUID) + ":" + requestType);
        if (mBluetoothGatt == null) {
            Log.d(TAG, "-request: mBluetoothGatt is null");
            return false;
        }
        BluetoothGattService service = getService(serviceUUID);
        if (service == null) {
            Log.d(TAG, "-request: service is null");
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUUID));
        if (characteristic == null) {
            Log.d(TAG, "-request: characteristic is null");
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_READ) > 0) {
            Log.d(TAG, "  PROPERTY_READ");
            if (requestType == Request.READ) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        readCharacteristic(characteristic);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_INDICATE) > 0) {
            Log.d(TAG, "  PROPERTY_INDICATE");
            if (requestType == Request.INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicIndication(characteristic);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            Log.d(TAG, "  PROPERTY_NOTIFY");
            if (requestType == Request.NOTIFY) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
            if (requestType == Request.DISABLE_NOTIFY_INDICATE) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        boolean waitForDescriptorWrite = setCharacteristicDisableNotification(characteristic, true);
                        if (!waitForDescriptorWrite) {
                            continueTaskExecution(false);
                        }
                    }
                });
            }
        }
        Log.d(TAG, "-request");
        return true;
    }



    /**
     * Specific request method to be used with Wireless UART.
     *
     * @param serviceUuid
     * @param characteristicUuid
     * @param data
     * @return
     */
    public boolean requestWrite(int serviceUuid, int characteristicUuid, final byte[] data) {
        Log.d(TAG, "+requestWrite");
        final boolean[] isSend = new boolean[1];
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = getService(serviceUuid);
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUuid));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) > 0) {
            registerTask(new Task() {

                @Override
                public void run() {
                    if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                        Log.d(TAG, "  invalid state");
                        return;
                    }
                    characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                    characteristic.setValue(data);
                    isSend[0] = mBluetoothGatt.writeCharacteristic(characteristic);
                    Log.d(TAG, "  writeCharacteristic:" + isSend[0]);
                }
            });
        }
        Log.d(TAG, "+requestWrite:" + isSend[0]);
        return true; //isSend[0];
    }


    public boolean requestWrite(String serviceUuid, String characteristicUuid, final byte[] data) {
        if (mBluetoothGatt == null) {
            Log.e("command send","step 1");

            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUuid));
        //BluetoothGattService service = getService(serviceUuid);
        if (service == null) {
            Log.e("command send","step 2");

            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUuid));
       /* final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUuid));*/
        if (characteristic == null) {
            Log.e("command send","step 3");

            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) > 0) {
            registerTask(new Task() {

                @Override
                public void run() {
                    if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                        return;
                    }
                    characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                    characteristic.setValue(data);
                    mBluetoothGatt.writeCharacteristic(characteristic);
                }
            });
        }
        return true;
    }

    /**
     * Request current RSSI value of connected BLE device.
     *
     * @return
     */
    public boolean requestRemoteRssi() {
        return mBluetoothGatt != null && mBluetoothGatt.readRemoteRssi();
    }

    /**
     * Add a task to single executor. This taks should be timed out after a few second.
     *
     * @param task
     */
    private void registerTask(Task task) {
        if (mTaskList == null) {
            mTaskList = new ArrayList<>();
        }
        mTaskList.add(task);
        Log.e("Task", "Registered with id " + task.id);
        if (mTaskList.size() == 1) {
            continueTaskExecution(true);
        }
    }

    /**
     * Unblock current execution, retry last task if needed or skip to next task.
     *
     * @param retryLastTask
     */
    private void continueTaskExecution(final boolean retryLastTask) {
        mMainLoop.removeCallbacks(mTimeOutRunnable);
        mMainLoop.post(new Runnable() {

            @Override
            public void run() {
                if (mTaskList == null || mTaskList.isEmpty()) {
                    return;
                }
                if (!retryLastTask) {
                    mTaskList.remove(0);
                }
                if (mTaskList.isEmpty()) {
                    return;
                }
                final Task task = mTaskList.get(0);
                if (task != null) {
                    Log.e("Task", "Schedule task " + task.id);
                    task.run();
                }
            }
        });
        mMainLoop.postDelayed(mTimeOutRunnable, AppConfig.DEFAULT_REQUEST_TIMEOUT);
    }

    /**
     * Clear all task variables.
     */
    private void clearTasks() {
        if (mTaskList != null) {
            mTaskList.clear();
            mTaskList = null;
        }
        Task.sInternalId = 0;
        mMainLoop.removeCallbacks(mTimeOutRunnable);
    }

    /**
     * Request a read on a given {@code BluetoothGattCharacteristic}. The read result is reported
     * asynchronously through the {@code BluetoothGattCallback#onCharacteristicRead(android.bluetooth.BluetoothGatt, android.bluetooth.BluetoothGattCharacteristic, int)}
     * callback. Always need to wait for callback.
     *
     * @param characteristic The characteristic to read from.
     */
    private void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return;
        }
        mBluetoothGatt.readCharacteristic(characteristic);
    }

    /**
     * Enables or disables notification on a give characteristic.
     *
     * @param characteristic Characteristic to act on.
     * @param enabled        If true, enable notification.  False otherwise.
     * @return false if no need to perform any action, true if we need to wait from callback
     */
    private boolean setCharacteristicNotification(BluetoothGattCharacteristic characteristic, boolean enabled) {

        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return false;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, enabled);
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                BLEConverter.uuidFromAssignedNumber(BLEAttributes.CLIENT_CHARACTERISTIC_CONFIG));
        if (descriptor != null) {
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            boolean writeS = mBluetoothGatt.writeDescriptor(descriptor);

            return true;
        }
        return false;
    }

    /**
     * send Disable Notification to the remove device
     * Enables or disables notification on a give characteristic.
     *
     * @param characteristic Characteristic to act on.
     * @param enabled        If true, enable notification.  False otherwise.
     * @return false if no need to perform any action, true if we need to wait from callback
     */
    private boolean setCharacteristicDisableNotification(BluetoothGattCharacteristic characteristic, boolean enabled) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return false;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, enabled);
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                BLEConverter.uuidFromAssignedNumber(BLEAttributes.CLIENT_CHARACTERISTIC_CONFIG));
        if (descriptor != null) {
            descriptor.setValue(BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE);
            mBluetoothGatt.writeDescriptor(descriptor);
            return true;
        }
        return false;
    }

    /**
     * @param serviceUUID
     * @param characteristicUUID
     * @param data
     */
    public boolean writeDataWithAuthen(int serviceUUID, int characteristicUUID, final byte[] data) {
        //remove write type with authentication ( because this type only work on Sony devices)
        return writeData(serviceUUID, characteristicUUID, Request.WRITE, data);
    }


    public boolean writeData(String serviceUUID, int characteristicUUID, final int writeType, final byte[] data) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
               BLEConverter.uuidFromAssignedNumber(characteristicUUID));
      //  final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if (Request.WRITE_WITH_AUTHEN == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_SIGNED) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_SIGNED);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE_NO_RESPONSE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        }
        return true;
    }


    public boolean writeData(String serviceUUID, String characteristicUUID, final int writeType, final byte[] data) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if (Request.WRITE_WITH_AUTHEN == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_SIGNED) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_SIGNED);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE_NO_RESPONSE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        }
        return true;
    }


    public boolean throughputWriteData(String serviceUUID, String characteristicUUID, byte[] value, boolean repeate){
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }

        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        isThroughput = repeate;

        characteristic.setValue(value);
        return mBluetoothGatt.writeCharacteristic(characteristic);
    }


    public boolean requestMTU(final int mtuSize) {
        if (Build.VERSION.SDK_INT >= 21 && mBluetoothGatt != null) {
            registerTask(new Task() {
                @Override
                public void run() {
                    mBluetoothGatt.requestMtu(mtuSize);
                }
            });
            return true;
        } else {
            return false;
        }
    }


    public boolean requestMTUInTaskWay(final int mtuSize) {
        if (Build.VERSION.SDK_INT >= 21 && mBluetoothGatt != null) {
            registerTask(new Task() {
                @Override
                public void run() {
                    mBluetoothGatt.requestMtu(mtuSize);
                }
            });
            return true;
        } else {
            return false;
        }
    }

    public boolean requestConnectPrioity(int priority) {
        if (Build.VERSION.SDK_INT >= 21) {
            return mBluetoothGatt.requestConnectionPriority(priority);
        } else {
            return false;
        }

    }

    /**
     * @param serviceUUID
     * @param characteristicUUID
     * @param writeType
     * @param data
     */
    public boolean writeData(int serviceUUID, int characteristicUUID, final int writeType, final byte[] data) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = getService(serviceUUID);
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
//        Log.d("OTA========", "send :" + BLEConverter.bytesToHex(data));

        int charaProp = characteristic.getProperties();
        if (Request.WRITE_WITH_AUTHEN == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_SIGNED) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_SIGNED);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                    }
                });
            }
        } else if (Request.WRITE_NO_RESPONSE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);

                    }
                });
            }
        } else if (Request.WRITE == writeType) {
            if ((charaProp | BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT) > 0) {
                registerTask(new Task() {

                    @Override
                    public void run() {
                        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
                            return;
                        }
                        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
                        characteristic.setValue(data);
                        mBluetoothGatt.writeCharacteristic(characteristic);
                        Log.d("ota", "SEND NewImageInfoResponse " + BLEConverter.bytesToHexWithSpace(data));

                    }
                });
            }
        }
        return true;
    }



    public boolean writeCharacteristic(String serviceUUID, String characteristicUUID, final int requestType, final int value, final int format, final int offset) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = mBluetoothGatt.getService(UUID.fromString(serviceUUID));
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) > 0) {
            if (requestType == Request.WRITE_NO_RESPONSE) {
                registerTask(new Task() {
                    @Override
                    public void run() {
                        boolean wrote = setCharacteristic(characteristic, value, format, offset);
                        continueTaskExecution(false);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE) > 0) {
            if (requestType == Request.WRITE) {
                registerTask(new Task() {
                    @Override
                    public void run() {
                        boolean wrote = setCharacteristic(characteristic, value, format, offset);
                    }
                });
            }
        }
        return true;
    }



    public boolean writeCharacteristic(int serviceUUID, int characteristicUUID, final int requestType, final int value, final int format, final int offset) {
        if (mBluetoothGatt == null) {
            return false;
        }
        BluetoothGattService service = getService(serviceUUID);
        if (service == null) {
            return false;
        }
        final BluetoothGattCharacteristic characteristic = service.getCharacteristic(
                BLEConverter.uuidFromAssignedNumber(characteristicUUID));
        if (characteristic == null) {
            return false;
        }
        int charaProp = characteristic.getProperties();
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) > 0) {
            if (requestType == Request.WRITE_NO_RESPONSE) {
                registerTask(new Task() {
                    @Override
                    public void run() {
                        boolean wrote = setCharacteristic(characteristic, value, format, offset);
                        continueTaskExecution(false);
                    }
                });
            }
        }
        if ((charaProp | BluetoothGattCharacteristic.PROPERTY_WRITE) > 0) {
            if (requestType == Request.WRITE) {
                registerTask(new Task() {
                    @Override
                    public void run() {
                        boolean wrote = setCharacteristic(characteristic, value, format, offset);
                    }
                });
            }
        }
        return true;
    }


    /**
     * @param characteristic
     * @param value
     * @param format
     * @param offset
     * @return false if no need to perform any action, true if we need to wait from callback
     */
    private boolean setCharacteristic(BluetoothGattCharacteristic characteristic, int value, int format, int offset) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return false;
        }
        characteristic.setValue(value, format, offset);
        return mBluetoothGatt.writeCharacteristic(characteristic);
    }

    /**
     * Enables or disables indication on a give characteris
     *tic.
     *
     * @param characteristic Characteristic to act on.
     * @return false if no need to perform any action, true if we need to wait from callback
     */
    private boolean setCharacteristicIndication(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null || characteristic == null) {
            return false;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, true);
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                BLEConverter.uuidFromAssignedNumber(BLEAttributes.CLIENT_CHARACTERISTIC_CONFIG));
        if (descriptor != null) {
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);
            mBluetoothGatt.writeDescriptor(descriptor);
            return true;
        }
        return false;
    }

    private boolean changeDevicePairing(@NonNull BluetoothDevice device, boolean remove) {
        try {
            Method m = device.getClass()
                    .getMethod(remove ? "removeBond" : "createBond", (Class[]) null);
            m.invoke(device, (Object[]) null);
            return true;
        } catch (Exception ignored) {
            return false;
        }
    }

    public void updatePreferredPhy(int txPhy,int rxPhy,int phyOptions){
        if(SdkUtils.hasO()){
        mBluetoothGatt.setPreferredPhy(txPhy,rxPhy,phyOptions);
    }
}

    public void readPhy(){
        if(SdkUtils.hasO()){
            mBluetoothGatt.readPhy();
        }
    }

    public BLEProtocol getProtocol() {
        return protocol;
    }
}
