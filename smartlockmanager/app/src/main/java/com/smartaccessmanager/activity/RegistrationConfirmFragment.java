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

import static android.content.Context.LAYOUT_INFLATER_SERVICE;
import static com.smartaccessmanager.activity.SmartAccessActivity.NFC;
import static com.smartaccessmanager.activity.SmartAccessActivity.FACE;
import static com.smartaccessmanager.activity.SmartAccessActivity.FINGERPRINT;
import static com.smartaccessmanager.activity.SmartAccessActivity.UWB;
import static com.smartaccessmanager.activity.UserDetailActivity.USER_DETAIL_RESULT_INVALID;
import static com.smartaccessmanager.activity.UserDetailActivity.USER_DETAIL_RESULT_OK;
import static com.smartaccessmanager.database.AppDatabase.databaseWriteExecutor;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.cardview.widget.CardView;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;

import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.google.android.material.textfield.TextInputEditText;
import com.google.android.material.textfield.TextInputLayout;
import com.smartaccessmanager.R;
import com.smartaccessmanager.database.AppDatabase;
import com.smartaccessmanager.database.UserDao;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.service.BLEService;
import com.smartaccessmanager.utility.StatusPopUp;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.regex.Pattern;

public class RegistrationConfirmFragment extends Fragment {
    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "SLM_RCA";

    private static final Pattern namePattern = Pattern.compile("[a-zA-Z0-9]+", Pattern.CASE_INSENSITIVE);
    private static final Pattern passwordPattern = Pattern.compile("[0-9][0-9][0-9][0-9][0-9][0-9]", Pattern.CASE_INSENSITIVE);

    private Bitmap mFaceImage;
    private byte[] mFaceFeature;

    private ImageView mFaceImageView;
    private EditText mNameEditText;
    private boolean reRegistration = false;
    private String duplicateFaceName = null;
    private TextInputLayout mPasswordEditText;
    private ArrayList<Integer> selectedMethods = new ArrayList<>();
    private AppDatabase db;
    private UserDao userDao;

    private static final int REGISTRATION_CMD_RESULT_OK = 0;
    private static final int REGISTRATION_CMD_RESULT_INVALID = -1;
    private static final int REGISTRATION_CMD_RESULT_DUPLICATE = 1;

    private static final int REGISTRATION_RESULT_OK = 0;
    private static final int REGISTRATION_RESULT_INVALID = -1;
    private static final int REGISTRATION_RESULT_DUPLICATE = 1;

    private static final int SET_PWD_RESULT_OK = 0;
    private static final int SET_PWD_RESULT_INVALID = -1;
    private static final int SET_PWD_RESULT_DUPLICATE = 1;

    private static final int NFC_RESULT_OK = 0;
    private static final int NFC_RESULT_INVALID = -1;
    private static final int NFC_RESULT_DUPLICATE = 1;

    private static final int UWB_RESULT_OK = 0;
    private static final int UWB_RESULT_INVALID = -1;
    private static final int UWB_RESULT_DUPLICATE = 1;

    private static final int FINGERPRINT_RESULT_OK = 0;
    private static final int FINGERPRINT_RESULT_INVALID = -1;
    private static final int FINGERPRINT_RESULT_DUPLICATE = 1;

    private static final int INVALID_PACKET = -2;

    private PopupWindow mPopupWindow;

