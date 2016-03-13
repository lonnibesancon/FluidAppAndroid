package fr.limsi.ARViewer;

import java.io.PrintWriter;

import android.app.AlertDialog;
import android.content.res.Resources;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.ColorDrawable;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.Log;
import android.util.TypedValue;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

import android.app.*;
import android.bluetooth.*;
import android.content.*;
import android.graphics.*;
import android.hardware.*;
import android.hardware.Camera;
import android.hardware.usb.*;
import android.os.*;
import android.util.*;
import android.view.*;
import android.widget.*;
import java.io.*;
import java.net.*;
import java.lang.*;
import java.util.*;
import android.view.View.OnClickListener;
import java.lang.Object;

import java.text.DecimalFormat ;

import com.google.atap.tangoservice.*;



// import com.qualcomm.QCAR.QCAR;
// // import com.qualcomm.ar.pl.CameraPreview;
// import com.lannbox.rfduinotest.RFduinoService;

public class MainActivity extends BaseARActivity
 implements View.OnTouchListener, GestureDetector.OnDoubleTapListener, Tango.OnTangoUpdateListener, SensorEventListener, InteractionMode, OnClickListener //, BluetoothAdapter.LeScanCallback //, View.OnLongClickListener
    // , CameraPreview.SizeCallback
{
    private static final String TAG = Config.APP_TAG;
    private static final boolean DEBUG = Config.DEBUG;

    private Button tangibleBtn ;

    // FIXME: static?
    private static FluidMechanics.Settings fluidSettings = new FluidMechanics.Settings();
    private static FluidMechanics.State fluidState = new FluidMechanics.State();

    private FluidMechanics.Settings tempSettings = new FluidMechanics.Settings();

    // private static boolean mFluidDataLoaded = false;
    private static int mDataSet = 0;
    private static boolean mDatasetLoaded = false;
    private static boolean mVelocityDatasetLoaded = false;

    // Menu
    private static boolean menuInitialized = false;
    private MenuItem mAxisClippingMenuItem, mStylusClippingMenuItem;

    // Pinch zoom
    private float mInitialPinchDist = 0;
    private float mInitialZoomFactor = 0;
    private boolean mZoomGesture = false;

    // Gestures
    private GestureDetector mGestureDetector;

    private ARSurfaceView mView;

    private Tango mTango;
    private TangoConfig mConfig;
    private boolean mIsTangoServiceConnected;

    // Gyro part
    private SensorManager mSensorManager;
    private Sensor mGyro ;
    private static final float NS2S = 1.0f / 1000000000.0f;
    private long mLastTimestamp = 0;

    private Client client ;

    private int interactionMode = sliceTangibleOnly;
    private boolean tangibleModeActivated = false ;


    //Constrain interaction part 
    private boolean considerX ;
    private boolean considerY ;
    private boolean considerZ ;
    private boolean considerRotation ;
    private boolean considerTranslation ;

    // private CameraPreview mCameraPreview;
    //
    // private final static int STATE_BLUETOOTH_OFF = 1;
    // private final static int STATE_DISCONNECTED = 2;
    // private final static int STATE_CONNECTING = 3;
    // private final static int STATE_CONNECTED = 4;
    // private int state;
    // private boolean scanStarted;
    // private boolean scanning;
    // private BluetoothAdapter bluetoothAdapter;
    // private BluetoothDevice bluetoothDevice;
    // private RFduinoService rfduinoService;
    //
    // private boolean mTogglingBluetooth = false;
    //
    // private final BroadcastReceiver bluetoothStateReceiver = new BroadcastReceiver() {
    //     @Override
    //     public void onReceive(Context context, Intent intent) {
    //         int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0);
    //         if (!mTogglingBluetooth) {
    //             if (state == BluetoothAdapter.STATE_ON) {
    //                 upgradeState(STATE_DISCONNECTED);
    //             } else if (state == BluetoothAdapter.STATE_OFF) {
    //                 downgradeState(STATE_BLUETOOTH_OFF);
    //             }
    //         } else {
    //             mTogglingBluetooth = false;
    //             Log.d(TAG, "re-enabling bluetooth");
    //             bluetoothAdapter.enable();
    //         }
    //     }
    // };
    //
    // private final BroadcastReceiver scanModeReceiver = new BroadcastReceiver() {
    //     @Override
    //     public void onReceive(Context context, Intent intent) {
    //         scanning = (bluetoothAdapter.getScanMode() != BluetoothAdapter.SCAN_MODE_NONE);
    //         scanStarted &= scanning;
    //         updateConnectionProcess();
    //     }
    // };
    //
    // private final ServiceConnection rfduinoServiceConnection = new ServiceConnection() {
    //     @Override
    //     public void onServiceConnected(ComponentName name, IBinder service) {
    //         Log.d(TAG, "onServiceConnected");
    //         rfduinoService = ((RFduinoService.LocalBinder) service).getService();
    //         if (rfduinoService.initialize()) {
    //             Log.d(TAG, "initialize() OK");
    //             // if (rfduinoService.connect("D6:97:56:25:20:CA")) {
    //             if (rfduinoService.connect(bluetoothDevice.getAddress())) {
    //                 Log.d(TAG, "connect() OK");
    //                 upgradeState(STATE_CONNECTING);
    //             }
    //         }
    //     }
    //
    //     @Override
    //     public void onServiceDisconnected(ComponentName name) {
    //         rfduinoService = null;
    //         downgradeState(STATE_DISCONNECTED);
    //     }
    // };
    //
    // private final BroadcastReceiver rfduinoReceiver = new BroadcastReceiver() {
    //     @Override
    //     public void onReceive(Context context, Intent intent) {
    //         final String action = intent.getAction();
    //         if (RFduinoService.ACTION_CONNECTED.equals(action)) {
    //             upgradeState(STATE_CONNECTED);
    //         } else if (RFduinoService.ACTION_DISCONNECTED.equals(action)) {
    //             downgradeState(STATE_DISCONNECTED);
    //         } else if (RFduinoService.ACTION_DATA_AVAILABLE.equals(action)) {
    //             addData(intent.getByteArrayExtra(RFduinoService.EXTRA_DATA));
    //         }
    //     }
    // };

    @Override
    protected int getAppType() {
        // return NativeApp.APP_TYPE_STUB;
        return NativeApp.APP_TYPE_FLUID;
    }

    @Override
    @SuppressWarnings("deprecation")
    public void onCreate(Bundle savedInstanceState) {
        boolean wasInitialized = isInitialized();

        super.onCreate(savedInstanceState);

        if (!isInitialized()) // || !isCameraAvailable())
            return;

        FluidMechanics.getSettings(fluidSettings);
        FluidMechanics.getState(fluidState);
        fluidSettings.precision = 1 ;

        this.client = new Client();
        this.client.execute();
        // Request an overlaid action bar (title bar)
        getWindow().requestFeature(Window.FEATURE_ACTION_BAR_OVERLAY);

        // setContentView(R.layout.main);
        setContentView(R.layout.main_noar);

        mView = (ARSurfaceView)findViewById(R.id.glSurfaceView);
        setView(mView);

        // ContextMenuFragment content = new ContextMenuFragment();
        // getFragmentManager().beginTransaction().add(android.R.id.content, content).commit();
        registerForContextMenu(mView);

        mView.setOnTouchListener(this);

        mGestureDetector = new GestureDetector(new SimpleOnGestureListener() {});
        mGestureDetector.setOnDoubleTapListener(this);
        // view.setOnLongClickListener(this);

        setupActionBar();
        setupSlider();
        setupSliderPrecision();

        // bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        // if (bluetoothAdapter.isEnabled()) {
        //     // Reboot the bluetooth adapter to make sure the RFDuino
        //     // will be correctly detected
        //     mTogglingBluetooth = true;
        //     Log.d(TAG, "toggling bluetooth...");
        //     registerReceiver(bluetoothStateReceiver, new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED));
        //     bluetoothAdapter.disable();
        // } else {
        //     if (!bluetoothAdapter.enable())
        //         Log.w(TAG, "Failed to enable Bluetooth!");
        // }

        if (!wasInitialized) {
            mDataSet = -1;
            loadNewData();
        }

        // =================================================
        // Tango service

        // Instantiate Tango client
        mTango = new Tango(this);

        // Set up Tango configuration for motion tracking
        // If you want to use other APIs, add more appropriate to the config
        // like: mConfig.putBoolean(TangoConfig.KEY_BOOLEAN_DEPTH, true)
        mConfig = mTango.getConfig(TangoConfig.CONFIG_TYPE_CURRENT);
        mConfig.putBoolean(TangoConfig.KEY_BOOLEAN_MOTIONTRACKING, true);

        mSensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        mGyro = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);

        
        FluidMechanics.setInteractionMode(this.interactionMode);

        this.tangibleBtn = (Button) findViewById(R.id.tangibleBtn);
        this.tangibleBtn.setOnClickListener(this);
        /*this.tangibleBtn.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.d(TAG,"Button Clicked");
                Toast.makeText(MainActivity.this, "Button Clicked", Toast.LENGTH_SHORT).show();
            }
        });*/

        Log.d(TAG,"Listener Set");

        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        int width = size.x;
        int height = size.y;

        Log.d(TAG,"width = "+width+" height = "+height);

        // mConfig.putBoolean(TangoConfig.KEY_BOOLEAN_AUTORECOVERY, true); // default is true

        // mConfig.putBoolean(TangoConfig.KEY_BOOLEAN_LEARNINGMODE, true);
    }

    private void setTangoListeners() {
        // Select coordinate frame pairs
        ArrayList<TangoCoordinateFramePair> framePairs = new ArrayList<TangoCoordinateFramePair>();
        framePairs.add(new TangoCoordinateFramePair(
                           TangoPoseData.COORDINATE_FRAME_START_OF_SERVICE, // OK?
                           // TangoPoseData.COORDINATE_FRAME_AREA_DESCRIPTION, // needs KEY_BOOLEAN_LEARNINGMODE == true
                           TangoPoseData.COORDINATE_FRAME_DEVICE
                           ));
        // Add a listener for Tango pose data
        mTango.connectListener(framePairs, this);
    }

    // private static final int SECS_TO_MILLISECS = 1000;
    // private static final double UPDATE_INTERVAL_MS = 50.0;
    // private double mPreviousTimeStamp;
    // private double mTimeToNextUpdate = UPDATE_INTERVAL_MS;
    @Override
    public void onPoseAvailable(TangoPoseData pose) {
        // Log.d(TAG, "onPoseAvailable");

        // if (pose.statusCode == TangoPoseData.POSE_INVALID) {
        if (pose.statusCode != TangoPoseData.POSE_VALID && mDatasetLoaded == true) {
            Log.w(TAG, "Invalid pose state");

            // if (mTranslationsEnabled) {
            //     runOnUiThread(new Runnable() {
            //         @Override
            //         public void run() {
            //             mTextOverlay.setVisibility(View.VISIBLE);
            //             mTextOverlay.setText("Position non détectée !");
            //         }});
            // }
        } else {
            FluidMechanics.setTangoValues(pose.translation[0],pose.translation[1],pose.translation[2],
                                          pose.rotation[0],pose.rotation[1],pose.rotation[2],pose.rotation[3] ) ;

            // runOnUiThread(new Runnable() {
            //     @Override
            //     public void run() {
            //         mTextOverlay.setVisibility(View.INVISIBLE);
            //     }});
        }

        requestRender();

    }

    @Override
    public void onXyzIjAvailable(TangoXyzIjData arg0) {
        // Ignoring XyzIj data
    }

    @Override
    public void onTangoEvent(TangoEvent arg0) {
        // Ignoring TangoEvents
    }

    @Override
    public void onFrameAvailable(int arg0) {
        // Ignoring onFrameAvailable Events

    }

    @Override
   public void onSensorChanged(SensorEvent event) {
       if (event.sensor.getType() == Sensor.TYPE_GYROSCOPE && mDatasetLoaded == true ) {
           if (mLastTimestamp != 0) {
               float dt = (event.timestamp - mLastTimestamp) * NS2S;
               if (dt != 0) {
                   FluidMechanics.setGyroValues(dt * event.values[0],   // rx
                                                dt * event.values[1],   // ry
                                                dt * event.values[2],0);  // rz
               }
           }
           mLastTimestamp = event.timestamp;
       } else if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
           // Attention aux axes qui peuvent varier selon le device !
           /*NativeApp.setAbsoluteOrientation(
               event.values[0], // x
               event.values[1], // y
               event.values[2], // z
               event.values[3]  // w
           );*/
       }

       requestRender();
   }

   @Override
   public void onAccuracyChanged(Sensor sensor, int accuracy) {
       // Log.d(TAG, "onAccuracyChanged");
   }

    // =============================================


    // @Override
    // protected void onStart() {
    //     super.onStart();
    //
    //     registerReceiver(scanModeReceiver, new IntentFilter(BluetoothAdapter.ACTION_SCAN_MODE_CHANGED));
    //     registerReceiver(bluetoothStateReceiver, new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED));
    //     registerReceiver(rfduinoReceiver, RFduinoService.getIntentFilter());
    //
    //     if (bluetoothAdapter != null)
    //         updateState(bluetoothAdapter.isEnabled() ? STATE_DISCONNECTED : STATE_BLUETOOTH_OFF);
    // }

    // @Override
	// public void previewSizeChanged(int width, int height) {
    //     // NativeApp.init(getAppType(), getStorageDir(), width, height);
    // }

    // @Override
    // protected void onStop() {
    //     super.onStop();
    //
    //     // if (rfduinoService != null)
    //     //     rfduinoService.disconnect();
    //     try {
    //         stopService(new Intent(this, RFduinoService.class));
    //         unbindService(rfduinoServiceConnection);
    //     } catch (Exception e) {}
    //
    //     if (bluetoothAdapter != null)
    //         bluetoothAdapter.stopLeScan(this);
    //
    //     unregisterReceiver(scanModeReceiver);
    //     unregisterReceiver(bluetoothStateReceiver);
    //     unregisterReceiver(rfduinoReceiver);
    // }


    // private void upgradeState(int newState) {
    //     if (newState > state) {
    //         updateState(newState);
    //     }
    // }
    //
    // private void downgradeState(int newState) {
    //     if (newState < state) {
    //         updateState(newState);
    //     }
    // }
    //
    // private void updateState(int newState) {
    //     state = newState;
    //
    //     TextView statusText = (TextView)findViewById(R.id.textOverlay);
    //     switch (state) {
    //         case STATE_BLUETOOTH_OFF:
    //             statusText.setText("");
    //             break;
    //         case STATE_DISCONNECTED:
    //             statusText.setText("");
    //             break;
    //         case STATE_CONNECTING:
    //             statusText.setText("connecting...");
    //             break;
    //         case STATE_CONNECTED:
    //             statusText.setText("connected");
    //             break;
    //     }
    //
    //     updateConnectionProcess();
    // }
    //
    // private void updateConnectionProcess() {
    //     // switch (state) {
    //     //     case STATE_BLUETOOTH_OFF:
    //     //         break;
    //
    //     //     case STATE_DISCONNECTED:
    //     //         break;
    //
    //     //     case STATE_CONNECTING:
    //     //         break;
    //
    //     //     case STATE_CONNECTED:
    //     //         break;
    //     // }
    //
    //     Log.d(TAG, "updateConnectionProcess state=" + state);
    //
    //     if (state == STATE_DISCONNECTED) {
    //         Log.d(TAG, "state_disconnected bluetoothDevice=" + bluetoothDevice + " scanStarted=" + scanStarted);
    //         if (bluetoothDevice == null) {
    //             if (!scanStarted) {
    //                 scanStarted = true;
    //                 Log.d(TAG, "starting scan");
    //                 bluetoothAdapter.startLeScan(new UUID[] { RFduinoService.UUID_SERVICE }, this);
    //             }
    //         } else {
    //             Intent rfduinoIntent = new Intent(this, RFduinoService.class);
    //             Log.d(TAG, "bindService");
    //             bindService(rfduinoIntent, rfduinoServiceConnection, BIND_AUTO_CREATE);
    //         }
    //     }
    // }
    //
    // @Override
    // public void onLeScan(BluetoothDevice device, final int rssi, final byte[] scanRecord) {
    //     Log.d(TAG, "onLeScan");
    //     bluetoothAdapter.stopLeScan(this);
    //     bluetoothDevice = device;
    //
    //     /*GyroActivity.this.*/runOnUiThread(new Runnable() {
    //         @Override
    //         public void run() {
    //             updateConnectionProcess();
    //         }
    //     });
    // }

    // private boolean mButton1Pressed = false, mButton2Pressed = false;
    // private void addData(byte[] data) {
    //     // if (data.length < 2) {
    //     //     Log.w(TAG, "wrong data length received: " + data.length);
    //     //     return;
    //     // }
    //
    //     // Log.d(TAG, "button state: " + data[0] + " " + data[1]);
    //     // Log.d(TAG, "button state: " + data[0]);
    //     boolean button1Pressed = ((data[0] & (1 << 0)) != 0);
    //     boolean button2Pressed = ((data[0] & (1 << 1)) != 0);
    //     Log.d(TAG, "button state: " + button1Pressed + " " + button2Pressed);
    //
    //     if (button1Pressed != mButton1Pressed) {
    //         if (button1Pressed) {
    //             Log.d(TAG, "button1 pressed");
    //             FluidMechanics.releaseParticles();
    //         } else {
    //             Log.d(TAG, "button1 released");
    //         }
    //         mButton1Pressed = button1Pressed;
    //     }
    //
    //     if (button2Pressed != mButton2Pressed) {
    //         if (button2Pressed) {
    //             Log.d(TAG, "button2 pressed");
    //             FluidMechanics.buttonPressed();
    //         } else {
    //             Log.d(TAG, "button2 released");
    //             fluidSettings.surfacePercentage = FluidMechanics.buttonReleased();
    //         }
    //         mButton2Pressed = button2Pressed;
    //     }
    // }

    // @Override
    // protected void onPause() {
    //     super.onPause();
    //     if (mCameraPreview != null)
    //         mCameraPreview.pause();
    // }

    @Override
    protected void onPause() {
        super.onPause();
        try {
            mTango.disconnect();
            mIsTangoServiceConnected = false;
        } catch (TangoErrorException e) {
            Toast.makeText(getApplicationContext(), "Tango Error!",
                           Toast.LENGTH_SHORT).show();
        }
        mSensorManager.unregisterListener(this);
    }
    // @Override
    // protected void onResume() {
    //     super.onResume();
    //     if (mCameraPreview != null)
    //         mCameraPreview.resume();
    //     QCAR.onResume();
    // }
    @Override
    protected void onResume() {
        super.onResume();
        if (!mIsTangoServiceConnected) {
            try {
                setTangoListeners();
            } catch (TangoErrorException e) {
                Toast.makeText(this, "Tango Error! Restart the app!",
                               Toast.LENGTH_SHORT).show();
            }
            try {
                mTango.connect(mConfig);
                mIsTangoServiceConnected = true;
            } catch (TangoOutOfDateException e) {
                Toast.makeText(getApplicationContext(),
                               "Tango Service out of date!", Toast.LENGTH_SHORT)
                    .show();
            } catch (TangoErrorException e) {
                Toast.makeText(getApplicationContext(),
                               "Tango Error! Restart the app!", Toast.LENGTH_SHORT)
                    .show();
            }
        }
        mSensorManager.registerListener(this, mGyro, SensorManager.SENSOR_DELAY_UI);
    }

    // @Override
    // protected void processVideoFrame(final byte[] data, Camera camera) {
    //     // Log.d(TAG, "processVideoFrame");
    //     if (com.qualcomm.ar.pl.CameraPreview.instance.isReady()) {
    //         // Log.d(TAG, "QCAR onPreviewFrame");
    //         com.qualcomm.ar.pl.CameraPreview.instance.onPreviewFrame(data, camera);
    //     }
    //     super.processVideoFrame(data, camera);
    //
    //     if (mButton2Pressed) {
    //         FluidMechanics.getSettings(tempSettings);
    //         // Log.d(TAG, "temp percentage = " + tempSettings.surfacePercentage);
    //         // Log.d(TAG, "temp slider = " + (int)(tempSettings.surfacePercentage * 100));
    //         final VerticalSeekBar slider = (VerticalSeekBar)findViewById(R.id.verticalSlider);
    //         slider.customSetProgress((int)(tempSettings.surfacePercentage * 100));
    //     }
    // }

    private void loadNewData() {
        loadDataset(++mDataSet);
    }

    private void loadDataset(int id) {
        mDatasetLoaded = false;

        mDataSet = (id % 4);

        // TODO: check exceptions + display error message
        // TODO: load in background?
        switch (mDataSet) {
        // switch ((mDataSet++ % 3)) {
            case 0:
                FluidMechanics.loadDataset(copyAssetsFileToStorage("ftlelog.vtk", false));
                mVelocityDatasetLoaded = false;
                client.dataset = 1 ;
                break;
            case 1:
                // FluidMechanics.loadDataset(copyAssetsFileToStorage("head.vti", false));
                FluidMechanics.loadDataset(copyAssetsFileToStorage("ironProt.vtk", false));
                mVelocityDatasetLoaded = false;
                client.dataset = 2 ;
                break;
            case 2:
                FluidMechanics.loadDataset(copyAssetsFileToStorage("head.vti", false));
                mVelocityDatasetLoaded = false;
                client.dataset = 3 ;
                break;
            case 3:
                FluidMechanics.loadDataset(copyAssetsFileToStorage("FTLE7.vtk", false));
                FluidMechanics.loadVelocityDataset(copyAssetsFileToStorage("Velocities7.vtk", false));
                mVelocityDatasetLoaded = true;
                client.dataset = 4 ;
                break;
        }

        // Apply the computed zoom factor
        updateDataState();
        // settings.zoomFactor = fluidState.computedZoomFactor;
        settings.zoomFactor = fluidState.computedZoomFactor * 0.75f;
        fluidSettings.showSlice = true ;
        fluidSettings.sliceType = FluidMechanics.SLICE_STYLUS ;
        updateSettings();

        mDatasetLoaded = true;
    }

    // public class ContextMenuFragment extends Fragment {
    //     @Override
    //     public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    //         View root = inflater.inflate(R.layout.main, container, false);
    //         registerForContextMenu(root.findViewById(R.id.glSurfaceView));
    //         return root;
    //     }

        @Override
        public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
            super.onCreateContextMenu(menu, v, menuInfo);
            menu.add(Menu.NONE, 0, Menu.NONE, "FTLElog");
            menu.add(Menu.NONE, 1, Menu.NONE, "Iron prot");
            menu.add(Menu.NONE, 2, Menu.NONE, "Head");
            menu.add(Menu.NONE, 3, Menu.NONE, "FTLE + velocity");
        }

        @Override
        public boolean onContextItemSelected(MenuItem item) {
            /*MainActivity.this.*/loadDataset(item.getItemId());
            return super.onContextItemSelected(item);
        }
    // }

    private void setupActionBar() {
        // Remove the title text and icon from the action bar
        getActionBar().setDisplayShowTitleEnabled(false);
        getActionBar().setDisplayShowHomeEnabled(false);
        // // Make the action bar (partially) transparent
        // // getActionBar().setBackgroundDrawable(new ColorDrawable(Color.argb(0, 0, 0, 0)));
        // getActionBar().setBackgroundDrawable(new ColorDrawable(Color.argb(100, 0, 0, 0)));
        getActionBar().setBackgroundDrawable(getResources().getDrawable(R.drawable.actionbar_bg));
    }

    private float dpToPixels(float dp) {
        Resources r = getResources();
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp, r.getDisplayMetrics());
    }

    @SuppressWarnings("deprecation")
    private void setupSlider() {
        // "Jet" color map
        int[] colors = new int[] {
            0xFF00007F, // dark blue
            0xFF0000FF, // blue
            0xFF007FFF, // azure
            0xFF00FFFF, // cyan
            0xFF7FFF7F, // light green
            0xFFFFFF00, // yellow
            0xFFFF7F00, // orange
            0xFFFF0000, // red
            0xFF7F0000  // dark red
        };
        GradientDrawable colormap = new GradientDrawable(GradientDrawable.Orientation.BOTTOM_TOP, colors);
        colormap.setGradientType(GradientDrawable.LINEAR_GRADIENT);
        final VerticalSeekBar slider = (VerticalSeekBar)findViewById(R.id.verticalSlider);
        slider.setBackgroundDrawable(colormap);
        slider.setProgressDrawable(new ColorDrawable(0x00000000)); // transparent

        slider.setProgress((int)(fluidSettings.surfacePercentage * 100));

        final TextView sliderTooltip = (TextView)findViewById(R.id.sliderTooltip);
        sliderTooltip.setVisibility(View.INVISIBLE);

        slider.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            double mProgress = -1;
            boolean mPressed = false;

			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
                sliderTooltip.setVisibility(View.VISIBLE);
                mPressed = true;
			}

			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
                sliderTooltip.setVisibility(View.INVISIBLE);
                if (mProgress != -1) {
                    // Log.d(TAG, "setSurfaceValue " + mProgress);
                    fluidSettings.surfacePreview = false;
                    fluidSettings.surfacePercentage = mProgress;
                    updateDataSettings();
                    mProgress = -1;
                }
                mPressed = false;
			}

			@Override
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // Only handle events called from VerticalSeekBar. Other events, generated
                // from the base class SeekBar, contain bogus values because the Android
                // SeekBar was not meant to be vertical.
                if (fromUser)
                    return;

                if (!mPressed)
                    return;

                sliderTooltip.setText(progress + "%");
                mProgress = (double)progress/seekBar.getMax();
                // Log.d(TAG, "mProgress = " + mProgress);
                int pos = seekBar.getTop() + (int)(seekBar.getHeight() * (1.0 - mProgress));
                sliderTooltip.setPadding((int)dpToPixels(5), 0, (int)dpToPixels(5), 0);
                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(sliderTooltip.getLayoutParams());
                lp.topMargin = pos - sliderTooltip.getHeight()/2;
                lp.rightMargin = (int)dpToPixels(25);
                sliderTooltip.setLayoutParams(lp);

                
                fluidSettings.surfacePreview = true;
                fluidSettings.surfacePercentage = mProgress;
                updateDataSettings();
                
			}
		});
    }


    private void setupSliderPrecision() {
        // "Jet" color map
        int[] colors = new int[] {
            0xFF00007F, // dark blue
            0xFF0000FF, // blue
            0xFF007FFF, // azure
            0xFF00FFFF, // cyan
            0xFF7FFF7F, // light green
            0xFFFFFF00, // yellow
            0xFFFF7F00, // orange
            0xFFFF0000, // red
            0xFF7F0000  // dark red
        };
        //Have to use int
        final int step = 1;
        final int max = 150;
        final int min = 10;
        int initialValue = 100 ;
        double initialPosition = 100 ;

        GradientDrawable colormap = new GradientDrawable(GradientDrawable.Orientation.BOTTOM_TOP, colors);
        colormap.setGradientType(GradientDrawable.LINEAR_GRADIENT);
        final VerticalSeekBar sliderPrecision = (VerticalSeekBar)findViewById(R.id.verticalSliderPrecision);
        //sliderPrecision.setBackgroundDrawable(colormap);
        //sliderPrecision.setProgressDrawable(new ColorDrawable(0x00000000)); // transparent

        sliderPrecision.setMax( (max - min) / step );
        sliderPrecision.setProgress((int)(initialPosition));

        final TextView sliderTooltipPrecision = (TextView)findViewById(R.id.sliderTooltipPrecision);
        sliderTooltipPrecision.setVisibility(View.INVISIBLE);

        sliderPrecision.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            double mProgress = -1;
            boolean mPressed = false;

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                sliderTooltipPrecision.setVisibility(View.VISIBLE);
                mPressed = true;
                //Log.d(TAG, "Precision Java = " + mProgress);
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                sliderTooltipPrecision.setVisibility(View.INVISIBLE);
                if (mProgress != -1) {
                    // Log.d(TAG, "setSurfaceValue " + mProgress);
                    //fluidSettings.precision = (float)mProgress;
                    //updateDataSettings();
                    mProgress = -1;
                }
                mPressed = false;
                //Log.d(TAG, "Precision Java = " + mProgress);
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // Only handle events called from VerticalSeekBar. Other events, generated
                // from the base class SeekBar, contain bogus values because the Android
                // SeekBar was not meant to be vertical.
                if (fromUser)
                    return;

                if (!mPressed)
                    return;

                
                mProgress = (double)progress/seekBar.getMax();
                // Log.d(TAG, "mProgress = " + mProgress);
                int pos = seekBar.getTop() + (int)(seekBar.getHeight() * (1.0 - mProgress));
                sliderTooltipPrecision.setPadding((int)dpToPixels(5), 0, (int)dpToPixels(5), 0);
                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(sliderTooltipPrecision.getLayoutParams());
                lp.topMargin = pos - sliderTooltipPrecision.getHeight()/2;
                lp.rightMargin = (int)dpToPixels(25);
                sliderTooltipPrecision.setLayoutParams(lp);

                //fluidSettings.surfacePreview = true;
                double value = min/(100.0) + (progress * (step/100.0));
                fluidSettings.precision = (float)value ;

                DecimalFormat df = new DecimalFormat("0.00##");
                String tooltipvalue = df.format(value);
                sliderTooltipPrecision.setText(tooltipvalue + "");
                updateDataSettings();
                //Log.d(TAG, "Precision Java = " + mProgress);

            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_menu, menu);

        mAxisClippingMenuItem = menu.findItem(R.id.action_axisClipping);
        mStylusClippingMenuItem = menu.findItem(R.id.action_stylusClipping);

        if (!menuInitialized) {
            fluidSettings.showVolume = menu.findItem(R.id.action_showVolume).isChecked();
            fluidSettings.showSurface = menu.findItem(R.id.action_showSurface).isChecked();
            fluidSettings.showStylus = true;
            fluidSettings.showSlice = menu.findItem(R.id.action_showSlice).isChecked();
            fluidSettings.showOutline = menu.findItem(R.id.action_showOutline).isChecked();
            settings.showCamera = menu.findItem(R.id.action_showCamera).isChecked();
            fluidSettings.showCrossingLines = menu.findItem(R.id.action_showLines).isChecked();
            considerX = menu.findItem(R.id.action_constrainX).isChecked();
            considerY = menu.findItem(R.id.action_constrainY).isChecked();
            considerZ = menu.findItem(R.id.action_constrainZ).isChecked();
            considerTranslation = menu.findItem(R.id.action_constrainTranslation).isChecked();
            considerRotation = menu.findItem(R.id.action_constrainRotation).isChecked();
            updateSettings();
            updateDataSettings();

            menuInitialized = true;

        } else {
            menu.findItem(R.id.action_showVolume).setChecked(fluidSettings.showVolume);
            menu.findItem(R.id.action_showSurface).setChecked(fluidSettings.showSurface);
            menu.findItem(R.id.action_showSlice).setChecked(fluidSettings.showSlice);
            menu.findItem(R.id.action_showCamera).setChecked(settings.showCamera);
            menu.findItem(R.id.action_showLines).setChecked(fluidSettings.showCrossingLines);
            menu.findItem(R.id.action_showOutline).setChecked(fluidSettings.showOutline);
            menu.findItem(R.id.action_axisClipping).setChecked(fluidSettings.sliceType == FluidMechanics.SLICE_AXIS);
            menu.findItem(R.id.action_stylusClipping).setChecked(fluidSettings.sliceType == FluidMechanics.SLICE_STYLUS);

            menu.findItem(R.id.action_constrainX).setChecked(considerX);
            menu.findItem(R.id.action_constrainY).setChecked(considerY);
            menu.findItem(R.id.action_constrainZ).setChecked(considerZ);
            menu.findItem(R.id.action_constrainTranslation).setChecked(considerTranslation);
            menu.findItem(R.id.action_constrainRotation).setChecked(considerRotation);
        }

        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        menu.findItem(R.id.action_clipDist).setEnabled(
            !menu.findItem(R.id.action_stylusClipping).isChecked()
            && menu.findItem(R.id.action_showSlice).isChecked()
        );
        menu.findItem(R.id.action_axisClipping).setEnabled(
            menu.findItem(R.id.action_showSlice).isChecked()
        );
        menu.findItem(R.id.action_stylusClipping).setEnabled(
            menu.findItem(R.id.action_showSlice).isChecked()
        );
        return true;
    }

    @Override
	public boolean onOptionsItemSelected(MenuItem item) {
        boolean handledSetting = false;
        boolean handledDataSetting = false;
        int tmp ;

		switch (item.getItemId()) {
            case R.id.action_showVolume:
                fluidSettings.showVolume = !fluidSettings.showVolume;
                item.setChecked(fluidSettings.showVolume);
                handledDataSetting = true;
                break;

            case R.id.action_showSurface:
                fluidSettings.showSurface = !fluidSettings.showSurface;
                item.setChecked(fluidSettings.showSurface);
                handledDataSetting = true;
                break;

            case R.id.action_showSlice:
                fluidSettings.showSlice = !fluidSettings.showSlice;
                item.setChecked(fluidSettings.showSlice);
                handledDataSetting = true;
                break;

            case R.id.action_showOutline:
                fluidSettings.showOutline = !fluidSettings.showOutline;
                item.setChecked(fluidSettings.showOutline);
                handledDataSetting = true ;
                break;

            case R.id.action_showCamera:
                settings.showCamera = !settings.showCamera;
                item.setChecked(settings.showCamera);
                handledSetting = true;
                break;

            case R.id.action_showLines:
                fluidSettings.showCrossingLines = !fluidSettings.showCrossingLines;
                item.setChecked(fluidSettings.showCrossingLines);
                handledDataSetting = true;
                break;

            case R.id.action_axisClipping:
                item.setChecked(!item.isChecked());
                if (item.isChecked() && mStylusClippingMenuItem.isChecked())
                    mStylusClippingMenuItem.setChecked(false);
                handledDataSetting = true;
                break;

            case R.id.action_stylusClipping:
                item.setChecked(!item.isChecked());
                if (item.isChecked() && mAxisClippingMenuItem.isChecked())
                    mAxisClippingMenuItem.setChecked(false);
                handledDataSetting = true;
                break;

            case R.id.action_clipDist:
                showDistanceDialog();
                break;

            case R.id.action_dataTangible:
                changeInteractionMode(dataTangibleOnly);
                break;

            case R.id.action_dataTouch:
                changeInteractionMode(dataTouchOnly);
                break ;

            case R.id.action_plane:
                changeInteractionMode(sliceTangibleOnly);
                break ;

            case R.id.action_data_plane:
                changeInteractionMode(dataSliceTouchTangible);
                break ;

            case R.id.action_seeding:
                changeInteractionMode(seedPoint);
                break ;

            case R.id.change_IP:
                changeIP();
                break ;

            //Constraining interaction part
            case R.id.action_constrainX:
                considerX = !considerX ;
                //If constrainX, we want to set value in JNI to 0
                tmp = (considerX) ? 0 : 1;  
                fluidSettings.considerX = tmp; 
                item.setChecked(considerX);
                Log.d(TAG,"tmp = "+tmp);
                handledDataSetting = true ;
                break;

            case R.id.action_constrainY:
                considerY = !considerY ;
                //If considerX, we want to set value in JNI to 0
                tmp = (considerY) ? 0 : 1;  
                fluidSettings.considerY = tmp; 
                item.setChecked(considerY);
                Log.d(TAG,"tmp = "+tmp);
                handledDataSetting = true;
                break;

            case R.id.action_constrainZ:
                considerZ = !considerZ ;
                //If considerX, we want to set value in JNI to 0
                tmp = (considerZ) ? 0 : 1;  
                fluidSettings.considerZ = tmp; 
                item.setChecked(considerZ);
                Log.d(TAG,"tmp = "+tmp);
                handledDataSetting = true;
                break;

            case R.id.action_constrainTranslation:
                considerTranslation = !considerTranslation ;
                //If considerX, we want to set value in JNI to 0
                tmp = (considerTranslation) ? 0 : 1;  
                fluidSettings.considerTranslation = tmp; 
                item.setChecked(considerTranslation);
                Log.d(TAG,"tmp = "+tmp);
                handledDataSetting = true;
                break;

            case R.id.action_constrainRotation:
                considerRotation = !considerRotation ;
                //If considerX, we want to set value in JNI to 0
                tmp = (considerRotation) ? 0 : 1;  
                fluidSettings.considerRotation = tmp;
                item.setChecked(considerRotation);
                Log.d(TAG,"tmp = "+tmp);
                handledDataSetting = true;
                break;

        }

        if (handledSetting) {
            updateSettings();
            return true;
        } else if (handledDataSetting) {
            updateDataSettings();
            return true;
        } else {
            return super.onOptionsItemSelected(item);
        }
    }

    private void showDistanceDialog() {
        LayoutInflater inflater = getLayoutInflater();
        View layout = inflater.inflate(R.layout.seekbar_dialog, null);

        SeekBar seekbar = (SeekBar)layout.findViewById(R.id.seekbar_dialog_seekbar);
        final TextView text = (TextView)layout.findViewById(R.id.seekbar_dialog_text);

        seekbar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                // Log.d(TAG, "tracking started");
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // Log.d(TAG, "tracking stopped");
            }

            @Override
            public void onProgressChanged(SeekBar seekbar, int progress, boolean fromUser) {
                // Log.d(TAG, "progress changed: " + progress + " (fromUser = " + fromUser + ")");
                text.setText("Distance: " + progress);
                if (fromUser) {
                    fluidSettings.clipDist = progress;
                    updateDataSettings();
                }
            }
        });

        seekbar.setMax(2000);
        seekbar.setProgress((int)fluidSettings.clipDist);

        new AlertDialog.Builder(this)
            .setView(layout)
            .setTitle("Slicing distance")
            .setPositiveButton("Close", null)
            .show();
    }

    private void updateDataState() {
        FluidMechanics.getState(fluidState);
    }

    private void updateSettings() {
        NativeApp.setSettings(settings);
    }

    private void updateDataSettings() {
        fluidSettings.sliceType =
            mStylusClippingMenuItem.isChecked() ? FluidMechanics.SLICE_STYLUS
            : mAxisClippingMenuItem.isChecked() ? FluidMechanics.SLICE_AXIS
            : FluidMechanics.SLICE_CAMERA;

        FluidMechanics.setSettings(fluidSettings);

        VerticalSeekBar verticalSlider = (VerticalSeekBar)findViewById(R.id.verticalSlider);
        if (verticalSlider != null) {
            verticalSlider.setVisibility(
                fluidSettings.showSurface ? View.VISIBLE : View.INVISIBLE);
        }
    }

    @Override
    protected void onFrameProcessed() {
        FluidMechanics.getState(fluidState);
    }

    @Override
    public void onBackPressed() {
        // (ignore the back button)
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        // Log.d(TAG, "onTouch");

        // // Log.d(TAG, "button state = " + event.getButtonState());
        // if (event.getButtonState() == 1) {
        //     FluidMechanics.releaseParticles();
        //     return true;
        // }
        
        mGestureDetector.onTouchEvent(event);

        if(mDatasetLoaded){
            switch (event.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:
                case MotionEvent.ACTION_POINTER_DOWN:
                {   
                    int index = event.getActionIndex();
                    int id = event.getPointerId(index);
                    Log.d("Finger ID", "Finger ID = "+id);
                    Log.d("Finger Index", "Finger Index = "+index);
                    FluidMechanics.addFinger(event.getX(id), event.getY(id), id);
                    break ;
                }

                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_POINTER_UP:
                {
                    int index = event.getActionIndex();
                    int id = event.getPointerId(index);
                    Log.d("Finger ID", "Finger ID = "+id);
                    Log.d("Finger Index", "Finger Index = "+index);
                    FluidMechanics.removeFinger(id);
                    break ;
                }

                case MotionEvent.ACTION_MOVE:
                {
                    int numPtrs = event.getPointerCount();
                    float [] xPos = new float[numPtrs];
                    float [] yPos = new float[numPtrs];
                    int [] ids = new int[numPtrs];
                    for (int i = 0; i < numPtrs; ++i)
                    {
                        ids[i]  = event.getPointerId(i);
                        xPos[i] = event.getX(i);
                        yPos[i] = event.getY(i);
                        FluidMechanics.updateFingerPositions(xPos[i],yPos[i],ids[i]);
                    }
                    break ;
                }
            }
            if(event.getPointerCount() == 2){
                float dx = event.getX(0) - event.getX(1);
                float dy = event.getY(0) - event.getY(1);
                float dist = (float)Math.sqrt(dx*dx + dy*dy);

                switch (event.getActionMasked()) {
                    case MotionEvent.ACTION_POINTER_DOWN: {
                        mInitialPinchDist = dist;
                        mInitialZoomFactor = settings.zoomFactor;
                        mZoomGesture = true;
                        break;
                    }
                    case MotionEvent.ACTION_MOVE: {
                        // settings.zoomFactor = mInitialZoomFactor * (float)Math.pow(dist/mInitialPinchDist, zoomExponent);
                        settings.zoomFactor = mInitialZoomFactor * dist/mInitialPinchDist;
                        if (settings.zoomFactor <= 0.25f)
                            settings.zoomFactor = 0.25f;
                        updateSettings();
                        break;
                    }
                }
            }
            

            // NativeApp.setZoom(mZoomFactor);

        }

        /*if (mDatasetLoaded) {
            if (event.getPointerCount() == 1) {
                // Log.d(TAG, "action = " + event.getActionMasked());
                switch (event.getActionMasked()) {
                    case MotionEvent.ACTION_DOWN:
                    {
                        Log.d(TAG, "pointer down");
                        if (mVelocityDatasetLoaded) {
                            FluidMechanics.releaseParticles();
                            // mButton1Pressed = true;
                        } else {
                            FluidMechanics.buttonPressed();
                            // mButton2Pressed = true;
                        }
                        break;
                    }

                    case MotionEvent.ACTION_UP:
                    {
                        Log.d(TAG, "pointer up");
                        //fluidSettings.surfacePercentage = FluidMechanics.buttonReleased();
                        FluidMechanics.buttonReleased();
                        // mButton1Pressed = false;
                        // mButton2Pressed = false;
                        break;
                    }

                }
            }
        }

        if (event.getPointerCount() <= 1) {
            int action = event.getActionMasked();

            if (action == MotionEvent.ACTION_UP) {
                mZoomGesture = false;
            }
        } else {
            float dx = event.getX(0) - event.getX(1);
            float dy = event.getY(0) - event.getY(1);
            float dist = (float)Math.sqrt(dx*dx + dy*dy);

            switch (event.getActionMasked()) {
                case MotionEvent.ACTION_POINTER_DOWN: {
                    mInitialPinchDist = dist;
                    mInitialZoomFactor = settings.zoomFactor;
                    mZoomGesture = true;
                    break;
                }
                case MotionEvent.ACTION_MOVE: {
                    // settings.zoomFactor = mInitialZoomFactor * (float)Math.pow(dist/mInitialPinchDist, zoomExponent);
                    settings.zoomFactor = mInitialZoomFactor * dist/mInitialPinchDist;
                    if (settings.zoomFactor <= 0.25f)
                        settings.zoomFactor = 0.25f;
                    updateSettings();
                    break;
                }
            }

            // NativeApp.setZoom(mZoomFactor);
        }*/

        return true;
    }

    // // Handle mouse wheel events
    // @Override
    // public boolean onGenericMotionEvent(MotionEvent event) {
    //     if (event.getSource() & InputDevice.SOURCE_CLASS_POINTER) {
    //         switch (event.getAction()) {
    //             case MotionEvent.ACTION_SCROLL:
    //                 if (event.getAxisValue(MotionEvent.AXIS_VSCROLL) < 0.0f)
    //                     Log.d(TAG, "wheel down");
    //                 else
    //                     Log.d(TAG, "wheel up");
    //                 return true;
    //         }
    //     }
    //     return super.onGenericMotionEvent(event);
    // }

    @Override
    public boolean onDoubleTap(MotionEvent e) {
        Log.d(TAG, "onDoubleTap");
        // loadNewData();
        openContextMenu(findViewById(R.id.glSurfaceView));
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e) {
        // Log.d(TAG, "onDoubleTapEvent");
        return true;
    }

	// @Override
    // public boolean onLongClick(View view) {
    //     openContextMenu(findViewById(R.id.glSurfaceView));
    //     return true;
    // }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e) {
        // Log.d(TAG, "onSingleTapConfirmed");

        // try {
        //     stopService(new Intent(this, RFduinoService.class));
        //     unbindService(rfduinoServiceConnection);
        // } catch (Exception e2) {}
        //
        // // if (state == STATE_DISCONNECTED) {
        // if (state != STATE_CONNECTED) {
        //     Intent rfduinoIntent = new Intent(this, RFduinoService.class);
        //     Log.d(TAG, "bindService");
        //     bindService(rfduinoIntent, rfduinoServiceConnection, BIND_AUTO_CREATE);
        // }

        return true;
    }

   public void requestRender(){
        if (mView != null){
            //Log.d(TAG,"RequestRender");
            mView.requestRender();
            client.setData(FluidMechanics.getData());
        }
            
   } 

   public void changeIP(){
        this.client.closeConnection = false ;
        this.client = new Client();
        this.client.execute();

   }

   public void changeInteractionMode(int mode){
        this.interactionMode = mode ;
        FluidMechanics.setInteractionMode(mode);
   }


    @Override
    public void onClick(View v) {
        //Log.d(TAG,"On click listener");
        if(v.getId() == R.id.tangibleBtn){
            //Log.d(TAG, "Tangible Button");
            this.tangibleModeActivated = !this.tangibleModeActivated ;
            if(this.tangibleModeActivated){
                //this.tangibleBtn.setBackgroundColor(Color.ORANGE);
                FluidMechanics.buttonPressed();    
            }
            else{
                //this.tangibleBtn.setBackgroundColor(Color.DARK_GRAY);
                FluidMechanics.buttonReleased();
            }
        }
    }
}
