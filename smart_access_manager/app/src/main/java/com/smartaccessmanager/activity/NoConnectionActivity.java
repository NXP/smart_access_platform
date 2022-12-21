package com.smartaccessmanager.activity;

import static com.smartaccessmanager.utility.SdkUtils.fullScreen;

import androidx.appcompat.app.AppCompatActivity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import com.smartaccessmanager.R;
import com.smartaccessmanager.service.BLEService;

public class NoConnectionActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        setContentView(R.layout.activity_no_connection);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    static public void jumpToDisconnectActivity(Activity activity)
    {
        Intent pendingIntent = new Intent();

        /* Stop handlers from running on disconnect */
        SmartAccessActivity.appTypeHandler.removeCallbacksAndMessages(null);
        SmartAccessActivity.connectionCheckHandler.removeCallbacksAndMessages(null);
        MainActivity.wakeUpHandler.removeCallbacksAndMessages(null);
        try { activity.unregisterReceiver(BLEService.INSTANCE.mReceiver); }
        catch (IllegalArgumentException ignored) { }

        if(activity.getClass() == SmartAccessActivity.class) {
            pendingIntent.setClass(activity, MainActivity.class);
        } else {
            pendingIntent.setClass(activity, NoConnectionActivity.class);
        }
        activity.startActivity(pendingIntent);
    }

    private void jumpToMainActivity(){
        Intent pendingIntent = new Intent();
        pendingIntent.setClass(this, MainActivity.class);
        pendingIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(pendingIntent);
    }

    public void onClickGoToScan(View view){
        jumpToMainActivity();
    }

    @Override
    public void onBackPressed() {
        jumpToMainActivity();
    }
}