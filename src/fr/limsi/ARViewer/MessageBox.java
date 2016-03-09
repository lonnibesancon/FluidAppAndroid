package fr.limsi.ARViewer;

import java.io.*;
import java.util.*;

import android.app.*;
import android.content.*;
import android.hardware.*;
import android.hardware.Camera;
import android.graphics.*;
import android.os.*;
import android.util.*;
import android.view.*;
import android.widget.*;

public class MessageBox {
    private static final String TAG = Config.APP_TAG;
    private static final boolean DEBUG = Config.DEBUG;

    public static void error(final Activity activity, String msg) {
        new AlertDialog.Builder(activity)
            .setTitle("Error")
            .setMessage(msg)
            .setCancelable(false)
            .setNeutralButton("OK", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int button) {
                    activity.finish();
                }
            })
            .show();
    }
}
