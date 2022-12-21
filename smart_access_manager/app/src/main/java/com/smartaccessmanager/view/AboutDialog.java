/**
 * Copyright 2021 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

/**
 * Copyright 2016 Freescale Semiconductors, Inc.
 */

package com.smartaccessmanager.view;

import android.content.Context;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.widget.TextView;

import com.afollestad.materialdialogs.MaterialDialog;
import com.smartaccessmanager.BuildConfig;
import com.smartaccessmanager.R;
import com.smartaccessmanager.utility.Algorithm;

import java.util.Locale;

import butterknife.BindView;
import butterknife.ButterKnife;

public class AboutDialog extends MaterialDialog {

    @BindView(R.id.about_info)
    TextView mAppInfo;

    public AboutDialog(Builder builder) {
        super(builder);
    }

    public static AboutDialog newInstance(Context context) {
        Builder builder = new Builder(context).customView(R.layout.about, true);
        AboutDialog dialog = new AboutDialog(builder);
        dialog.getWindow().setBackgroundDrawable(new ColorDrawable(android.graphics.Color.TRANSPARENT));
        return dialog;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ButterKnife.bind(this);
        Algorithm algorithm = new Algorithm();

        mAppInfo.setText(String.format(Locale.getDefault(),
                "Release Version\n%s", BuildConfig.VERSION_NAME));
    }
}