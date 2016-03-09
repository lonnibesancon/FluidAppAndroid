package fr.limsi.ARViewer;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.lang.Runnable;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.Enumeration;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

public abstract class BaseARActivity extends Activity
    implements // Camera.PreviewCallback,
    SurfaceHolder.Callback
{
    private static final String TAG = Config.APP_TAG;
    private static final boolean DEBUG = Config.DEBUG;

    private ARSurfaceView mView;
    // private Camera mCamera;
    // protected Camera mCamera;
    private boolean mDestroyed = false;

    // Native interface
    private static boolean initialized = false;
    protected static NativeApp.Settings settings = new NativeApp.Settings();
    private static NativeApp.State state = new NativeApp.State();

    // Network server
    private Thread mServerThread;
    private ServerSocket mServerSocket;
    private Socket mClient;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        if (DEBUG)
            enableStrictMode();

        super.onCreate(savedInstanceState);

        // Make window fullscreen + disable screen blanking
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN
                             | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // try {
        //     initCamera();
        // } catch (Exception e) {
        //     return;
        // }

        if (!initialized) {
            // // Camera calibration parameters
            // copyAssetsFileToStorage("tablet_camera_para.dat", false);
            //
            // // Tangible markers
            // for (int i : new int[] {2, 4, 5, 6, 7, 52, 57, 59, 65, 244, 260, 397, 540, 929})
            //     copyAssetsFileToStorage(i + ".patt", false);
            //
            // // Stylus markers
            // for (int i : new int[] {26, 15, 19, 24, 25})
            //     copyAssetsFileToStorage(i + ".patt", false);
            //
            // // Camera.Parameters parameters = mCamera.getParameters();
            // // Camera.Size previewSize = parameters.getPreviewSize();

            // NativeApp.init(getAppType(), getStorageDir(), previewSize.width, previewSize.height);
            // NativeApp.init(getAppType(), getStorageDir(), 640, 480); // XXX: hardcoded preview size
            NativeApp.init(getAppType(), getStorageDir()); //, 800, 480); // XXX: hardcoded preview size

            initialized = true;
        }

        NativeApp.getSettings(settings);
        NativeApp.getState(state);
    }

    protected abstract int getAppType();

    protected static boolean isInitialized() {
        return initialized;
    }

    // protected NativeApp.Settings getSettings() {
    //     return settings;
    // }

    // protected void setSettings(NativeApp.Settings s) {
    //     NativeApp.setSettings(s);
    // }

    protected NativeApp.State getState() {
        NativeApp.getState(state);
        return state;
    }

    // protected boolean isCameraAvailable() {
    //     // return (mCamera != null);
    //     return true;
    // }

    protected void setView(ARSurfaceView view) {
        if (mView != null)
            mView.getHolder().removeCallback(this);
        mView = view;
        mView.getHolder().addCallback(this);
    }

    protected void startServer(int port) {
        if (mServerThread == null) {
            mServerThread = new Thread(new ServerThread(port));
            mServerThread.start();
        } else {
            Log.w(TAG, "Server thread already started!");
        }
    }

    // private void initCamera() throws RuntimeException, IOException {
	// 	try {
    //         mCamera = Camera.open();
	// 	} catch (RuntimeException e) {
    //         if (DEBUG) Log.e(TAG, "Error: Camera.open()", e);
    //         errorMessage("Unable to open the camera.");
    //         throw e;
	// 	}

    //     // if (mSwitchRecordingMode) {
    //     //     mSwitchRecordingMode = false;
    //     //     setRecordingHint(!recordingHint);
    //     // } else if (recordingHint) {
    //     //     setRecordingHint(true);
    //     // }

    //     Camera.Parameters parameters = mCamera.getParameters();

    //     parameters.setPreviewFrameRate(30);
    //     // parameters.setPreviewSize(1280, 720);
    //     // parameters.setPreviewSize(800, 480); // (*)
    //     parameters.setPreviewSize(640, 480); // NOTE: the Vuforia SDK seems to only support 4:3 modes
    //     // parameters.setPreviewSize(240, 160);

    //     // Request the NV21 format (needed for the optimized native
    //     // conversion code). The NV21 format is guaranteed to be
    //     // supported.
    //     parameters.setPreviewFormat(ImageFormat.NV21);

    //     // for (Integer format : parameters.getSupportedPreviewFormats()) {
    //     //     Log.d(TAG, "supported format: " + format);
    //     // }

    //     try {
    //         mCamera.setParameters(parameters);
	// 	} catch (RuntimeException e) {
    //         closeCamera();
    //         if (DEBUG) Log.e(TAG, "Error: Camera.setParameters()", e);
    //         errorMessage("Error setting camera parameters.");
    //         throw e;
	// 	}

    //     try {
    //         // Either a preview display or a preview texture must be
    //         // set for the "onPreviewFrame" callback to work
    //         // ("security" measure?)
    //         mCamera.setPreviewTexture(new SurfaceTexture(-1));
    //     } catch (IOException e) {
    //         closeCamera();
    //         if (DEBUG) Log.e(TAG, "Error: Camera.setPreviewTexture()", e);
    //         errorMessage("Unable to render the camera view.");
    //         throw e;
    //     }
    // }

    // private void startCamera() {
    //     if (mCamera == null) {
    //         try {
    //             initCamera();
    //         } catch (Exception e) {
    //             return;
    //         }
    //     }

    //     try {
    //         mCamera.startPreview();
    //     } catch (RuntimeException e) {
    //         closeCamera();
    //         if (DEBUG) Log.e(TAG, "Error: Camera.startPreview()", e);
    //         errorMessage("Unable to start camera preview.");
    //         throw e;
    //     }

    //     // if (mPreviewBuffers != null) {
    //     //     mCamera.setPreviewCallbackWithBuffer(this);
    //     //     mCamera.addCallbackBuffer(mPreviewBuffers[0]);
    //     // } else {
    //         mCamera.setPreviewCallback(this);
    //     // }
    // }

    // private void closeCamera() {
    //     if (mCamera != null) {
    //         mCamera.stopPreview();
    //         mCamera.setPreviewCallback(null);
    //         mCamera.release();
    //         mCamera = null;
    //     }
    // }

    protected void onFrameProcessed() {
        //
    }

    // protected void processVideoFrame(final byte[] data, Camera camera) {
    //     NativeApp.processVideoFrame(data);
    // }

    // @Override
	// public void onPreviewFrame(final byte[] data, Camera camera) {
    //     // Log.d(TAG, "onPreviewFrame");
    //
    //     // Log.d(TAG, "onPreviewFrame " + mDestroyed + " " + camera + " " + mCamera + " " + mView);
    //     if (mDestroyed /*|| camera != mCamera*/ || mView == null)
    //         return;
    //
    //     processVideoFrame(data,mCamera);
    //     mView.requestRender();
    //
    //     // int oldFocusDist = state.focusDist;
    //     // NativeApp.getState(state);
    //
    //     // if (state.focusDist != oldFocusDist) {
    //     //     switch (state.focusDist) {
    //     //         case NativeApp.FOCUS_NEAR:
    //     //             Log.d(TAG, "refocus needed (near)");
    //     //             break;
    //     //         case NativeApp.FOCUS_FAR:
    //     //             Log.d(TAG, "refocus needed (far)");
    //     //             break;
    //     //     }
    //     //     // TODO: call Camera.Parameters.setFocusArea() to make
    //     //     // sure the focus point is on the reference object, and not
    //     //     // somewhere else in the background
    //     //     mCamera.autoFocus(new AutoFocusCallback() {
    //     //         public void onAutoFocus(boolean autoFocusSuccess, Camera arg1) {}
    //     //     });
    //     // }
    //
    //     onFrameProcessed();
    //
    //     // if (mPreviewBuffers != null) {
    //     //     // Double-buffering
    //     //     mCamera.addCallbackBuffer(mPreviewBuffers[data == mPreviewBuffers[0] ? 1 : 0]);
    //     // }
    // }

    @Override
    protected void onPause() {
        super.onPause();
        if (mView != null)
            mView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mView != null)
            mView.onResume();
    }

    @Override
    protected void onDestroy() {
    	super.onDestroy();
        mDestroyed = true;
    }

    @Override
	protected void onStop() {
        super.onStop();
        if (mServerSocket != null) {
            try {
                Log.i(TAG, "Closing the server socket");
                mServerSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "Error while closing the server socket:", e);
            }
        }
        if (mClient != null) {
            try {
                Log.i(TAG, "Disconnecting client");
                mClient.close();
            } catch (IOException e) {
                Log.e(TAG, "Error while closing the client socket:", e);
            }
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // startCamera();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        //
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        // closeCamera();
    }

    protected String getStorageDir() {
        return getExternalFilesDir(null).getAbsolutePath();
    }

    // From: KiwiViewerActivity.java
    // NOTE: requires the WRITE_EXTERNAL_STORAGE permission
    protected String copyAssetsFileToStorage(String fileName, boolean force) {
        String storageDir = getStorageDir();

        String destFilename = storageDir + "/" + fileName;

        File destFile = new File(destFilename);
        if (destFile.exists() && !force)
            return destFilename;

        if (DEBUG)
            Log.d(TAG, "Copying " + fileName + " to " + destFilename);

        InputStream in = null;
        OutputStream out = null;
        try {
            in = getAssets().open(fileName);
            out = new FileOutputStream(destFilename);
            byte[] buffer = new byte[1024];
            int read;
            while ((read = in.read(buffer)) != -1)
                out.write(buffer, 0, read);
            in.close();
            in = null;
            out.flush();
            out.close();
            out = null;
        } catch (Exception e) {
            Log.e(TAG, "Error copying file: " + e.getMessage());
        }

        return destFilename;
    }

    private static void enableStrictMode() {
        // Requires API level >= 11
        android.os.StrictMode.setThreadPolicy(
            new android.os.StrictMode.ThreadPolicy.Builder()
            .detectAll()
            .permitDiskReads()
            .permitDiskWrites()
            .penaltyLog()
            .build());
        android.os.StrictMode.setVmPolicy(
            new android.os.StrictMode.VmPolicy.Builder()
            // .detectAll()
            .detectLeakedClosableObjects()
            .detectLeakedSqlLiteObjects()
            .setClassInstanceLimit(BaseARActivity.class, 1) // 2 ??
            .penaltyLog()
            .build());
    }

    protected void errorMessage(String msg) {
        new AlertDialog.Builder(this)
            .setTitle("Error")
            .setMessage(msg)
            .setCancelable(false)
            .setNeutralButton("OK", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int button) {
                    BaseARActivity.this.finish();
                }
            })
            .show();
    }

    protected void handleNetworkCommand(String cmd, PrintWriter out) {
        Log.d(TAG, "Received network command: \"" + cmd + "\"");
    }

    // Based on: http://thinkandroid.wordpress.com/2010/03/27/incorporating-socket-programming-into-your-applications/
    private String getLocalIpAddress() {
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces();
                 en.hasMoreElements(); )
            {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses();
                     enumIpAddr.hasMoreElements(); )
                {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()
                        // FIXME: hack
                        && !inetAddress.getHostAddress().toString().startsWith("fe80:"))
                    {
                        return inetAddress.getHostAddress().toString();
                    }
                }
            }
        } catch (SocketException ex) {
            Log.e(TAG, ex.toString());
        }
        return null;
    }

    // Based on: http://thinkandroid.wordpress.com/2010/03/27/incorporating-socket-programming-into-your-applications/
    // TODO: WifiManager.WifiLock (to prevent the wireless connection from turning off)
    class ServerThread implements Runnable {
        final int mPort;

        ServerThread(int port) {
            mPort = port;
        }

        public void run() {
            try {
                String localAddress = getLocalIpAddress();

                // Fall back to the loopback address
                if (localAddress == null)
                    localAddress = InetAddress.getLocalHost().getHostAddress();

                Log.i(TAG, "Listening on " + localAddress + ":" + mPort);

                mServerSocket = new ServerSocket();
                mServerSocket.setReuseAddress(true);
                mServerSocket.bind(new InetSocketAddress(mPort));

                while (true) {
                    mClient = mServerSocket.accept();
                    Log.i(TAG, "New connection from " + mClient.getInetAddress());

                    try {
                        BufferedReader in = new BufferedReader(new InputStreamReader(mClient.getInputStream()));
                        PrintWriter out = new PrintWriter(mClient.getOutputStream(), true);
                        out.println("Connection established.");
                        String line = null;
                        while ((line = in.readLine()) != null) {
                            if (mServerSocket.isClosed()) {
                                Log.e(TAG, "The server socket is already closed");
                                break;
                            }
                            handleNetworkCommand(line, out);
                        }
                    } catch (Exception e) {
                        if (!mClient.isClosed())
                            Log.e(TAG, "Connection interrupted:", e);
                    }

                    if (!mClient.isClosed())
                        Log.d(TAG, "Client " + mClient.getInetAddress() + " disconnected");

                    mClient = null;
                }

            } catch (Exception e) {
                if (mServerSocket == null || !mServerSocket.isClosed())
                    Log.e(TAG, "Network error:", e);
            }
        }
    }
}