    /**
     * Convert String to bitmap.
     *
     * @param encodedString image String
     * @return android.graphics.Bitmap
     */
    public Bitmap stringToBitMap(String encodedString) {
        try {
            byte[] encodeByte = Base64.decode(encodedString, Base64.DEFAULT);
            return BitmapFactory.decodeByteArray(encodeByte, 0, encodeByte.length);
        } catch (Exception e) {
            e.getMessage();
            return null;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_registration_confirm, container, false);

        ((TextView) getActivity().findViewById(R.id.toolbar_title)).setText(getString(R.string.app_registration_confirm));
        ((TextView) getActivity().findViewById(R.id.toolbar_subtitle)).setText("");


        if(getArguments() != null && getArguments().getString(BaseServiceActivity.INTENT_KEY_IMAG) != null) {
            String strBase64Image = getArguments().getString(BaseServiceActivity.INTENT_KEY_IMAG);
            mFaceImageView = view.findViewById(R.id.imageview);
            mFaceImage = stringToBitMap(strBase64Image);
            if (mFaceImageView != null) {
                mFaceImageView.setImageBitmap(mFaceImage);
            }
            mFaceFeature = getArguments().getByteArray(BaseServiceActivity.INTENT_KEY_FEATURE);
        }else{
            view.findViewById(R.id.imageview_confirm).setVisibility(View.GONE);
            view.findViewById(R.id.imageview_confirm_details).setVisibility(View.GONE);
        }

        mNameEditText = view.findViewById(R.id.registration_name);
        mPasswordEditText = view.findViewById(R.id.registration_password);

        db = AppDatabase.getDatabase(getActivity());
        userDao = db.userDao();

        if (getArguments() != null && getArguments().getString(getString(R.string.intent_user_name)) != null){
            reRegistration = true;

            view.findViewById(R.id.outlinedTextField).setVisibility(View.GONE);
            view.findViewById(R.id.outlinedTextFieldFixed).setVisibility(View.VISIBLE);

            mNameEditText = view.findViewById(R.id.registration_name_fixed);
            mNameEditText.setText(getArguments().getString(getString(R.string.intent_user_name)));
        }

        if(getArguments() != null && getArguments().getIntegerArrayList(BaseServiceActivity.INTENT_KEY_METHODS) != null) {
            selectedMethods = getArguments().getIntegerArrayList(BaseServiceActivity.INTENT_KEY_METHODS);
        }

        CircularProgressIndicator circularProgressIndicator = getActivity().findViewById(R.id.toolbar_loading);
        circularProgressIndicator.setVisibility(View.GONE);

        TextView fab_name = getActivity().findViewById(R.id.toolbar_loading_text);
        ImageView fab_image = getActivity().findViewById(R.id.toolbar_loading_static);

        fab_image.setImageDrawable(ContextCompat.getDrawable(getActivity(),R.drawable.ic_baseline_arrow_forward_24));
        fab_image.setVisibility(View.VISIBLE);
        fab_name.setText(getString(R.string.button_register));

        CardView register_button = getActivity().findViewById(R.id.register_fab_cardview);
        register_button.setVisibility(View.VISIBLE);
        register_button.setOnClickListener(v -> pressFABCardButton(v));

        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        EventBus.getDefault().register(this);
    }

    @Override
    public void onPause() {
        super.onPause();
        EventBus.getDefault().unregister(this);
    }

