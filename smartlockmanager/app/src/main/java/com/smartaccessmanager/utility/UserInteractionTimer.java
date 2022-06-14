package com.smartaccessmanager.utility;

import android.app.Activity;
import android.os.CountDownTimer;

import com.smartaccessmanager.activity.BaseServiceActivity;
import com.smartaccessmanager.event.BaseEvent;

import org.greenrobot.eventbus.EventBus;

public class UserInteractionTimer extends BaseServiceActivity{

    private static final int TIMEOUT = Integer.MAX_VALUE;//120000;
    private static final int TICK = Integer.MAX_VALUE;//120000;

    CountDownTimer timer = null;

    private static volatile UserInteractionTimer TIMER_INSTANCE = null;

    /*
     * TIMER FINISHED EVENT
     */
    public static class TimerFinished extends BaseEvent {
    }

    public static UserInteractionTimer getTimerInstance() {
        if (TIMER_INSTANCE == null) {
            synchronized (UserInteractionTimer.class) {
                if (TIMER_INSTANCE == null) {
                    TIMER_INSTANCE = new UserInteractionTimer();
                }
            }
        }
        return TIMER_INSTANCE;
    }

    public void startTimer(Activity activity) {
        EventBus.getDefault().register(TIMER_INSTANCE);

        if( timer != null ) {
            timer.cancel();
        }

        timer = new CountDownTimer(TIMEOUT, TICK) {
            @Override
            public void onTick(long millisUntilFinished) { }

            @Override
            public void onFinish() {
                releaseConnection();
            }
        }.start();
    }

    public void resetTimer(){
        if( timer != null ) {
            timer.cancel();
            timer.start();
        }
    }

    public void stopTimer(){
        EventBus.getDefault().unregister(TIMER_INSTANCE);

        if( timer != null ) {
            timer.cancel();
        }
    }
}
