package com.smartaccessmanager.activity;

import static com.smartaccessmanager.utility.SdkUtils.fullScreen;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.fragment.app.Fragment;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
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

public class RegistrationMainActivity extends AppCompatActivity {

    private static final String TAG = "SLM_RCA";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        EventBus.getDefault().register(this);

        setContentView(R.layout.activity_registration_main);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        getSupportActionBar().setDisplayShowTitleEnabled(false);

        if (savedInstanceState == null) {
            Bundle extras = getIntent().getExtras();
            if (extras.getString("user_id") != null && extras.getString("user_name") != null) {
                // re-register face
                Fragment regCameraFrag = new RegistrationCameraFragment();
                Bundle args = new Bundle();
                args.putString("user_id", extras.getString("user_id"));
                args.putString("user_name", extras.getString("user_name"));
                regCameraFrag.setArguments(args);

                getSupportFragmentManager().beginTransaction()
                        .setReorderingAllowed(true)
                        .add(R.id.fragment_container_view, regCameraFrag, null)
                        .commit();
            } else {
                getSupportFragmentManager().beginTransaction()
                        .setReorderingAllowed(true)
                        .add(R.id.fragment_container_view, RegistrationSelectFragment.class, null)
                        .commit();
            }
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

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.DeleteUserRes e) {
        Log.d(TAG, "+DeleteUserInfoRes");

        setResultToActivity(RegistrationMainActivity.RESULT_CANCELED);
        finish();

        Log.d(TAG, "-DeleteUserInfoRes:" + e.mDeleteUserResult);
    }

    public void setResultToActivity(int result) {
        Intent intent = new Intent();
        if (getParent() == null) {
            setResult(result, intent);
        } else {
            getParent().setResult(result, intent);
        }
    }

    @Override
    public void onBackPressed() {
        if (StatusPopUp.getStatusPopUpInstance().popupShowing()) {
            StatusPopUp.getStatusPopUpInstance().dismiss(this);
            try {
                SharedPreferences sharedPref = getPreferences(Context.MODE_PRIVATE);
                String nameID = sharedPref.getString(RegistrationConfirmFragment.getUserName(), "");
                if (!nameID.equals("") && (getClass() == RegistrationMainActivity.class)) {
                    BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                }
            } catch (NullPointerException e) {
                setResultToActivity(RegistrationMainActivity.RESULT_CANCELED);
                finish();
            }
        } else {
            setResultToActivity(RegistrationMainActivity.RESULT_CANCELED);
            finish();
        }
    }

    public void onBackFABPressed(View view) {
        if (StatusPopUp.getStatusPopUpInstance().popupShowing()) {
            StatusPopUp.getStatusPopUpInstance().dismiss(this);
            try {
                SharedPreferences sharedPref = getPreferences(Context.MODE_PRIVATE);
                String nameID = sharedPref.getString(RegistrationConfirmFragment.getUserName(), "");
                if (!nameID.equals("") && (getClass() == RegistrationMainActivity.class)) {
                    BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                }
            } catch (NullPointerException e) {
                setResultToActivity(RegistrationMainActivity.RESULT_CANCELED);
                finish();
            }
        } else {
            setResultToActivity(RegistrationMainActivity.RESULT_CANCELED);
            finish();
        }

        Log.d(TAG, "+onClickCancel");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }
}