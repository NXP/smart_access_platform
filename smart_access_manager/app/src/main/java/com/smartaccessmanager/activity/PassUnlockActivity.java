package com.smartaccessmanager.activity;

import static com.smartaccessmanager.utility.SdkUtils.fullScreen;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.google.android.material.textfield.TextInputEditText;
import com.smartaccessmanager.R;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.service.BLEService;
import com.smartaccessmanager.utility.StatusPopUp;
import com.smartaccessmanager.utility.UserInteractionTimer;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

public class PassUnlockActivity extends AppCompatActivity {

    private static final String TAG = "SLM_RA";

    private static final int PASSWORD_UNLOCK_RESULT_OK = 0;
    private static final int PASSWORD_UNLOCK_RESULT_INVALID = -1;
    private static final byte INVALID_PACKET = -2;

    private String user_id;

    TextInputEditText passwordText;

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        EventBus.getDefault().register(this);

        setContentView(R.layout.activity_pass_unlock);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        getSupportActionBar().setDisplayShowTitleEnabled(false);
        ((TextView) findViewById(R.id.toolbar_title)).setText(getString(R.string.password_unlock_title));
        ((TextView) findViewById(R.id.toolbar_subtitle)).setText("");

        passwordText = findViewById(R.id.pass_unlock_password);

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

    public void onUnlockFABPressed(View view) {
        BLEService.INSTANCE.getProtocol().sendPasswordUnlockReq(user_id, passwordText.getText().toString());
        StatusPopUp.getStatusPopUpInstance().showProgress(this, findViewById(R.id.pass_unlock_view), getString(R.string.unlock_processing));
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.PasswordUnlockRes e) {
        Log.d(TAG, "+PasswordUnlockRes");

        if (e == null) return;

        Log.d(TAG, " PasswordUnlockRes:" + e.mPasswordUnlockRes);

        if (e.mPasswordUnlockRes == PASSWORD_UNLOCK_RESULT_OK) {
            Log.d(TAG, "PasswordUnlockRes success");
            StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(this, findViewById(R.id.pass_unlock_view), getString(R.string.password_unlock_success));
        } else if (e.mPasswordUnlockRes == PASSWORD_UNLOCK_RESULT_INVALID) {
            Log.d(TAG, "PasswordUnlockRes failed");
            StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(this, findViewById(R.id.pass_unlock_view), getString(R.string.password_unlock_failed));
        }

        Log.d(TAG, "-PasswordUnlockRes");
    }

}