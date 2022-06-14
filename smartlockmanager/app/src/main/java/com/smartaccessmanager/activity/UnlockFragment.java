package com.smartaccessmanager.activity;

import static com.smartaccessmanager.activity.SmartAccessActivity.NFC;
import static com.smartaccessmanager.activity.SmartAccessActivity.NFC_SUPPORT;
import static com.smartaccessmanager.activity.SmartAccessActivity.PASSWORD;
import static com.smartaccessmanager.activity.SmartAccessActivity.UWB;
import static com.smartaccessmanager.activity.SmartAccessActivity.UWB_SUPPORT;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.smartaccessmanager.R;
import com.smartaccessmanager.UnlockMethod;
import com.smartaccessmanager.event.BLEStateEvent;

import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.List;

public class UnlockFragment extends Fragment {

    UnlockAdapter adapter;
    private RecyclerView mRecyclerView;

    private List<UnlockMethod> unlockMethodList = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        unlockMethodList = new ArrayList<>();
        unlockMethodList.add(new UnlockMethod(getString(R.string.password_unlock),
                        getString(R.string.password_unlock_detail),
                        R.drawable.ic_baseline_lock_24,
                        PassUnlockActivity.class, PASSWORD));

        if(NFC_SUPPORT)
            unlockMethodList.add(new UnlockMethod(getString(R.string.nfc_unlock),
                            getString(R.string.nfc_unlock_detail),
                            R.drawable.ic_baseline_nfc_24,
                            NfcUnlockActivity.class, NFC));

        if(UWB_SUPPORT)
            unlockMethodList.add(new UnlockMethod(getString(R.string.uwb_unlock),
                            getString(R.string.uwb_unlock_detail),
                            R.drawable.ic_uwb_lock_icon,
                            UwbUnlockActivity.class, UWB));

        adapter = new UnlockAdapter(unlockMethodList);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(getActivity());
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_unlock, container, false);

        // Lookup the recyclerview in activity layout
        mRecyclerView = (RecyclerView) view.findViewById(R.id.unlock_option_list);
        mRecyclerView.setHasFixedSize(true);
        mRecyclerView.setAdapter(adapter);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(getActivity()));

        return view;
    }


    private final View.OnClickListener mAppClickHandler = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            int itemPosition = mRecyclerView.getChildAdapterPosition(v);

            Intent pendingIntent = new Intent();
            pendingIntent.setClass(getActivity(), unlockMethodList.get(itemPosition).intentClass);

            getActivity().startActivity(pendingIntent);
        }
    };

    public class UnlockAdapter extends RecyclerView.Adapter<UnlockAdapter.ViewHolder> {

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

        // Pass in the users array into the constructor
        public UnlockAdapter(List<UnlockMethod> unlockMethods){
            mUnlockMethods = unlockMethods;
        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            Context context = parent.getContext();
            LayoutInflater inflater = LayoutInflater.from(context);

            // Inflate the custom layout
            View unlockOptionView = inflater.inflate(R.layout.unlock_option_item, parent, false);
            unlockOptionView.setOnClickListener(mAppClickHandler);

            return new ViewHolder(unlockOptionView);
        }

        @Override
        public void onBindViewHolder(UnlockAdapter.ViewHolder holder, int position) {
            // Set item views based on your views and data model
            holder.titleTextView.setText(mUnlockMethods.get(position).title);
            holder.detailTextView.setText(String.valueOf(mUnlockMethods.get(position).description));
            holder.icon.setImageDrawable(ContextCompat.getDrawable(getContext(), mUnlockMethods.get(position).icon));
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
}