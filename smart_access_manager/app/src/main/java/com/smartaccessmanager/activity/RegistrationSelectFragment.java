package com.smartaccessmanager.activity;

import static com.smartaccessmanager.activity.SmartAccessActivity.FACE;
import static com.smartaccessmanager.activity.SmartAccessActivity.FACE_SUPPORT;
import static com.smartaccessmanager.activity.SmartAccessActivity.FINGERPRINT;
import static com.smartaccessmanager.activity.SmartAccessActivity.FINGERPRINT_SUPPORT;
import static com.smartaccessmanager.activity.SmartAccessActivity.NFC;
import static com.smartaccessmanager.activity.SmartAccessActivity.NFC_SUPPORT;
import static com.smartaccessmanager.activity.SmartAccessActivity.PASSWORD;
import static com.smartaccessmanager.activity.SmartAccessActivity.UWB;
import static com.smartaccessmanager.activity.SmartAccessActivity.UWB_SUPPORT;

import androidx.annotation.NonNull;
import androidx.cardview.widget.CardView;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.android.material.checkbox.MaterialCheckBox;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.smartaccessmanager.R;
import com.smartaccessmanager.event.BLEStateEvent;
import com.smartaccessmanager.service.BLEService;
import com.smartaccessmanager.utility.StatusPopUp;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.List;

public class RegistrationSelectFragment extends Fragment {

    private static final String TAG = "SLM_RCA";

    class UnlockType
    {
        String title;
        int icon;
        boolean checked;
        int tag;

        public UnlockType(String title, int icon, boolean checked, int tag)
        {
            this.title = title;
            this.icon = icon;
            this.checked = checked;
            this.tag = tag;
        }

        public boolean isChecked(){
            return checked;
        }
    }

    UnlockAdapter adapter;
    private RecyclerView mRecyclerView;

    private List<UnlockType> unlockTypeList = null;
    private ArrayList<Integer> selectedMethods = null;

    AlertDialog faceRegistrationSourceDialog;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        unlockTypeList = new ArrayList<UnlockType>();
//        unlockTypeList.add(new UnlockType(getString(R.string.password_unlock),
//                R.drawable.ic_baseline_lock_24, true, PASSWORD));

        if(FACE_SUPPORT)
            unlockTypeList.add(new UnlockType(getString(R.string.face_unlock),
                    R.drawable.ic_baseline_face_24, false, FACE));

        if(FINGERPRINT_SUPPORT)
            unlockTypeList.add(new UnlockType(getString(R.string.fingerprint_unlock),
                    R.drawable.ic_baseline_fingerprint_24, false, FINGERPRINT));

        if(NFC_SUPPORT)
            unlockTypeList.add(new UnlockType(getString(R.string.nfc_unlock),
                    R.drawable.ic_baseline_nfc_24, false, NFC));

        if(UWB_SUPPORT)
            unlockTypeList.add(new UnlockType(getString(R.string.uwb_unlock),
                    R.drawable.ic_uwb_lock_icon, false, UWB));

        adapter = new UnlockAdapter(unlockTypeList);
    }

    @Override
    public void onResume() {
        super.onResume();
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

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(BLEStateEvent.Disconnected e) {
        NoConnectionActivity.jumpToDisconnectActivity(getActivity());
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_registration_select, container, false);

        EventBus.getDefault().register(this);

        ((TextView) getActivity().findViewById(R.id.toolbar_title)).setText(getString(R.string.app_registration_select));
        ((TextView) getActivity().findViewById(R.id.toolbar_subtitle)).setText("");

        CircularProgressIndicator circularProgressIndicator = getActivity().findViewById(R.id.toolbar_loading);
        circularProgressIndicator.setVisibility(View.GONE);

        TextView fab_name = getActivity().findViewById(R.id.toolbar_loading_text);
        ImageView fab_image = getActivity().findViewById(R.id.toolbar_loading_static);

        fab_image.setImageDrawable(ContextCompat.getDrawable(getContext(),R.drawable.ic_baseline_arrow_forward_24));
        fab_image.setVisibility(View.VISIBLE);
        fab_name.setText(getString(R.string.next));

        CardView register_button = getActivity().findViewById(R.id.register_fab_cardview);
        register_button.setVisibility(View.VISIBLE);
        register_button.setOnClickListener(v -> pressFABCardButton(v));

        mRecyclerView = view.findViewById(R.id.unlock_select_list);
        mRecyclerView.setHasFixedSize(true);
        mRecyclerView.setAdapter(adapter);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(getActivity()));

        createMessageBox(view);

        return view;
    }

    private void createMessageBox(View view) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this.getActivity());
        builder.setMessage("On which device's camera do you want to register?");
