package com.smartaccessmanager.activity;

import static com.smartaccessmanager.utility.SdkUtils.fullScreen;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.smartaccessmanager.R;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.service.BLEService;
import com.smartaccessmanager.utility.StatusPopUp;
import com.smartaccessmanager.utility.UserInteractionTimer;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

public class UwbUnlockActivity extends AppCompatActivity {

    private static final String TAG = "SLM_UUA";

    private static final int UWB_UNLOCK_RESULT_OK = 0;
    private static final int UWB_UNLOCK_RESULT_INVALID = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        EventBus.getDefault().register(this);

        setContentView(R.layout.activity_uwb_unlock);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.UwbUnlockRes e) {
        Log.d(TAG, "+UwbUnlockRes");

        if (e == null) return;

        Log.d(TAG, " UwbUnlockRes:" + e.mUwbUnlockRes);

        StatusPopUp.getStatusPopUpInstance().dismiss(this);

        if (e.mUwbUnlockRes == UWB_UNLOCK_RESULT_OK) {
            Log.d(TAG, "UwbUnlockRes success");
            StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(this, findViewById(R.id.uwb_unlock_view), getString(R.string.uwb_unlock_success));
        } else if (e.mUwbUnlockRes == UWB_UNLOCK_RESULT_INVALID) {
            Log.d(TAG, "UwbUnlockRes failed");
            StatusPopUp.getStatusPopUpInstance().showErrorPopUp(this, findViewById(R.id.uwb_unlock_view), getString(R.string.uwb_unlock_failed));
        }

        Log.d(TAG, "-UwbUnlockRes");
    }

    @Override
    protected void onResume() {
        super.onResume();
        UserInteractionTimer.getTimerInstance().startTimer(this);

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            String user_id = extras.getString("user_id");
            BLEService.INSTANCE.getProtocol().sendUwbUnlockReq(user_id);
            StatusPopUp.getStatusPopUpInstance().showProgress(this, findViewById(R.id.uwb_unlock_view), getString(R.string.unlock_processing), getString(R.string.helper_uwb_registration));
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
        EventBus.getDefault().register(this);
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