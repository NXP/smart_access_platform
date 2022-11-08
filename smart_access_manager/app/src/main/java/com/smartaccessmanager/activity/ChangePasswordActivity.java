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

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.smartaccessmanager.R;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.service.BLEService;
import com.smartaccessmanager.utility.StatusPopUp;
import com.smartaccessmanager.utility.UserInteractionTimer;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import static com.smartaccessmanager.utility.SdkUtils.fullScreen;

import java.util.regex.Pattern;

public class ChangePasswordActivity extends AppCompatActivity {

    private static final String TAG = "SLM_RA";

    private static EditText passwordOld = null;
    private static EditText passwordNew = null;

    private String user_id;

    private static final int UPDATE_PASSWORD_RESULT_OK = 0;
    private static final int UPDATE_PASSWORD_RESULT_INVALID = -1;
    private static final byte INVALID_PACKET = -2;

    private static final Pattern passwordPattern = Pattern.compile("[0-9][0-9][0-9][0-9][0-9][0-9]", Pattern.CASE_INSENSITIVE);

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        EventBus.getDefault().register(this);

        setContentView(R.layout.activity_change_password);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        getSupportActionBar().setDisplayShowTitleEnabled(false);
        ((TextView) findViewById(R.id.toolbar_title)).setText(getString(R.string.change_password));
        ((TextView) findViewById(R.id.toolbar_subtitle)).setText("");

        CircularProgressIndicator circularProgressIndicator = findViewById(R.id.toolbar_loading);
        circularProgressIndicator.setVisibility(View.GONE);

        TextView fab_name = findViewById(R.id.toolbar_loading_text);
        ImageView fab_image = findViewById(R.id.toolbar_loading_static);

        fab_image.setImageDrawable(getDrawable(R.drawable.ic_baseline_replace_24));
        fab_image.setVisibility(View.VISIBLE);
        fab_name.setText(getString(R.string.change));

        passwordOld = findViewById(R.id.password_old);
        passwordNew = findViewById(R.id.password_new);

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            user_id = extras.getString("user_id");
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
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
        UserInteractionTimer.getTimerInstance().stopTimer();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);
    }

    private void setResultToActivity(int result) {
        Intent intent = new Intent();
        if (getParent() == null) {
            setResult(result, intent);
        } else {
            getParent().setResult(result, intent);
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        setResultToActivity(SmartAccessActivity.RESULT_CANCELED);
        finish();
    }

    public void onBackFABPressed(View view) {
        super.onBackPressed();
        setResultToActivity(SmartAccessActivity.RESULT_CANCELED);
        finish();
    }

    public void onPressFABCardView(View view) {
        Log.d(TAG, "+onClickOk");

        String s_passwordOld = passwordOld.getText().toString();
        String s_passwordNew = passwordNew.getText().toString();

        SharedPreferences sharedPref = getSharedPreferences(getString(R.string.password_shared_pref), Context.MODE_PRIVATE);
        String memoryOldPassword = sharedPref.getString(user_id, "");

        if (!passwordPattern.matcher(s_passwordOld).find() || !passwordPattern.matcher(s_passwordNew).find()){
            StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                    this, findViewById(R.id.changing_password_view), getString(R.string.error_incorrect_password));
            return;
        }

        if (!s_passwordOld.equals(memoryOldPassword)) {
            StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                    this, findViewById(R.id.changing_password_view), getString(R.string.error_wrong_old_password));
            return;
        }

        StatusPopUp.getStatusPopUpInstance().showProgress(
                this, findViewById(R.id.changing_password_view), getString(R.string.state_changing_password));
        BLEService.INSTANCE.getProtocol().sendUpdatePasswordReq(user_id, s_passwordNew);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.UpdatePasswordRes e) {
        Log.d(TAG, "+RegistrationRes");

        if (e == null) return;

        switch (e.mUpdatePasswordResult){
            case UPDATE_PASSWORD_RESULT_OK:
                SharedPreferences sharedPref = ChangePasswordActivity.
                        this.getSharedPreferences(getString(R.string.password_shared_pref), Context.MODE_PRIVATE);
                SharedPreferences.Editor editor = sharedPref.edit();
                editor.putString(user_id, passwordNew.getText().toString());
                editor.commit();

                StatusPopUp.getStatusPopUpInstance().dismiss(this);

                setResultToActivity(SmartAccessActivity.RESULT_OK);
                finish();
                break;
            case UPDATE_PASSWORD_RESULT_INVALID:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.changing_password_view), getString(R.string.error_wrong_password));
                break;
            case INVALID_PACKET:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.changing_password_view), getString(R.string.error_invalid_packet));
                break;
            default:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.changing_password_view), getString(R.string.error_general));
        }

        Log.d(TAG, "-UpdatePasswordRes:" + e.mUpdatePasswordResult);
    }
}