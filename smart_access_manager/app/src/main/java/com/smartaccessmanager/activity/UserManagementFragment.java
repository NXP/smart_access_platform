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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.smartaccessmanager.R;
import com.smartaccessmanager.database.AppDatabase;
import com.smartaccessmanager.database.User;
import com.smartaccessmanager.database.UserDao;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.service.BLEService;
import com.smartaccessmanager.utility.SdkUtils;
import com.smartaccessmanager.utility.StatusPopUp;

import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.jetbrains.annotations.NotNull;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import static android.app.Activity.RESULT_OK;

public class UserManagementFragment extends Fragment {

    private static final String TAG = "SLM_RA";

    public static final String USER_DETAIL_RETURN = "intent.user.detail";
    public static final String NEW_USER_NAME = "intent.user.name";
    public static final int UPDATE_USER = 0;
    public static final int DELETE_USER = 1;

    public static final int INVALID_PACKET = -2;
    public static final int NO_USERS_AVAILABLE = 0;

    public final int REQUEST_CODE_USER_DETAIL = 300;

    UserManagementAdapter adapter;
    private RecyclerView mRecyclerView;
    private UserDao userDao;
    List<User> mUsers = new ArrayList<>();

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.GetUserCountRes e) {
        Log.d(TAG, "+GetUserCountRes");

        if (e == null) return;

        Log.d(TAG, "Users:" + e.mGetUserCountNumber);

        SmartAccessActivity.userCount = e.mGetUserCountNumber;
        BLEService.INSTANCE.getProtocol().sendGetUserInfoReq();

        Log.d(TAG, "-GetUserCountRes:" + e.mGetUserCountResult);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(getActivity());
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.GetUserInfoRes e) {
        Log.d(TAG, "+GetUserInfoRes");
        if (e == null || getActivity() == null) return;

        if (e.mGetUserInfoResult <= INVALID_PACKET) {
            StatusPopUp.getStatusPopUpInstance().showErrorPopUp(
                    getActivity(), getView(), getString(R.string.error_invalid_packet));
        }

        if (e.mGetUserInfoCount > 0){
            AppDatabase.databaseWriteExecutor.execute(() -> {
                // Populate the database in the background.
                userDao.insertAll(e.mGetUserInfoData);
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        mUsers.clear();
                        mUsers.addAll(e.mGetUserInfoData);
                        updateUserList();
                        adapter.notifyDataSetChanged();
                    }
                });
            });
        }else if(e.mGetUserInfoCount == NO_USERS_AVAILABLE){
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mUsers.clear();
                    updateUserList();
                    adapter.notifyDataSetChanged();
                }
            });
        }

        ((SmartAccessActivity) requireActivity()).isSyncing = false;
        SdkUtils.changeToolbarFABButtonState(getActivity(), R.string.menu_scan_stop, R.string.sync, ((SmartAccessActivity)getActivity()).isSyncing);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_user_management, container, false);

        EventBus.getDefault().register(this);

        // Lookup the recyclerview in activity layout
        mRecyclerView = (RecyclerView) view.findViewById(R.id.user_management_list);
        mRecyclerView.setHasFixedSize(true);
        mRecyclerView.setAdapter(adapter);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(getActivity()));

        return view;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, @Nullable @org.jetbrains.annotations.Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == ((SmartAccessActivity)getActivity()).REQUEST_CODE_REGISTRATION && resultCode == RESULT_OK) {
            ((SmartAccessActivity)getActivity()).userChange = true;
        } else if(requestCode == REQUEST_CODE_USER_DETAIL) {
            if (data != null) {
                getActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if(data.getIntExtra(USER_DETAIL_RETURN, 0) == DELETE_USER) {
                            mUsers.remove(resultCode);
                            updateUserList();
                            adapter.notifyItemRemoved(resultCode);
                        } else if (data.getIntExtra(USER_DETAIL_RETURN, 0) == UPDATE_USER){
                            mUsers.get(resultCode).name = data.getStringExtra(NEW_USER_NAME);
                            adapter.notifyItemChanged(resultCode);
                        }
                    }
                });
            }
        }
    }

    @Override
    public void onConfigurationChanged(@NonNull @NotNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        requestUserInfo();
    }

    @Override
    public void onResume() {
        super.onResume();

        if (((SmartAccessActivity) getActivity()).userChange){
            ((SmartAccessActivity) getActivity()).userChange = false;
            requestUserInfo();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        AppDatabase db = AppDatabase.getDatabase(getActivity());
        userDao = db.userDao();

        adapter = new UserManagementAdapter(mUsers);
    }

    private void updateUserList(){
        RecyclerView userList = getView().findViewById(R.id.user_management_list);
        LinearLayout placeholder = getView().findViewById(R.id.no_users_placeholder);

        if (adapter.getItemCount() == 0){
            userList.setVisibility(View.GONE);
            placeholder.setVisibility(View.VISIBLE);
        } else {
            userList.setVisibility(View.VISIBLE);
            placeholder.setVisibility(View.GONE);
        }
    }

    private void requestUserInfo(){
        SdkUtils.changeToolbarFABButtonState(requireActivity(), R.string.menu_scan_stop, R.string.sync, ((SmartAccessActivity)getActivity()).isSyncing);

        BLEService.INSTANCE.getProtocol().sendGetUserCountReq();
    }

    private final View.OnClickListener mAppClickHandler = new View.OnClickListener() {
        @Override
        public void onClick(View v) {

            int itemPosition = mRecyclerView.getChildAdapterPosition(v);

            Intent pendingIntent = new Intent();
            pendingIntent.setClass(getActivity(), UserDetailActivity.class);

            pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_PHY, getActivity().getIntent().getExtras().getInt(BaseServiceActivity.INTENT_KEY_PHY));
            pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_ADDRESS, getActivity().getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_ADDRESS));
            pendingIntent.putExtra(BaseServiceActivity.INTENT_KEY_NAME, getActivity().getIntent().getExtras().getString(BaseServiceActivity.INTENT_KEY_NAME));
            pendingIntent.putExtra(getString(R.string.intent_user_id), mUsers.get(itemPosition).getID());
            pendingIntent.putExtra(getString(R.string.intent_user_name), mUsers.get(itemPosition).getName());
            pendingIntent.putExtra(getString(R.string.intent_user_position), itemPosition);

            getActivity().startActivityForResult(pendingIntent, REQUEST_CODE_USER_DETAIL);
        }
    };

    public class UserManagementAdapter extends RecyclerView.Adapter<UserManagementAdapter.ViewHolder> {

        // Provide a direct reference to each of the views within a data item
        // Used to cache the views within the item layout for fast access
        public class ViewHolder extends RecyclerView.ViewHolder {
            // Your holder should contain a member variable
            // for any view that will be set as you render a row
            // We also create a constructor that accepts the entire item row
            // and does the view lookups to find each subview
            public TextView nameTextView;
            public TextView idTextView;
            public ImageView iconBG;
            public TextView iconText;

            public ViewHolder(View itemView) {
                // Stores the itemView in a public final member variable that can be used
                // to access the context from any ViewHolder instance.
                super(itemView);
                nameTextView = (TextView) itemView.findViewById(R.id.user_name);
                idTextView = (TextView) itemView.findViewById(R.id.user_id);
                iconBG = (ImageView) itemView.findViewById(R.id.person_icon_bg);
                iconText = (TextView) itemView.findViewById(R.id.person_icon_text);

            }
        }

        private List<User> mListUsers = null;

        // Pass in the users array into the constructor
        public UserManagementAdapter(List<User> users){
                mListUsers = users;
        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            Context context = parent.getContext();
            LayoutInflater inflater = LayoutInflater.from(context);

            // Inflate the custom layout
            View userView = inflater.inflate(R.layout.user_management_item, parent, false);
            userView.setOnClickListener(mAppClickHandler);

            return new ViewHolder(userView);
        }

        @Override
        public void onBindViewHolder(UserManagementAdapter.ViewHolder holder, int position) {
            // Get the data model based on position
            User user = mListUsers.get(position);

            // Set item views based on your views and data model
            TextView nameTextView = holder.nameTextView;
            nameTextView.setText(user.getName());

            TextView idTextView = holder.idTextView;
            idTextView.setText(String.valueOf(user.getID()) + ":");

            Random rnd = new Random();
            int color = Color.argb(212, rnd.nextInt(256), rnd.nextInt(256), rnd.nextInt(256));
            holder.iconBG.setColorFilter(color, android.graphics.PorterDuff.Mode.MULTIPLY);

            holder.iconText.setText(user.getID()+"");
        }

        @Override
        public int getItemCount() {
            return mListUsers.size();
        }
    }


}