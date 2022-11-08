package com.smartaccessmanager.activity;

import static com.smartaccessmanager.utility.SdkUtils.fullScreen;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.PowerManager;
import android.util.Log;
import android.view.View;

import com.smartaccessmanager.R;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.service.BLEService;
import com.smartaccessmanager.utility.StatusPopUp;
import com.smartaccessmanager.utility.UserInteractionTimer;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

public class NfcUnlockActivity extends AppCompatActivity {

    private static final String TAG = "SLM_NUA";

    private static final int NFC_UNLOCK_RESULT_OK = 0;
    private static final int NFC_UNLOCK_RESULT_INVALID = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        EventBus.getDefault().register(this);

        setContentView(R.layout.activity_nfc_unlock);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.NfcUnlockRes e) {
        Log.d(TAG, "+NfcUnlockRes");

        if (e == null) return;

        Log.d(TAG, " NfcUnlockRes:" + e.mNfcUnlockRes);

        StatusPopUp.getStatusPopUpInstance().dismiss(this);

        if (e.mNfcUnlockRes == NFC_UNLOCK_RESULT_OK) {
            Log.d(TAG, "NfcUnlockRes success");
            StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(this, findViewById(R.id.nfc_unlock_view), getString(R.string.nfc_unlock_success));
        } else if (e.mNfcUnlockRes == NFC_UNLOCK_RESULT_INVALID) {
            Log.d(TAG, "NfcUnlockRes failed");
            StatusPopUp.getStatusPopUpInstance().showErrorPopUp(this, findViewById(R.id.nfc_unlock_view), getString(R.string.nfc_unlock_failed));
        }

        Log.d(TAG, "-NfcUnlockRes");
    }

    @Override
    protected void onResume() {
        super.onResume();
        UserInteractionTimer.getTimerInstance().startTimer(this);

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            String user_id = extras.getString("user_id");
            BLEService.INSTANCE.getProtocol().sendNfcUnlockReq(user_id);
            StatusPopUp.getStatusPopUpInstance().showProgress(NfcUnlockActivity.this, findViewById(R.id.nfc_unlock_view), getString(R.string.unlock_processing), getString(R.string.helper_nfc_registration));
        }
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

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        setResultToActivity(SmartAccessActivity.RESULT_CANCELED);
        finish();
    }

    public void onClickNFCCancel(View view) {
        setResultToActivity(SmartAccessActivity.RESULT_CANCELED);
        finish();
    }

}