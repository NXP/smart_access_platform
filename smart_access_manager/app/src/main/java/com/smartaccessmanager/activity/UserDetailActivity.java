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

import static com.smartaccessmanager.activity.SmartAccessActivity.NFC;
import static com.smartaccessmanager.activity.SmartAccessActivity.NFC_SUPPORT;
import static com.smartaccessmanager.activity.SmartAccessActivity.PASSWORD;
import static com.smartaccessmanager.activity.SmartAccessActivity.UWB;
import static com.smartaccessmanager.activity.SmartAccessActivity.UWB_SUPPORT;
import static com.smartaccessmanager.activity.SmartAccessActivity.FACE;
import static com.smartaccessmanager.activity.SmartAccessActivity.FACE_SUPPORT;
import static com.smartaccessmanager.activity.SmartAccessActivity.FINGERPRINT;
import static com.smartaccessmanager.activity.SmartAccessActivity.FINGERPRINT_SUPPORT;
import static com.smartaccessmanager.database.AppDatabase.databaseWriteExecutor;
import static com.smartaccessmanager.utility.SdkUtils.fullScreen;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.text.InputFilter;
import android.text.InputType;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.ScrollView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;
import com.smartaccessmanager.R;
import com.smartaccessmanager.UnlockMethod;
import com.smartaccessmanager.database.AppDatabase;
import com.smartaccessmanager.database.UserDao;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.service.BLEService;
import com.smartaccessmanager.utility.StatusPopUp;
import com.smartaccessmanager.utility.UserInteractionTimer;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class UserDetailActivity extends AppCompatActivity {
    private static final String TAG = "SLM_RA";

    private static final int REQUEST_CODE_REGISTRATION = 100;
    public static final int REGISTRATION_RESULT_DUPLICATE = 1;

    public static final int USER_DETAIL_RESULT_OK = 0;
    public static final int USER_DETAIL_RESULT_INVALID = -1;
    public final byte INVALID_PACKET = -2;

    private static final int BUTTON_DELETE = 0;
    private static final int BUTTON_UPDATE = 1;
    private static final int BUTTON_REREGISTER = 2;

    private AlertDialog reRegisterAlertBox;
    private AppDatabase db;
    private UserDao userDao;
    private PopupWindow popupWindow;
    private int pressedButton;
    private Intent pendingIntent;

    private String user_id;
    private String user_name;
    private boolean timeout_or_abort = false;

    private TextView userDetailTextTitle;
    private ScrollView userDetailScrollView;

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(this);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        fullScreen(getWindow());

        EventBus.getDefault().register(this);

        setContentView(R.layout.activity_user_detail);

        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
        getSupportActionBar().setDisplayShowTitleEnabled(false);
        ((TextView) findViewById(R.id.toolbar_title)).setText(getIntent().getExtras().getString(getString(R.string.intent_user_name)));

        String subtitle = "User ID is " + String.valueOf(getIntent().getExtras().getInt(getString(R.string.intent_user_id)));

        ((TextView) findViewById(R.id.toolbar_subtitle)).setText(subtitle);

        user_id = String.valueOf(getIntent().getExtras().getInt(getString(R.string.intent_user_id)));
        user_name = getIntent().getExtras().getString(getString(R.string.intent_user_name));
        
        db = AppDatabase.getDatabase(this);
        userDao = db.userDao();

        userDetailScrollView = findViewById(R.id.user_actions_scroll_view);

        userDetailScrollView.post(() -> {
            DisplayMetrics displayMetrics = this.getResources().getDisplayMetrics();

            userDetailScrollView.getLayoutParams().height = (int) (displayMetrics.heightPixels - userDetailScrollView.getTop());
            userDetailScrollView.setLayoutParams(userDetailScrollView.getLayoutParams());
        });
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
        if (reRegisterAlertBox != null && reRegisterAlertBox.isShowing()) reRegisterAlertBox.dismiss();
        EventBus.getDefault().unregister(this);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        switch (requestCode) {
            case REQUEST_CODE_REGISTRATION:
                if (resultCode == RESULT_OK) {
                    StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
                            this, findViewById(R.id.user_detail_view), getString(R.string.success_reregistration));
                } else if (resultCode == RESULT_CANCELED) {
//                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
//                            this, findViewById(R.id.user_detail_view), getString(R.string.success_reregistration_cancelled));
                } else if (resultCode == REGISTRATION_RESULT_DUPLICATE) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.user_detail_view), getString(R.string.error_duplicate));
                } else if (resultCode == INVALID_PACKET) {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.user_detail_view), getString(R.string.error_invalid_packet));
                } else {
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            this, findViewById(R.id.user_detail_view), getString(R.string.error_general));
                }
                break;
        }
    }

    private void setResultToActivity(int result, int type, String name) {
        Intent intent = new Intent();

        intent.putExtra(UserManagementFragment.USER_DETAIL_RETURN, type);
        if (type == UserManagementFragment.UPDATE_USER) {
            intent.putExtra(UserManagementFragment.NEW_USER_NAME, name);
        }

        if (getParent() == null) {
            setResult(result, intent);
        } else {
            getParent().setResult(result, intent);
        }
    }

    @Override
    public void onBackPressed() {
        if(StatusPopUp.getStatusPopUpInstance().popupShowing()){
            reRegisterAlertBox.dismiss();
            StatusPopUp.getStatusPopUpInstance().dismiss(this);
            try {
                if (!user_id.equals("") && (getClass() == UserDetailActivity.class)) {
                    timeout_or_abort = true;
                    BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(user_id));
                }
            } catch (NullPointerException ignored) {
                super.onBackPressed();
                finish();
            }
        } else {
            super.onBackPressed();
            finish();
        }
    }

    public void onBackFABPressed(View view) {
        if(StatusPopUp.getStatusPopUpInstance().popupShowing()){
            reRegisterAlertBox.dismiss();
            StatusPopUp.getStatusPopUpInstance().dismiss(this);
            try {
                if (!user_id.equals("") && (getClass() == UserDetailActivity.class)) {
                    timeout_or_abort = true;
                    BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(user_id));
                }
            } catch (NullPointerException ignored) {
                super.onBackPressed();
                finish();
            }
        } else {
            super.onBackPressed();
            finish();
        }
    }

    private void defocusEffect(float alpha){
        WindowManager.LayoutParams lp = this.getWindow().getAttributes();
        lp.alpha = alpha;
        this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        this.getWindow().setAttributes(lp);
    }

    private void openPopUp(int buttonType){

        pressedButton = buttonType;

        // inflate the layout of the popup window
        LayoutInflater inflater = (LayoutInflater)
                getSystemService(LAYOUT_INFLATER_SERVICE);
        View popupView = inflater.inflate(R.layout.popup, null);

        popupWindow = new PopupWindow(popupView, LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT, true);

        TextView popup_title = popupWindow.getContentView().findViewById(R.id.title_popup);
        TextInputEditText user_name_view = popupWindow.getContentView().findViewById(R.id.popup_user_detail_name);
        TextInputLayout user_name_layout = popupWindow.getContentView().findViewById(R.id.outlinedTextField);
        TextView popup_description = popupWindow.getContentView().findViewById(R.id.description_popup);
        TextView popup_question = popupWindow.getContentView().findViewById(R.id.question_popup);
        Button buttonOK = popupWindow.getContentView().findViewById(R.id.button_confirm_popup);

        user_name_view.setText(user_name);

        defocusEffect(0.7f);

        popupWindow.setOnDismissListener(() -> defocusEffect(1.0f));

        findViewById(R.id.user_detail_view).post(new Runnable() {
            public void run() {
                // show the popup window
                popupWindow.showAtLocation(findViewById(R.id.user_detail_view), Gravity.CENTER, 0, 0);
            }
        });

        switch(buttonType){
            case BUTTON_UPDATE:
                user_name_layout.setVisibility(View.VISIBLE);
                popup_description.setVisibility(View.GONE);
                popup_question.setVisibility(View.GONE);

                popup_title.setText(getString(R.string.update_title_popup));
                buttonOK.setBackgroundColor(getResources().getColor(R.color.button_blue));
                buttonOK.setText(getString(R.string.button_update_name));
                break;
            case BUTTON_DELETE:
                user_name_layout.setVisibility(View.GONE);
                popup_description.setVisibility(View.VISIBLE);
                popup_question.setVisibility(View.VISIBLE);

                popup_title.setText(getString(R.string.delete_title_popup));
                buttonOK.setBackgroundColor(getResources().getColor(R.color.red));
                buttonOK.setText(getString(R.string.button_delete));
                break;
        }
    }

    public void onClickConfirm(View view) {
        Log.d(TAG, "+onClickConfirm");

        switch(pressedButton){
            case BUTTON_UPDATE:
                TextInputEditText popup_name_view = popupWindow.getContentView().findViewById(R.id.popup_user_detail_name);
                user_name = popup_name_view.getText().toString().replaceAll("( +)"," ").trim();

                if (!user_name.matches("[a-zA-Z]+[a-zA-Z0-9]*")){
                    StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                            UserDetailActivity.this, findViewById(R.id.user_detail_view), getString(R.string.error_registration_no_name));
                    InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                    imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
                    return;
                }

                StatusPopUp.getStatusPopUpInstance().showProgress(
                        this, findViewById(R.id.user_detail_view), getString(R.string.state_updating_name));
                BLEService.INSTANCE.getProtocol().sendUpdateUserInfoReq(Integer.parseInt(user_id),user_name);
                break;
            case BUTTON_DELETE:
                StatusPopUp.getStatusPopUpInstance().showProgress(
                        this, findViewById(R.id.user_detail_view), getString(R.string.state_deleting_user));
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(user_id));
                break;
        }
    }

    public void onClickCancel(View view) {
        Log.d(TAG, "+onClickCancel");
        popupWindow.dismiss();
    }

    public void onClickDelete(View view) {
        Log.d(TAG, "+onClickDelete");
        openPopUp(BUTTON_DELETE);
    }

    public void onClickUpdateName(View view) {
        Log.d(TAG, "+onClickUpdateName");
        openPopUp(BUTTON_UPDATE);
    }

    public class UnlockAdapter extends RecyclerView.Adapter<UserDetailActivity.UnlockAdapter.ViewHolder> {

        public class ViewHolder extends RecyclerView.ViewHolder {

            public TextView titleTextView;
            public TextView detailTextView;
            public ImageView icon;

            public ViewHolder(@NonNull View itemView) {
                super(itemView);
                titleTextView = (TextView) itemView.findViewById(R.id.unlock_title);
                detailTextView = (TextView) itemView.findViewById(R.id.unlock_detail);
                icon = (ImageView) itemView.findViewById(R.id.button_icon_unlock);
            }
        }

        private List<UnlockMethod> mUnlockMethods = null;
        private View.OnClickListener mClickListener = null;

        // Pass in the users array into the constructor
        public UnlockAdapter(List<UnlockMethod> unlockMethods, View.OnClickListener clickListener){
            mUnlockMethods = unlockMethods;
            mClickListener = clickListener;
        }

        @Override
        public UserDetailActivity.UnlockAdapter.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            Context context = parent.getContext();
            LayoutInflater inflater = LayoutInflater.from(context);

            // Inflate the custom layout
            View unlockOptionView = inflater.inflate(R.layout.unlock_option_item, parent, false);
            unlockOptionView.setOnClickListener(this.mClickListener);

            return new UserDetailActivity.UnlockAdapter.ViewHolder(unlockOptionView);
        }

        @Override
        public void onBindViewHolder(UserDetailActivity.UnlockAdapter.ViewHolder holder, int position) {
            // Set item views based on your views and data model
            holder.titleTextView.setText(mUnlockMethods.get(position).title);
            holder.detailTextView.setText(String.valueOf(mUnlockMethods.get(position).description));
            holder.icon.setImageDrawable(ContextCompat.getDrawable(UserDetailActivity.this, mUnlockMethods.get(position).icon));
            if(position == 2){
                holder.icon.setScaleX(1.4f);
                holder.icon.setScaleY(1.4f);
            }
        }

        @Override
        public int getItemCount() {
            return mUnlockMethods.size();
        }
    }

    private RecyclerView mRecyclerView;
    private List<UnlockMethod> unlockMethodList = null;

    private final View.OnClickListener mUnlockClickHandler = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            int itemPosition = mRecyclerView.getChildAdapterPosition(v);

            Intent pendingIntent = new Intent();
            pendingIntent.setClass(UserDetailActivity.this, unlockMethodList.get(itemPosition).intentClass);
            pendingIntent.putExtra("user_id", user_id);
            UserDetailActivity.this.startActivity(pendingIntent);
        }
    };

    public void onClickUnlock(View view) {
        Log.d(TAG, "+onClickUnlock");

        LayoutInflater factory = LayoutInflater.from(this);
        View unlockAlertView = factory.inflate(R.layout.fragment_unlock, null);

        unlockMethodList = new ArrayList<>();
        unlockMethodList.add(new UnlockMethod(getString(R.string.password_unlock),
                getString(R.string.password_unlock_detail),
                R.drawable.ic_baseline_lock_24,
                PassUnlockActivity.class, PASSWORD));
//        if(NFC_SUPPORT)
//            unlockMethodList.add(new UnlockMethod(getString(R.string.nfc_unlock),
//                    getString(R.string.nfc_unlock_detail),
//                    R.drawable.ic_baseline_nfc_24,
//                    NfcUnlockActivity.class, NFC));
        if(UWB_SUPPORT)
            unlockMethodList.add(new UnlockMethod(getString(R.string.uwb_unlock),
                    getString(R.string.uwb_unlock_detail),
                    R.drawable.ic_uwb_lock_icon,
                    UwbUnlockActivity.class, UWB));

        UserDetailActivity.UnlockAdapter adapter;
        adapter = new UserDetailActivity.UnlockAdapter(unlockMethodList, mUnlockClickHandler);

        mRecyclerView = (RecyclerView) unlockAlertView.findViewById(R.id.unlock_option_list);
        mRecyclerView.setHasFixedSize(true);
        mRecyclerView.setAdapter(adapter);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(this));

        AlertDialog.Builder alert = new AlertDialog.Builder(UserDetailActivity.this);
        alert.setView(unlockAlertView);
        AlertDialog dialog = alert.create();
        dialog.show();
    }

    private List<UnlockMethod> reRegisterMethodList = null;

    private final View.OnClickListener mReRegisterClickHandler = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            int itemPosition = mRecyclerView.getChildAdapterPosition(v);

            UnlockMethod clickedMethod = reRegisterMethodList.get(itemPosition);
            switch (clickedMethod.tag) {
                case PASSWORD:
                    Intent change_password = new Intent(UserDetailActivity.this, ChangePasswordActivity.class);
                    change_password.putExtra("user_id",user_id);
                    startActivity(change_password);
                    break;
                case FACE:
                    Intent change_face = new Intent(UserDetailActivity.this, RegistrationMainActivity.class);
                    change_face.putExtra("user_id",user_id);
                    change_face.putExtra("user_name",user_name);
                    startActivity(change_face);
                    break;
                case NFC:
                    StatusPopUp.getStatusPopUpInstance().showProgress(UserDetailActivity.this, findViewById(R.id.user_detail_view), getString(R.string.state_registering), getString(R.string.helper_nfc_registration));
                    BLEService.INSTANCE.getProtocol().sendRecordNfcReq(user_id);
                    break;
                case UWB:
                    break;
                case FINGERPRINT:
                    StatusPopUp.getStatusPopUpInstance().showProgress(UserDetailActivity.this, findViewById(R.id.user_detail_view), getString(R.string.state_registering), getString(R.string.helper_fingerprint_registration));
                    BLEService.INSTANCE.getProtocol().sendEnrollFingerprintReq(user_id);
                    break;
            }

            reRegisterAlertBox.hide();
        }
    };

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.ConnectionTimeout e) {
        Log.d(TAG, "+ConnectionTimeout");

        if (e == null) return;

        reRegisterAlertBox.dismiss();
        StatusPopUp.getStatusPopUpInstance().dismiss(this);

        if (!user_id.equals("") && (getClass() == UserDetailActivity.class)) {
            timeout_or_abort = true;
            BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(user_id));
        }

        Log.d(TAG, "-ConnectionTimeout");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.RecordNfcRes e) {
        Log.d(TAG, "+RecordNfcRes");

        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(UserDetailActivity.this);

        if(e.mRecordNfcResult == 0) {
            StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(UserDetailActivity.this,
                    findViewById(R.id.user_detail_view), getString(R.string.success_reregistration));
        }

        Log.d(TAG, "-RecordNfcRes:" + e.mRecordNfcResult);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.EnrollFingerprintRes e) {
        Log.d(TAG, "+EnrollFingerprintRes");

        if (e == null) return;

        if (e.mEnrollFingerprintProgress <= -1)
            StatusPopUp.getStatusPopUpInstance().dismiss(UserDetailActivity.this);

        if (e.mEnrollFingerprintResult == 0) {
            if (e.mEnrollFingerprintProgress > -1) {
                StatusPopUp.getStatusPopUpInstance().updateFingerprintProgress(e.mEnrollFingerprintProgress);
            } else {
                StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(UserDetailActivity.this,
                        findViewById(R.id.user_detail_view), getString(R.string.success_reregistration));
            }
        }

        Log.d(TAG, "-EnrollFingerprintRes:" + e.mEnrollFingerprintResult);
    }

    public void onClickReRegister(View view) {
        Log.d(TAG, "+onClickReRegister");

        LayoutInflater factory = LayoutInflater.from(this);
        View reRegisterAlertView = factory.inflate(R.layout.fragment_unlock, null);

        reRegisterMethodList = new ArrayList<>();
        reRegisterMethodList.add(new UnlockMethod(getString(R.string.password_unlock),
                getString(R.string.password_unlock_detail),
                R.drawable.ic_baseline_lock_24,
                null, PASSWORD));
        if(NFC_SUPPORT)
            reRegisterMethodList.add(new UnlockMethod(getString(R.string.nfc_unlock),
                    getString(R.string.nfc_unlock_detail),
                    R.drawable.ic_baseline_nfc_24,
                    null, NFC));
        if(UWB_SUPPORT)
            reRegisterMethodList.add(new UnlockMethod(getString(R.string.uwb_unlock),
                    getString(R.string.uwb_unlock_detail),
                    R.drawable.ic_uwb_lock_icon,
                    null, UWB));
//        if (FACE_SUPPORT)
//            reRegisterMethodList.add(new UnlockMethod(getString(R.string.face_unlock),
//                    getString(R.string.face_unlock_detail),
//                    R.drawable.ic_baseline_face_24,
//                    null, FACE));
        if (FINGERPRINT_SUPPORT)
            reRegisterMethodList.add(new UnlockMethod(getString(R.string.fingerprint_unlock),
                    getString(R.string.fingerprint_unlock_detail),
                    R.drawable.ic_baseline_fingerprint_24,
                    null, FINGERPRINT));

        UserDetailActivity.UnlockAdapter adapter;
        adapter = new UserDetailActivity.UnlockAdapter(reRegisterMethodList, mReRegisterClickHandler);

        mRecyclerView = (RecyclerView) reRegisterAlertView.findViewById(R.id.unlock_option_list);
        mRecyclerView.setHasFixedSize(true);
        mRecyclerView.setAdapter(adapter);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(this));

        AlertDialog.Builder alert = new AlertDialog.Builder(UserDetailActivity.this);
        alert.setView(reRegisterAlertView);
        reRegisterAlertBox = alert.create();
        reRegisterAlertBox.show();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.UpdateUserInfoRes e) {
        Log.d(TAG, "+UpdateUserInfoRes");
        if (e == null) return;

        switch (e.mUpdateUserInfoResult){
            case USER_DETAIL_RESULT_OK:
                ((TextView) findViewById(R.id.toolbar_title)).setText(user_name);

                databaseWriteExecutor.execute(() -> {
                    userDao.updateName(user_name, Integer.parseInt(user_id));
                });

                if (popupWindow !=  null) {
                    popupWindow.dismiss();
                }

                setResultToActivity(getIntent().getExtras().getInt(getString(R.string.intent_user_position)),
                        UserManagementFragment.UPDATE_USER, user_name);

                StatusPopUp.getStatusPopUpInstance().showSuccessPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.success_update));
                break;
            case USER_DETAIL_RESULT_INVALID:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_command_sending));
                break;
            case INVALID_PACKET:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_invalid_packet));
                break;
            default:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_duplicate));
        }


        Log.d(TAG, "-UpdateUserInfoRes:" + e.mUpdateUserInfoResult);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.DeleteUserRes e) {
        Log.d(TAG, "+DeleteUserInfoRes");
        if (e == null) return;

        if (timeout_or_abort) {
            finish();
            return;
        }

        switch (e.mDeleteUserResult){
            case USER_DETAIL_RESULT_OK:
                int id = Integer.parseInt(user_id);

                databaseWriteExecutor.execute(() -> {
                    userDao.deleteByID(id);
                });

                popupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
                    @Override
                    public void onDismiss() {
                        setResultToActivity(getIntent().getExtras().getInt(getString(R.string.intent_user_position)),
                                UserManagementFragment.DELETE_USER, null);
                        UserDetailActivity.this.finish();
                    }
                });

                StatusPopUp.getStatusPopUpInstance().dismiss(this);

                if (popupWindow !=  null) {
                    popupWindow.dismiss();
                }

                break;
            case USER_DETAIL_RESULT_INVALID:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_command_sending));
                break;
            case INVALID_PACKET:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_invalid_packet));
                break;
            default:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        this, findViewById(R.id.user_detail_view), getString(R.string.error_general));
        }


        Log.d(TAG, "-DeleteUserInfoRes:" + e.mDeleteUserResult);
    }
}