//        builder.setTitle("Alert !");
//        builder.setCancelable(false);

        builder.setPositiveButton( "Phone", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which)
            {
                loadRegistrationFragment(new RegistrationCameraFragment());
                dialog.cancel();
            }
        });

        builder.setNegativeButton( "Board", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which)
            {
                loadRegistrationFragment(new RegistrationConfirmFragment());
                dialog.cancel();
            }
        });

        faceRegistrationSourceDialog = builder.create();
    }

    private void loadRegistrationFragment(Fragment regFrag) {
        Bundle args = new Bundle();
        args.putIntegerArrayList(BaseServiceActivity.INTENT_KEY_METHODS, selectedMethods);
        regFrag.setArguments(args);
        getActivity().getSupportFragmentManager().beginTransaction()
                .setReorderingAllowed(true)
                .replace(R.id.fragment_container_view, regFrag, null)
                .commit();
    }

    private void pressFABCardButton(View v) {
        Log.d(TAG, "+onClickOk");

        selectedMethods = new ArrayList<>();
        selectedMethods.add(PASSWORD);
        for (UnlockType method : unlockTypeList) {
            if (method.isChecked()) {
                selectedMethods.add(method.tag);
            }
        }

        if(false /* selectedMethods.contains(FACE) */) {
            faceRegistrationSourceDialog.show();
        } else {
            loadRegistrationFragment(new RegistrationConfirmFragment());
        }
    }

    private final View.OnClickListener mAppClickHandler = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            int itemPosition = mRecyclerView.getChildAdapterPosition(v);
            /* If we press an already checked button, toggle it to false */
            if (unlockTypeList.get(itemPosition).isChecked()) {
                unlockTypeList.get(itemPosition).checked = false;
                adapter.notifyItemChanged(itemPosition);
            } else {
                /* If we are empty or press another button, check only the one pressed and disable all other */
                for (int i = 0; i < unlockTypeList.size(); i++) {
                    unlockTypeList.get(i).checked = i == itemPosition;
                    adapter.notifyItemChanged(i);
                }
            }
        }
    };

    public class UnlockAdapter extends RecyclerView.Adapter<UnlockAdapter.ViewHolder> {

        public class ViewHolder extends RecyclerView.ViewHolder {

            public TextView titleTextView;
            public ImageView icon;
            public MaterialCheckBox checkBox;

            public ViewHolder(@NonNull View itemView) {
                super(itemView);
                titleTextView = itemView.findViewById(R.id.unlock_title);
                icon = itemView.findViewById(R.id.button_icon_unlock);
            }
        }

        private List<UnlockType> mUnlockTypes = null;

        // Pass in the users array into the constructor
        public UnlockAdapter(List<UnlockType> unlockTypes){
            mUnlockTypes = unlockTypes;
        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            Context context = parent.getContext();
            LayoutInflater inflater = LayoutInflater.from(context);

            // Inflate the custom layout
            View unlockOptionView = inflater.inflate(R.layout.unlock_select_item, parent, false);
            unlockOptionView.setOnClickListener(mAppClickHandler);

            return new ViewHolder(unlockOptionView);
        }

        @Override
        public void onBindViewHolder(ViewHolder holder, int position) {
            // Set item views based on your views and data model
            holder.titleTextView.setText(mUnlockTypes.get(position).title);
            holder.icon.setImageDrawable(ContextCompat.getDrawable(getContext(), mUnlockTypes.get(position).icon));

            if (mUnlockTypes.get(position).checked) {
                holder.itemView.setBackgroundColor(getResources().getColor(R.color.grey));
                holder.icon.setScaleX(1.4f);
                holder.icon.setScaleY(1.4f);
            } else {
                holder.itemView.setBackgroundColor(getResources().getColor(R.color.white));
                holder.icon.setScaleX(1.0f);
                holder.icon.setScaleY(1.0f);
            }
        }

        @Override
        public int getItemCount() {
            return mUnlockTypes.size();
        }
    }
}