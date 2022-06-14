/**
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

package com.smartaccessmanager.activity;

import android.Manifest;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.widget.Toolbar;
import androidx.cardview.widget.CardView;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentContainerView;
import androidx.viewpager2.widget.ViewPager2;

import com.google.android.material.floatingactionbutton.ExtendedFloatingActionButton;
import com.google.android.material.tabs.TabLayout;
import com.google.android.material.tabs.TabLayoutMediator;
import com.smartaccessmanager.R;
import com.smartaccessmanager.database.User;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.service.BLEService;
import com.smartaccessmanager.utility.Algorithm;
import com.smartaccessmanager.utility.ForegroundObserver;
import com.smartaccessmanager.utility.SdkUtils;
import com.smartaccessmanager.utility.StatusPopUp;
import com.smartaccessmanager.utility.UserInteractionTimer;
import com.smartaccessmanager.view.AboutDialog;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Objects;

import butterknife.OnClick;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;
import static androidx.viewpager2.widget.ViewPager2.ORIENTATION_HORIZONTAL;
import static com.smartaccessmanager.utility.SdkUtils.fullScreen;

public class SmartAccessActivity extends BaseServiceActivity {
    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_SLA";

    private final int REQUEST_PERMISSION_CODE = 2710;
    public final int REQUEST_CODE_REGISTRATION = 100;
    public final int REQUEST_CODE_CHANGING_PASSWORD = 200;
    public final int NO_REQUEST_CODE = 0;

    public static final int MTU_WUART = 247;
    private int currentMTU = 0;

    public static final int SET_PWD_RESULT_DUPLICATE = 1;
    public static final int SET_PWD_RESULT_INVALID = 2;

    public static final int NFC_RESULT_DUPLICATE = 1;
    public static final int NFC_RESULT_INVALID = 2;

    public static final int UWB_RESULT_DUPLICATE = 1;
    public static final int UWB_RESULT_INVALID = 2;

    public static final int FINGERPRINT_RESULT_DUPLICATE = 1;
    public static final int FINGERPRINT_RESULT_INVALID = 2;

    public static final int REGISTRATION_CMD_RESULT_DUPLICATE = 1;
    public static final int REGISTRATION_CMD_RESULT_INVALID = 2;

    public static final int REGISTRATION_RESULT_DUPLICATE = 1;
    public static final int REGISTRATION_RESULT_INVALID = 2;

    public static final int INVALID_PACKET = -2;
    public static final int GENERAL_ERROR = -3;

    public boolean isSyncing = false;
    public boolean userChange = false;

    public static boolean FACE_SUPPORT = false;
    public static boolean FINGERPRINT_SUPPORT = false;
    public static boolean NFC_SUPPORT = false;
    public static boolean UWB_SUPPORT = false;

    public static final int PASSWORD = 0;
    public static final int FACE = 1;
    public static final int FINGERPRINT = 2;
    public static final int NFC = 3;
    public static final int UWB = 4;

    public static int userCount = 0;
    private boolean bluetoothInit = false;

    @Override
    public void onEventMainThread(BLEStateEvent.Connecting e) {
        super.onEventMainThread(e);
        StatusPopUp.getStatusPopUpInstance().showProgress(
                this, findViewById(R.id.smartaccess_view), getString(R.string.state_connecting));
    }

    @Override
    public void onEventMainThread(BLEStateEvent.Connected e) {
        super.onEventMainThread(e);
//        StatusPopUp.getStatusPopUpInstance().showProgress(
//                this, findViewById(R.id.smartaccess_view), getString(R.string.state_get_algo_version));

        StatusPopUp.getStatusPopUpInstance().showProgress(
                this, this.findViewById(R.id.smartaccess_view), getString(R.string.state_get_app_type));

        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (!bluetoothInit){
                    BLEService.INSTANCE.getProtocol().sendGetAppTypeReq();
                    handler.postDelayed(this, 2000);
                }
            }
        }, 2000);
    }

    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.GetAppTypeRes e) {
        if (e == null) return;

        if (e.mAppFeatures.contains(FACE))
            FACE_SUPPORT = true;
        if (e.mAppFeatures.contains(NFC))
            NFC_SUPPORT = true;
        if (e.mAppFeatures.contains(UWB))
            UWB_SUPPORT = true;
        if (e.mAppFeatures.contains(FINGERPRINT))
            FINGERPRINT_SUPPORT = true;

        bluetoothInit = true;

        // startup sync
        onPressFABCardView(findViewById(R.id.smartaccess_view));

        StatusPopUp.getStatusPopUpInstance().dismiss(SmartAccessActivity.this);
    }

    @Override
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        super.onEventMainThread(e);
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }

    @Override
    @Subscribe(threadMode =ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.DataAvailable e) {
        super.onEventMainThread(e);
        if (e == null) return;

        // request mtu start
        if (currentMTU == 0) {
            BLEService.INSTANCE.requestMTU(MTU_WUART);
            currentMTU = MTU_WUART;
        }
        // request mtu end
    }

    @Subscribe
    public void onEvent(BLEStateEvent.MTUUpdated mtuUpdated) {
        Log.d(TAG, "mtuUpdated = " + mtuUpdated.mtuSize + " success " + mtuUpdated.success);
        // request mtu start
        if (currentMTU != mtuUpdated.mtuSize) {
            BLEService.INSTANCE.requestMTU(mtuUpdated.mtuSize);
            currentMTU = mtuUpdated.mtuSize;
        }
        // request mtu end

//        BLEService.INSTANCE.getProtocol().sendGetAlgoVersionReq();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        setContentView(R.layout.activity_smart_access);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        Objects.requireNonNull(getSupportActionBar()).setDisplayShowTitleEnabled(false);

        String activity_title = getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_NAME)
                + " " + getString(R.string.upper_board_suffix);

        ((TextView) findViewById(R.id.toolbar_title))
                .setText(activity_title);
        ((TextView) findViewById(R.id.toolbar_subtitle)).setText("");

        CardView sync_fab = findViewById(R.id.sync_fab_cardview);
        ExtendedFloatingActionButton add_user_fab = findViewById(R.id.add_user_fab);

        add_user_fab.animate().scaleX(1.0f).scaleY(1.0f).setDuration(300).start();
        sync_fab.animate().scaleX(1.0f).scaleY(1.0f).setDuration(300).start();
        add_user_fab.setVisibility(View.VISIBLE);
        sync_fab.setVisibility(View.VISIBLE);

        this.getSupportFragmentManager().beginTransaction()
                .setReorderingAllowed(true)
                .replace(R.id.my_view_main, new UserManagementFragment(), null)
                .commit();

        SdkUtils.changeToolbarFABButtonState(this, R.string.menu_scan_stop, R.string.sync, isSyncing);

        BLEService.INSTANCE.init(getApplicationContext());
        if (TextUtils.isEmpty(mDeviceAddress)) {
            //throw new NullPointerException("Invalid Bluetooth MAC Address");
            toggleState(false);
        } else {
            toggleState(true);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (bluetoothInit) {
            // simulate pressing the sync button everytime we resume this activity
            onPressFABCardView(findViewById(R.id.smartaccess_view));
        }
        ForegroundObserver.getForegroundObserver().updateActivity(this);
        UserInteractionTimer.getTimerInstance().startTimer(this);
    }

    @Override
    public void onUserInteraction() {
        super.onUserInteraction();
        UserInteractionTimer.getTimerInstance().resetTimer();
    }

    @Override
    protected void onPause() {
        super.onPause();
        EventBus.getDefault().unregister(this);
        UserInteractionTimer.getTimerInstance().stopTimer();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        Fragment user_mgmt_fragment = getSupportFragmentManager().findFragmentByTag("f1");
        if (user_mgmt_fragment != null) {
            user_mgmt_fragment.onActivityResult(requestCode, resultCode, data);
        }

        switch (requestCode) {
            case REQUEST_CODE_REGISTRATION:
                if (resultCode == RESULT_OK) {
                    StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
                            this, findViewById(R.id.smartaccess_view), getString(R.string.success_registration));
                } else if (resultCode == RESULT_CANCELED) {
//                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
//                            this, findViewById(R.id.smartaccess_view), getString(R.string.success_registration_cancelled));
                } else if (resultCode == REGISTRATION_RESULT_INVALID) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.smartaccess_view), getString(R.string.failed_registration_cancelled));
                } else if (resultCode == REGISTRATION_RESULT_DUPLICATE) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.smartaccess_view), getString(R.string.error_duplicate));
                } else if (resultCode == INVALID_PACKET) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.smartaccess_view), getString(R.string.error_invalid_packet));
                } else {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.smartaccess_view), getString(R.string.error_general));
                }
                break;
            case REQUEST_CODE_CHANGING_PASSWORD:
                if (resultCode == RESULT_OK) {
                    StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
                            this, findViewById(R.id.smartaccess_view), getString(R.string.success_password));
                }
                break;
        }
    }

    @OnClick(R.id.status_view_info)
    public void viewAboutInfo() {
        AboutDialog.newInstance(this).show();
    }

    private void setResultToSkipAuthActivity() {
        Intent intent = new Intent();
        if (getParent() == null) {
            setResult(AuthenticationActivity.RESULT_OK, intent);
        } else {
            getParent().setResult(AuthenticationActivity.RESULT_OK, intent);
        }
    }

    @Override
    public void onBackPressed() {
        releaseConnection();
        finish();
    }

    public void onBackFABPressed(View view) {
        releaseConnection();
        finish();
    }

    public void onPressFABCardView(View view) {
        Log.d(TAG, "+onClickOk");

        if(isSyncing){
            isSyncing = false;
            SdkUtils.changeToolbarFABButtonState(this, R.string.menu_scan_stop, R.string.sync, isSyncing);
        }
        else {
            isSyncing = true;
            BLEService.INSTANCE.getProtocol().sendGetUserCountReq();
            SdkUtils.changeToolbarFABButtonState(this, R.string.menu_scan_stop, R.string.sync, isSyncing);
        }
    }


    public void onClickAddUser(View v) {
        // if OS >= 6 -> need ask permission access device's location
        if (SdkUtils.hasMarshmallow()) {
            if (PERMISSION_GRANTED != checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION)) {
                // handle "Never ask again"
                if (!shouldShowRequestPermissionRationale(Manifest.permission.ACCESS_FINE_LOCATION)) {
                    showMessageOKCancel(getString(R.string.grant_permission),
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    String[] permissions = {Manifest.permission.ACCESS_FINE_LOCATION};
                                    requestPermissions(permissions, REQUEST_PERMISSION_CODE);
                                }
                            });
                    return;
                }

                String[] permissions = {Manifest.permission.ACCESS_FINE_LOCATION};
                requestPermissions(permissions, REQUEST_PERMISSION_CODE);
            } else {
                jumpToActivity(SmartAccessActivity.this, RegistrationMainActivity.class, REQUEST_CODE_REGISTRATION);
            }
        } else {
            jumpToActivity(SmartAccessActivity.this, RegistrationMainActivity.class, REQUEST_CODE_REGISTRATION);
        }
    }

    private void showMessageOKCancel(String message, DialogInterface.OnClickListener okListener) {
        new AlertDialog.Builder(SmartAccessActivity.this)
                .setMessage(message)
                .setPositiveButton("OK", okListener)
                .setNegativeButton("Cancel", null)
                .create()
                .show();
    }

    private void showMessageOK(String message) {
        new AlertDialog.Builder(SmartAccessActivity.this)
                .setMessage(message)
                .setPositiveButton("OK", null)
                .create()
                .show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        Log.e("- - - - "," requestCode   :" + requestCode + " permissions   :" +  String.valueOf(permissions) + " grantResults   :" + String.valueOf(grantResults));

        for (int i = 0 ; i < grantResults.length; i++){
            Log.e("- - - - "," = = = = =  = =    :" + grantResults[i]);
        }
        if (REQUEST_PERMISSION_CODE == requestCode) {
            boolean grantedAccessCoarseLocation = true;
            for (int i = 0; i < permissions.length; i++) {
                String permission = permissions[i];
                if (Manifest.permission.ACCESS_COARSE_LOCATION.equals(permission)) {
                    grantedAccessCoarseLocation = grantResults[i] == PERMISSION_GRANTED;
                }
            }

            if (!grantedAccessCoarseLocation) {
                 Toast.makeText(this, "Please grant permissions", Toast.LENGTH_SHORT).show();
            }
        }
    }

    public void jumpToActivity(Context currentActivity, Class nextActivityClass, int requestCode){
        Intent pendingIntent = new Intent();
        pendingIntent.setClass(currentActivity, nextActivityClass);

        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_PHY, getIntent().getExtras().getInt(BaseServiceActivity.INTENT_KEY_PHY));
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_ADDRESS, getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_ADDRESS));
        pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_NAME, getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_NAME));

        if (requestCode == NO_REQUEST_CODE)
            startActivity(pendingIntent);
        else
            startActivityForResult(pendingIntent, requestCode);
    }

    private void treatReturnValue(int Result) {
        int algoVersion = new Algorithm().GetVersion();
        EventBus.getDefault().unregister(this);

        if (Result != algoVersion) {
            StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                    this, findViewById(R.id.smartaccess_view), getString(R.string.error_algo_version_mismatch));
        }else{
            StatusPopUp.getStatusPopUpInstance().dismiss(this);
        }

//        isSyncing = true;
//        BLEService.INSTANCE.getProtocol().sendGetUserInfoReq();
//        SdkUtils.changeToolbarFABButtonState(this, R.string.menu_scan_stop, R.string.sync, isSyncing);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.GetAlgoVersionRes e) {
        Log.d(TAG, "+GetAlgoVersionRes");

        if (e == null) return;

        treatReturnValue(e.mGetAlgoVersionResult);

        Log.d(TAG, "-GetAlgoVersionRes:" + e.mGetAlgoVersionResult);
    }
}