    private boolean isMethodSelected(int tag) {
        return selectedMethods.contains(tag);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(getActivity());
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.RecordUwbRes e) {
        Log.d(TAG, "+RecordUwbRes");

        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(getActivity());

        Log.d(TAG, " RecordUwbRes:" + e.mRecordUwbResult);

        /* Here, additional methods may be called to disable the phone uwb */

        SharedPreferences sharedPref = getActivity().getPreferences(Context.MODE_PRIVATE);
        String nameID = sharedPref.getString(fixInputField(mNameEditText.getText().toString()), "");

        switch (e.mRecordUwbResult) {
            case UWB_RESULT_OK:
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.RESULT_OK);
                getActivity().finish();
                break;
            case UWB_RESULT_INVALID:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.UWB_RESULT_INVALID);
                getActivity().finish();
                break;
            case UWB_RESULT_DUPLICATE:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.UWB_RESULT_DUPLICATE);
                getActivity().finish();
                break;
            case INVALID_PACKET:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.INVALID_PACKET);
                getActivity().finish();
                break;
            default:
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.GENERAL_ERROR);
                getActivity().finish();
                break;
        }

        Log.d(TAG, "-RecordUwbRes");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.RecordNfcRes e) {
        Log.d(TAG, "+RecordNfcRes");

        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(getActivity());

        Log.d(TAG, " RecordNfcRes:" + e.mRecordNfcResult);

        SharedPreferences sharedPref = getActivity().getPreferences(Context.MODE_PRIVATE);
        String nameID = sharedPref.getString(fixInputField(mNameEditText.getText().toString()), "");

        switch (e.mRecordNfcResult) {
            case NFC_RESULT_OK:
                if (isMethodSelected(UWB)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_uwb_registration));
                    BLEService.INSTANCE.getProtocol().sendRecordUwbReq(nameID);
                } else {
                    ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.RESULT_OK);
                    getActivity().finish();
                }
                break;
            case NFC_RESULT_INVALID:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.NFC_RESULT_INVALID);
                getActivity().finish();
                break;
            case NFC_RESULT_DUPLICATE:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.NFC_RESULT_DUPLICATE);
                getActivity().finish();
                break;
            case INVALID_PACKET:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.INVALID_PACKET);
                getActivity().finish();
                break;
            default:
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.GENERAL_ERROR);
                getActivity().finish();
                break;
        }

        Log.d(TAG, "-RecordNfcRes");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.EnrollFingerprintRes e) {
        Log.d(TAG, "+EnrollFingerprintRes");

        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(getActivity());

        Log.d(TAG, " EnrollFingerprintRes:" + e.mEnrollFingerprintResult);

        SharedPreferences sharedPref = getActivity().getPreferences(Context.MODE_PRIVATE);
        String nameID = sharedPref.getString(fixInputField(mNameEditText.getText().toString()), "");

        switch (e.mEnrollFingerprintResult) {
            case FINGERPRINT_RESULT_OK:
                if (isMethodSelected(NFC)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_nfc_registration));
                    BLEService.INSTANCE.getProtocol().sendRecordNfcReq(nameID);
                } else if (isMethodSelected(UWB)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_uwb_registration));
                    BLEService.INSTANCE.getProtocol().sendRecordUwbReq(nameID);
                } else {
                    ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.RESULT_OK);
                    getActivity().finish();
                }
                break;
            case FINGERPRINT_RESULT_INVALID:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.FINGERPRINT_RESULT_INVALID);
                getActivity().finish();
                break;
            case FINGERPRINT_RESULT_DUPLICATE:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.FINGERPRINT_RESULT_DUPLICATE);
                getActivity().finish();
                break;
            case INVALID_PACKET:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.INVALID_PACKET);
                getActivity().finish();
                break;
            default:
                ((RegistrationMainActivity) getActivity()).setResultToActivity(SmartAccessActivity.GENERAL_ERROR);
                getActivity().finish();
                break;
        }

        Log.d(TAG, "-EnrollFingerprintRes");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.RegistrationCMDRes e) {
        Log.d(TAG, "+RegistrationCMDRes");

        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(getActivity());

        Log.d(TAG, " RegistrationCMDRes:" + e.mRegistrationCMDResult);

        SharedPreferences sharedPref = getActivity().getPreferences(Context.MODE_PRIVATE);
        String nameID = sharedPref.getString(fixInputField(mNameEditText.getText().toString()), "");

        switch (e.mRegistrationCMDResult) {
            case REGISTRATION_CMD_RESULT_OK:
                if (isMethodSelected(FINGERPRINT)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_fingerprint_registration));
                    BLEService.INSTANCE.getProtocol().sendEnrollFingerprintReq(nameID);
                } else if (isMethodSelected(NFC)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_nfc_registration));
                    BLEService.INSTANCE.getProtocol().sendRecordNfcReq(nameID);
                } else if (isMethodSelected(UWB)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_uwb_registration));
                    BLEService.INSTANCE.getProtocol().sendRecordUwbReq(nameID);
                } else {
                    ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.RESULT_OK);
                    getActivity().finish();
                }
                break;
            case REGISTRATION_CMD_RESULT_INVALID:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.REGISTRATION_CMD_RESULT_INVALID);
                getActivity().finish();
                break;
            case REGISTRATION_CMD_RESULT_DUPLICATE:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.REGISTRATION_CMD_RESULT_DUPLICATE);
                getActivity().finish();
                break;
            case INVALID_PACKET:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.INVALID_PACKET);
                getActivity().finish();
                break;
            default:
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.GENERAL_ERROR);
                getActivity().finish();
        }

        Log.d(TAG, "-RegistrationCMDRes");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.SetPwdRes e) {
        Log.d(TAG, "+SetPwdRes");

        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(getActivity());

        Log.d(TAG, " SetPwdRes:" + e.mSetPwdResult);

        switch (e.mSetPwdResult) {
            case SET_PWD_RESULT_OK:
                // save (name, event.mSetPwdReceivedID) pair on phone
                SharedPreferences sharedPref = getActivity().getPreferences(Context.MODE_PRIVATE);
                SharedPreferences.Editor editor = sharedPref.edit();
                editor.putString(fixInputField(mNameEditText.getText().toString()), e.mSetPwdReceivedID);
                editor.apply();

                sharedPref = getActivity().getSharedPreferences(getString(R.string.password_shared_pref), Context.MODE_PRIVATE);
                editor = sharedPref.edit();
                editor.putString(e.mSetPwdReceivedID, fixInputField(mPasswordEditText.getEditText().getText().toString()));
                editor.apply();

                if (isMethodSelected(FACE)) {
                    if (mFaceFeature != null) {
                        StatusPopUp.getStatusPopUpInstance().showProgress(
                                getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering));
                        BLEService.INSTANCE.getProtocol().sendRegistrationReq(e.mSetPwdReceivedID, fixInputField(mNameEditText.getText().toString()), mFaceFeature, false);
                    } else {
                        StatusPopUp.getStatusPopUpInstance().showProgress(
                                getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_face_board_registration));
                        BLEService.INSTANCE.getProtocol().sendRegistrationCmdReq(e.mSetPwdReceivedID);
                    }
                } else if (isMethodSelected(FINGERPRINT)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_fingerprint_registration));

                    BLEService.INSTANCE.getProtocol().sendEnrollFingerprintReq(e.mSetPwdReceivedID);
                } else if (isMethodSelected(NFC)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_nfc_registration));

                    BLEService.INSTANCE.getProtocol().sendRecordNfcReq(e.mSetPwdReceivedID);
                } else if (isMethodSelected(UWB)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_uwb_registration));

                    /* Here, additional methods may be called to enable the phone uwb */
                    BLEService.INSTANCE.getProtocol().sendRecordUwbReq(e.mSetPwdReceivedID);
                } else {
                    ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.RESULT_OK);
                    getActivity().finish();
                }
                break;
            case SET_PWD_RESULT_INVALID:
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.SET_PWD_RESULT_INVALID);
                getActivity().finish();
                break;
            case SET_PWD_RESULT_DUPLICATE:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.error_duplicate));
                break;
            case INVALID_PACKET:
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.INVALID_PACKET);
                getActivity().finish();
                break;
            default:
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.GENERAL_ERROR);
                getActivity().finish();
        }

        Log.d(TAG, "-SetPwdRes");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.RegistrationRes e) {
        Log.d(TAG, "+RegistrationRes");

        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(getActivity());

        Log.d(TAG, " RegistrationRes:" + e.mRegistrationResult);

        SharedPreferences sharedPref = getActivity().getPreferences(Context.MODE_PRIVATE);
        String nameID = sharedPref.getString(fixInputField(mNameEditText.getText().toString()), "");

        switch (e.mRegistrationResult) {
            case REGISTRATION_RESULT_OK:
                if (isMethodSelected(FINGERPRINT)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_fingerprint_registration));
                    BLEService.INSTANCE.getProtocol().sendEnrollFingerprintReq(nameID);
                }  else if (isMethodSelected(NFC)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_nfc_registration));
                    BLEService.INSTANCE.getProtocol().sendRecordNfcReq(nameID);
                } else if (isMethodSelected(UWB)) {
                    StatusPopUp.getStatusPopUpInstance().showProgress(
                            getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.helper_uwb_registration));
                    BLEService.INSTANCE.getProtocol().sendRecordUwbReq(nameID);
                } else {
                    // No other method, close
                    ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.RESULT_OK);
                    getActivity().finish();
                }
                break;
            case REGISTRATION_RESULT_INVALID:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.REGISTRATION_RESULT_INVALID);
                getActivity().finish();
                break;
            case REGISTRATION_RESULT_DUPLICATE:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.REGISTRATION_RESULT_DUPLICATE);
                duplicateFaceHandle(e);
                break;
            case INVALID_PACKET:
                BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.INVALID_PACKET);
                getActivity().finish();
                break;
            default:
                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.GENERAL_ERROR);
                getActivity().finish();
        }

        Log.d(TAG, "-RegistrationRes");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.ConnectionTimeout e) {
        Log.d(TAG, "+ConnectionTimeout");

        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(getActivity());

        SharedPreferences sharedPref = getActivity().getPreferences(Context.MODE_PRIVATE);
        String nameID = sharedPref.getString(fixInputField(mNameEditText.getText().toString()), "");
        if (!nameID.equals("") && (getActivity().getClass() == RegistrationMainActivity.class)) {
            StatusPopUp.getStatusPopUpInstance().showProgress(
                    getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering), getString(R.string.delete));
            BLEService.INSTANCE.getProtocol().sendDeleteUserReq(Integer.parseInt(nameID));
        }

        Log.d(TAG, "-ConnectionTimeout");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.DeleteUserRes e) {
        Log.d(TAG, "+DeleteUserInfoRes");
        if (e == null) return;

        StatusPopUp.getStatusPopUpInstance().dismiss(getActivity());

        switch (e.mDeleteUserResult){
            case USER_DETAIL_RESULT_OK:
                SharedPreferences sharedPref = getActivity().getPreferences(Context.MODE_PRIVATE);
                String nameID = sharedPref.getString(fixInputField(mNameEditText.getText().toString()), "");

                int id = Integer.parseInt(nameID);

                databaseWriteExecutor.execute(() -> {
                    userDao.deleteByID(id);
                });

                ((RegistrationMainActivity)getActivity()).setResultToActivity(SmartAccessActivity.REGISTRATION_RESULT_INVALID);
                getActivity().finish();

                break;
            case USER_DETAIL_RESULT_INVALID:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.error_command_sending));
                break;
            case INVALID_PACKET:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.error_invalid_packet));
                break;
            default:
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.error_general));
        }


        Log.d(TAG, "-DeleteUserInfoRes:" + e.mDeleteUserResult);
    }

    public void pressFABCardButton(View view) {
        Log.d(TAG, "+onClickOk");
        if (reRegistration) {
            // re-register branch
        } else {
            String nameText = fixInputField(mNameEditText.getText().toString());
            String passText = fixInputField(mPasswordEditText.getEditText().getText().toString());

            if (!namePattern.matcher(nameText).find()){
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.error_registration_no_name));
                return;
            }

            if (!passwordPattern.matcher(passText).find()){
                StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                        getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.error_incorrect_password));
                return;
            }

            StatusPopUp.getStatusPopUpInstance().showProgress(
                    getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering));

            BLEService.INSTANCE.getProtocol().sendSetPasswordReq(nameText, passText);
        }
    }

    public String fixInputField(String inputField) {
        return inputField.replaceAll("( +)"," ").trim();
    }

    public void setImage(Bitmap image){
        this.mFaceImage = image;
    }

    private void defocusEffect(float alpha){
        WindowManager.LayoutParams lp = getActivity().getWindow().getAttributes();
        lp.alpha = alpha;
        getActivity().getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        getActivity().getWindow().setAttributes(lp);
    }

    private void duplicateFaceHandle(BLEStateEvent.RegistrationRes e) {
        // inflate the layout of the popup window
        duplicateFaceName = e.mRegistrationDuplicateName;

        LayoutInflater inflater = (LayoutInflater)
                getActivity().getSystemService(LAYOUT_INFLATER_SERVICE);
        View popupView = inflater.inflate(R.layout.popup, null);

        mPopupWindow = new PopupWindow(popupView, LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT, true);

        TextView popup_title = mPopupWindow.getContentView().findViewById(R.id.title_popup);
        TextInputEditText user_name_view = mPopupWindow.getContentView().findViewById(R.id.popup_user_detail_name);
        TextInputLayout user_name_layout = mPopupWindow.getContentView().findViewById(R.id.outlinedTextField);
        TextView popup_description = mPopupWindow.getContentView().findViewById(R.id.description_popup);
        TextView popup_question = mPopupWindow.getContentView().findViewById(R.id.question_popup);
        Button buttonOK = mPopupWindow.getContentView().findViewById(R.id.button_confirm_popup);
        buttonOK.setOnClickListener(v -> onClickConfirm(v));
        Button buttonCancel = mPopupWindow.getContentView().findViewById(R.id.button_cancel_popup);
        buttonCancel.setOnClickListener(v -> onClickCancel(v));

        user_name_view.setVisibility(View.GONE);
        user_name_layout.setVisibility(View.GONE);
        user_name_layout.setVisibility(View.GONE);
        popup_description.setVisibility(View.VISIBLE);
        popup_question.setVisibility(View.VISIBLE);

        popup_title.setText(String.format(getString(R.string.registration_duplicate_title), duplicateFaceName));
        popup_description.setText(R.string.registration_duplicate_description);
        popup_question.setText(R.string.registration_duplicate_question);
        buttonOK.setText(R.string.button_update);

        defocusEffect(0.7f);

        mPopupWindow.setOnDismissListener(() -> defocusEffect(1.0f));

        mPopupWindow.setOutsideTouchable(false);
        mPopupWindow.setFocusable(false);
        mPopupWindow.setTouchable(true);
        mPopupWindow.showAtLocation(getActivity().findViewById(R.id.registration_confirm_view), Gravity.CENTER, 0, 0);
    }

    public void onClickConfirm(View view) {
        Log.d(TAG, "+onClickConfirm");

        TextView popup_description= mPopupWindow.getContentView().findViewById(R.id.description_popup);
        StatusPopUp.getStatusPopUpInstance().showProgress(
                getActivity(), getActivity().findViewById(R.id.registration_confirm_view), getString(R.string.state_registering));
        // if re-register, then we have the (name, ID) pair saved on the phone, so take it from there.
        SharedPreferences sharedPref = getActivity().getPreferences(Context.MODE_PRIVATE);
        String duplicateNameID = sharedPref.getString(fixInputField(duplicateFaceName), "");
        BLEService.INSTANCE.getProtocol().sendRegistrationReq(duplicateNameID, fixInputField(duplicateFaceName), mFaceFeature, true);
        mPopupWindow.dismiss();
    }

    public void onClickCancel(View view) {
        Log.d(TAG, "+onClickCancel");
        mPopupWindow.dismiss();
        getActivity().finish();
    }
}
