package com.qualcomm.ar.pl;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Vector;
import org.json.JSONException;
import org.json.JSONObject;
import android.util.Log;

// Functions called by the native code:
// CameraPreview() [ctor]
// checkPermission()
// getDirection()
// getTypedCameraParameter()
// start()
// close()
// getBitsPerPixel()
// getCameraCapabilities()
// getCaptureInfo()
// getDeviceID()
// getNumberOfCameras()
// getStatus()
// init()
// open()
// setCaptureInfo()
// setSurfaceManager()
// setTypedCameraParameter()
// stop()

public class CameraPreview
    implements Camera.PreviewCallback
{
    private SurfaceManager surfaceManager;
    private Vector<CameraCacheInfo> cameraCacheInfo;
    private HashMap<Camera, Integer> cameraCacheInfoIndexCache;
    private static Method _addCallbackBufferFunc;
    private static Object[] _addCallbackBufferArgs;
    private static Method _setPreviewCallbackWithBufferFunc;
    private static Method _setPreviewTextureFunc;
    private static Method _updateTexImage;
    private static Constructor<?> _surfaceTextureConstructor;
    private static final int CAMERA_CAPTUREINFO_VALUE_WIDTH = 0;
    private static final int CAMERA_CAPTUREINFO_VALUE_HEIGHT = 1;
    private static final int CAMERA_CAPTUREINFO_VALUE_FORMAT = 2;
    private static final int CAMERA_CAPTUREINFO_VALUE_FRAMERATE = 3;
    private static final int CAMERA_CAPTUREINFO_VALUE_PREVIEWSURFACEENABLED = 4;
    private static final int _NUM_CAMERA_CAPTUREINFO_VALUE_ = 5;
    private static final int CAMERA_CAPSINFO_VALUE_SUPPORTED_QUERYABLE_PARAMS = 0;
    private static final int CAMERA_CAPSINFO_VALUE_SUPPORTED_SETTABLE_PARAMS = 1;
    private static final int CAMERA_CAPSINFO_VALUE_SUPPORTED_PARAMVALUES = 2;
    private static final int CAMERA_CAPSINFO_VALUE_NUM_SUPPORTED_IMAGESIZES = 3;
    private static final int CAMERA_CAPSINFO_VALUE_NUM_SUPPORTED_FRAMERATES = 4;
    private static final int CAMERA_CAPSINFO_VALUE_NUM_SUPPORTED_IMAGEFORMATS = 5;
    private static final int _NUM_CAMERA_CAPSINFO_VALUE_ = 6;
    private static final int AR_CAMERA_PARAMTYPE_BASE = 536870912;
    private static final int AR_CAMERA_PARAMTYPE_TORCHMODE = 536870913;
    private static final int AR_CAMERA_PARAMTYPE_FOCUSMODE = 536870914;
    private static final int AR_CAMERA_PARAMTYPE_FOCUSVALUE = 536870916;
    private static final int AR_CAMERA_PARAMTYPE_FOCUSRANGE = 536870920;
    private static final int AR_CAMERA_PARAMTYPE_FOCUSREGION = 536870928;
    private static final int AR_CAMERA_PARAMTYPE_EXPOSUREMODE = 536870944;
    private static final int AR_CAMERA_PARAMTYPE_EXPOSUREVALUE = 536870976;
    private static final int AR_CAMERA_PARAMTYPE_EXPOSURERANGE = 536871040;
    private static final int AR_CAMERA_PARAMTYPE_EXPOSURECOMPENSATIONVALUE = 536871168;
    private static final int AR_CAMERA_PARAMTYPE_EXPOSURECOMPENSATIONRANGE = 536871424;
    private static final int AR_CAMERA_PARAMTYPE_WHITEBALANCEMODE = 536871936;
    private static final int AR_CAMERA_PARAMTYPE_WHITEBALANCEVALUE = 536872960;
    private static final int AR_CAMERA_PARAMTYPE_WHITEBALANCERANGE = 536875008;
    private static final int AR_CAMERA_PARAMTYPE_ZOOMVALUE = 536879104;
    private static final int AR_CAMERA_PARAMTYPE_ZOOMRANGE = 536887296;
    private static final int AR_CAMERA_PARAMTYPE_BRIGHTNESSVALUE = 536903680;
    private static final int AR_CAMERA_PARAMTYPE_BRIGHTNESSRANGE = 536936448;
    private static final int AR_CAMERA_PARAMTYPE_CONTRASTVALUE = 537001984;
    private static final int AR_CAMERA_PARAMTYPE_CONTRASTRANGE = 537133056;
    private static final int AR_CAMERA_PARAMTYPE_ROTATION = 537395200;
    private static final int AR_CAMERA_PARAMVALUE_BASE = 805306368;
    private static final int AR_CAMERA_TORCHMODE_OFF = 805306369;
    private static final int AR_CAMERA_TORCHMODE_ON = 805306370;
    private static final int AR_CAMERA_TORCHMODE_AUTO = 805306372;
    private static final int AR_CAMERA_TORCHMODE_CONTINUOUSAUTO = 805306376;
    private static final int AR_CAMERA_FOCUSMODE_NORMAL = 805306384;
    private static final int AR_CAMERA_FOCUSMODE_AUTO = 805306400;
    private static final int AR_CAMERA_FOCUSMODE_CONTINUOUSAUTO = 805306432;
    private static final int AR_CAMERA_FOCUSMODE_MACRO = 805306496;
    private static final int AR_CAMERA_FOCUSMODE_INFINITY = 805306624;
    private static final int AR_CAMERA_FOCUSMODE_FIXED = 805306880;
    private static final int AR_CAMERA_EXPOSUREMODE_NORMAL = 805310464;
    private static final int AR_CAMERA_EXPOSUREMODE_AUTO = 805314560;
    private static final int AR_CAMERA_EXPOSUREMODE_CONTINUOUSAUTO = 805322752;
    private static final int AR_CAMERA_WHITEBALANCEMODE_NORMAL = 805371904;
    private static final int AR_CAMERA_WHITEBALANCEMODE_AUTO = 805437440;
    private static final int AR_CAMERA_WHITEBALANCEMODE_CONTINUOUSAUTO = 805568512;
    private static final int AR_CAMERA_TYPE_UNKNOWN = 268447760;
    private static final int AR_CAMERA_TYPE_MONO = 268447761;
    private static final int AR_CAMERA_TYPE_STEREO = 268447762;
    private static final int AR_CAMERA_DIRECTION_UNKNOWN = 268443664;
    private static final int AR_CAMERA_DIRECTION_BACK = 268443665;
    private static final int AR_CAMERA_DIRECTION_FRONT = 268443666;
    private static final int AR_CAMERA_STATUS_UNKNOWN = 268443648;
    private static final int AR_CAMERA_STATUS_UNINITIALIZED = 268443649;
    private static final int AR_CAMERA_STATUS_OPENED = 268443650;
    private static final int AR_CAMERA_STATUS_CAPTURE_RUNNING = 268443651;
    private static final int AR_CAMERA_IMAGE_FORMAT_UNKNOWN = 268439808;
    private static final int AR_CAMERA_IMAGE_FORMAT_LUM = 268439809;
    private static final int AR_CAMERA_IMAGE_FORMAT_RGB565 = 268439810;
    private static final int AR_CAMERA_IMAGE_FORMAT_RGB888 = 268439811;
    private static final int AR_CAMERA_IMAGE_FORMAT_RGB24 = 268439811;
    private static final int AR_CAMERA_IMAGE_FORMAT_RGBA8888 = 268439812;
    private static final int AR_CAMERA_IMAGE_FORMAT_RGBA32 = 268439812;
    private static final int AR_CAMERA_IMAGE_FORMAT_ARGB8888 = 268439813;
    private static final int AR_CAMERA_IMAGE_FORMAT_ARGB32 = 268439813;
    private static final int AR_CAMERA_IMAGE_FORMAT_BGRA8888 = 268439814;
    private static final int AR_CAMERA_IMAGE_FORMAT_BGRA32 = 268439814;
    private static final int AR_CAMERA_IMAGE_FORMAT_NV12 = 268439815;
    private static final int AR_CAMERA_IMAGE_FORMAT_NV16 = 268439816;
    private static final int AR_CAMERA_IMAGE_FORMAT_NV21 = 268439817;
    private static final int AR_CAMERA_IMAGE_FORMAT_YV12 = 268439818;
    private static final int AR_CAMERA_IMAGE_FORMAT_YV16 = 268439819;
    private static final int AR_CAMERA_IMAGE_FORMAT_RGBA5551 = 268439820;
    private static final int AR_CAMERA_IMAGE_FORMAT_RGBA4444 = 268439821;
    private static final int AR_CAMERA_IMAGE_FORMAT_BGR888 = 268439822;
    private static final int AR_CAMERA_IMAGE_FORMAT_BGR24 = 268439822;
    private static final int[] CAMERA_IMAGE_FORMAT_CONVERSIONTABLE = { 16, AR_CAMERA_IMAGE_FORMAT_NV16, 17, AR_CAMERA_IMAGE_FORMAT_NV21, 4, AR_CAMERA_IMAGE_FORMAT_RGB565, 842094169, AR_CAMERA_IMAGE_FORMAT_YV12 };

    private static boolean CONVERT_FORMAT_TO_PL = true;
    private static boolean CONVERT_FORMAT_TO_ANDROID = false;
    private static final int NUM_CAPTURE_BUFFERS = 2;
    private static final int NUM_CAPTURE_BUFFERS_TO_ADD = 2;
    private static final int NUM_MAX_CAMERAOPEN_RETRY = 10;
    private static final int TIME_CAMERAOPEN_RETRY_DELAY_MS = 250;
    private static final String MODULENAME = "CameraPreview";
    private static final String FOCUS_MODE_CONTINUOUS_PICTURE = "continuous-picture";
    private static final String FOCUS_MODE_NORMAL = "normal";

    public static CameraPreview instance = null;
    public static CameraCacheInfo cci = null;
    private static final String TAG = "ARViewer";

    private boolean ready = false;

    public CameraPreview() {
        // Log.d(TAG, "CameraPreview()");
        instance = this;
    }

    public boolean isReady() {
        return ready;
    }

    // (native)
    private boolean checkPermission() {
        // Log.d(TAG, "checkPermission()");
        return true;
    }

    private void setCameraCapsBit(CameraCacheInfo cci, int capsIndex, int paramType, boolean value) {
        // Log.d(TAG, "setCameraCapsBit()");

        int baseValue = 0;
        switch (capsIndex) {
            case 2:
                baseValue = AR_CAMERA_PARAMVALUE_BASE;
                break;
            case 0:
            case 1:
                baseValue = AR_CAMERA_PARAMTYPE_BASE;
                break;
            default:
                return;
        }

        int index = (int)(Math.log(paramType & (baseValue ^ 0xFFFFFFFF)) / Math.log(2.0D));

        if (value)
            cci.caps[capsIndex] |= 1 << index;
        else
            cci.caps[capsIndex] &= (1 << index ^ 0xFFFFFFFF);
    }

    private int translateImageFormat(int fromValue, boolean conversionMode) {
        // Log.d(TAG, "translateImageFormat(" + fromValue + ", " + conversionMode + ")");

        for (int i = 0; i < CAMERA_IMAGE_FORMAT_CONVERSIONTABLE.length / 2; i++) {
            int compareValue = conversionMode == CONVERT_FORMAT_TO_PL ? CAMERA_IMAGE_FORMAT_CONVERSIONTABLE[(i * 2)] : CAMERA_IMAGE_FORMAT_CONVERSIONTABLE[(i * 2 + 1)];

            if (fromValue == compareValue)
                return conversionMode == CONVERT_FORMAT_TO_PL ? CAMERA_IMAGE_FORMAT_CONVERSIONTABLE[(i * 2 + 1)] : CAMERA_IMAGE_FORMAT_CONVERSIONTABLE[(i * 2)];
        }

        return conversionMode == CONVERT_FORMAT_TO_PL ? AR_CAMERA_IMAGE_FORMAT_UNKNOWN : 0;
    }

    // (native)
    int getBitsPerPixel(int imgFormat) {
        // Log.d(TAG, "getBitsPerPixel(" + imgFormat + ")");

        switch (imgFormat) {
            case 16:
                return 16;
            case 4:
                return 16;
            case 17:
                return 12;
            case 842094169: // internal format?
                return 12;
        }

        return 0;
    }

    @Override
    public void onPreviewFrame(byte[] capturedBuffer, Camera camera) {
        if (!ready)
            return;

        newFrameAvailable(0, cci.bufferWidth, cci.bufferHeight, cci.bufferFormatPL, capturedBuffer);
    }

    private native void newFrameAvailable(int cameraCacheInfoIndex, int bufferWidth, int bufferHeight, int bufferFormatPL, byte[] buffer);

    // (native)
    public boolean init() {
        // Log.d(TAG, "init()");
        return true;
    }

    // (native)
    public void setSurfaceManager(SurfaceManager sm) {
        // Log.d(TAG, "setSurfaceManager()");
    }

    // (native)
    public int getNumberOfCameras() {
        // Log.d(TAG, "getNumberOfCameras()");
        return 1;
    }

    // (native)
    public int getDirection(int cameraID) {
        // Log.d(TAG, "getDirection()");
        return AR_CAMERA_DIRECTION_BACK;
    }

    // (native)
    public int getDeviceID(int cameraCacheInfoIndex) {
        // Log.d(TAG, "getDeviceID()");

        if (cci == null) {
            SystemTools.setSystemErrorCode(4);
            return -1;
        }

        return cci.deviceID;
    }

    public void setCamera(Camera camera) {
        Camera.Parameters cameraParams = camera.getParameters();

        cci = new CameraCacheInfo();
        cci.deviceID = 0;
        cci.camera = camera;
        cci.surface = null;
        cci.surfaceTexture = null;
        cci.buffer = ((byte[][])null);
        cci.bufferWidth = cameraParams.getPreviewSize().width;
        cci.bufferHeight = cameraParams.getPreviewSize().height;
        cci.bufferFormatPL = translateImageFormat(cameraParams.getPreviewFormat(), CONVERT_FORMAT_TO_PL);
        cci.caps = null;
        // cci.status = AR_CAMERA_STATUS_OPENED;
        cci.status = AR_CAMERA_STATUS_CAPTURE_RUNNING;
        cci.isAutoFocusing = false;
        // cci.requestWidth = cci.bufferWidth;
        // cci.requestHeight = cci.bufferHeight;
        // cci.requestFormatAndroid = 17; // FIXME: hardcoded
        // cci.overrideWidth = cci.bufferWidth;
        // cci.overrideHeight = cci.bufferHeight;
        // cci.overrideFormatAndroid = cci.requestFormatAndroid;
    }

    // (native)
    public int open(int camIndex, int type, int direction, String customData, int[] captureInfo, int[] overrideCaptureInfo) {
        // Log.d(TAG, "open()");
        ready = true;
        return 0; // cameraCacheInfoIndex
    }

    // (native)
    public boolean close(int cameraCacheInfoIndex) {
        // Log.d(TAG, "close()");
        // cci.status = AR_CAMERA_STATUS_UNINITIALIZED;
        return true;
    }

    // (native)
    public int[] getCameraCapabilities(int cameraCacheInfoIndex) {
        // Log.d(TAG, "getCameraCapabilities()");

        if (cci == null) {
            SystemTools.setSystemErrorCode(4);
            return null;
        }

        if (cci.caps != null)
            return cci.caps;

        Camera.Parameters cameraParams = cci.camera.getParameters();

        if (cameraParams == null) {
            SystemTools.setSystemErrorCode(6);
            return null;
        }

        List supportedImageSizes = cameraParams.getSupportedPreviewSizes();
        List supportedFrameRates = cameraParams.getSupportedPreviewFrameRates();
        List supportedImageFormats = cameraParams.getSupportedPreviewFormats();
        List supportedFlashModes = cameraParams.getSupportedFlashModes();
        List supportedFocusModes = cameraParams.getSupportedFocusModes();

        int numSupportedImageSizes = supportedImageSizes != null ? supportedImageSizes.size() : 0;
        int numSupportedFrameRates = supportedFrameRates != null ? supportedFrameRates.size() : 0;
        int numSupportedImageFormats = supportedImageFormats != null ? supportedImageFormats.size() : 0;

        int capsArraySize = 6 + numSupportedImageSizes * 2 + numSupportedFrameRates + numSupportedImageFormats;

        cci.caps = new int[capsArraySize];

        int capsIndex = 0;
        cci.caps[capsIndex] = AR_CAMERA_PARAMTYPE_BASE;
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_TORCHMODE, (supportedFlashModes.contains("torch")) || (supportedFlashModes.contains("on")));
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_FOCUSMODE, true);
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_FOCUSVALUE, SystemTools.checkMinimumApiLevel(8));
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_EXPOSURECOMPENSATIONVALUE, SystemTools.checkMinimumApiLevel(8));
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_EXPOSURECOMPENSATIONRANGE, SystemTools.checkMinimumApiLevel(8));
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_ZOOMVALUE, (SystemTools.checkMinimumApiLevel(8)) && (cameraParams.isZoomSupported()));
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_ZOOMRANGE, (SystemTools.checkMinimumApiLevel(8)) && (cameraParams.isZoomSupported()));

        capsIndex = 1;
        cci.caps[capsIndex] = AR_CAMERA_PARAMTYPE_BASE;
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_TORCHMODE, (supportedFlashModes.contains("torch")) || (supportedFlashModes.contains("on")));
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_FOCUSMODE, true);
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_EXPOSURECOMPENSATIONVALUE, SystemTools.checkMinimumApiLevel(8));
        setCameraCapsBit(cci, capsIndex, AR_CAMERA_PARAMTYPE_ZOOMVALUE, (SystemTools.checkMinimumApiLevel(8)) && (cameraParams.isZoomSupported()));

        capsIndex = 2;
        cci.caps[capsIndex] = AR_CAMERA_PARAMVALUE_BASE;
        if ((supportedFlashModes != null) && ((supportedFlashModes.contains("torch")) || (supportedFlashModes.contains("on")))) {
            setCameraCapsBit(cci, capsIndex, AR_CAMERA_TORCHMODE_OFF, true);
            setCameraCapsBit(cci, capsIndex, AR_CAMERA_TORCHMODE_ON, true);
        }
        if (supportedFocusModes != null) {
            setCameraCapsBit(cci, capsIndex, AR_CAMERA_FOCUSMODE_NORMAL, true);
            setCameraCapsBit(cci, capsIndex, AR_CAMERA_FOCUSMODE_AUTO, supportedFocusModes.contains("auto"));
            setCameraCapsBit(cci, capsIndex, AR_CAMERA_FOCUSMODE_CONTINUOUSAUTO, ((SystemTools.checkMinimumApiLevel(9)) && (supportedFocusModes.contains("continuous-video"))) || ((SystemTools.checkMinimumApiLevel(14)) && (supportedFocusModes.contains("continuous-picture"))));

            setCameraCapsBit(cci, capsIndex, AR_CAMERA_FOCUSMODE_MACRO, supportedFocusModes.contains("macro"));
            setCameraCapsBit(cci, capsIndex, AR_CAMERA_FOCUSMODE_INFINITY, supportedFocusModes.contains("infinity"));
            setCameraCapsBit(cci, capsIndex, AR_CAMERA_FOCUSMODE_FIXED, supportedFocusModes.contains("fixed"));
        }

        cci.caps[3] = numSupportedImageSizes;
        cci.caps[4] = numSupportedFrameRates;
        cci.caps[5] = numSupportedImageFormats;

        int indexOffset = 6;

        if (numSupportedImageSizes > 0) {
            ListIterator iterSizes = supportedImageSizes.listIterator();
            while (iterSizes.hasNext()) {
                Camera.Size size = (Camera.Size)iterSizes.next();
                cci.caps[indexOffset] = size.width;
                cci.caps[(indexOffset + 1)] = size.height;
                indexOffset += 2;
            }

        }

        if (numSupportedFrameRates > 0) {
            ListIterator iterFramerates = supportedFrameRates.listIterator();
            while (iterFramerates.hasNext()) {
                int framerate = ((Integer)iterFramerates.next()).intValue();
                cci.caps[indexOffset] = framerate;
                indexOffset++;
            }

        }

        if (numSupportedImageFormats > 0) {
            ListIterator iterFormats = supportedImageFormats.listIterator();
            while (iterFormats.hasNext()) {
                int format = ((Integer)iterFormats.next()).intValue();
                cci.caps[indexOffset] = translateImageFormat(format, true);
                indexOffset++;
            }

        }

        return cci.caps;
    }

    // (native)
    public boolean setCaptureInfo(int cameraCacheInfoIndex, int[] captureInfo, int[] overrideCaptureInfo) {
        // Log.d(TAG, "setCaptureInfo()");
        return true;
    }

    // (native)
    public int[] getCaptureInfo(int cameraCacheInfoIndex) {
        // Log.d(TAG, "getCaptureInfo()");

        if (cci == null) {
            SystemTools.setSystemErrorCode(4);
            return null;
        }

        Camera.Parameters cameraParams = cci.camera.getParameters();

        if (cameraParams == null) {
            SystemTools.setSystemErrorCode(6);
            return null;
        }

        int[] captureInfo = null;

        try {
            captureInfo = new int[5];
            captureInfo[0] = cameraParams.getPreviewSize().width;
            captureInfo[1] = cameraParams.getPreviewSize().height;
            captureInfo[2] = translateImageFormat(cameraParams.getPreviewFormat(), CONVERT_FORMAT_TO_PL);
            captureInfo[3] = cameraParams.getPreviewFrameRate();
            captureInfo[4] = ((cci.surface != null) || (cci.surfaceTexture != null) ? 1 : 0);

        } catch (Exception e) {
            SystemTools.setSystemErrorCode(6);
            return null;
        }

        return captureInfo;
    }

    // (native)
    public boolean start(int cameraCacheInfoIndex) {
        // Log.d(TAG, "start()");
        // cci.status = AR_CAMERA_STATUS_CAPTURE_RUNNING;
        return true;
    }

    // (native)
    public boolean stop(int cameraCacheInfoIndex) {
        // Log.d(TAG, "stop()");
        // cci.status = AR_CAMERA_STATUS_OPENED;
        return true;
    }

    // (native)
    boolean setTypedCameraParameter(int cameraCacheInfoIndex, int type, Object value) {
        // Log.d(TAG, "setTypedCameraParameter()");
        return true;
    }

    // (native)
    Object getTypedCameraParameter(int cameraCacheInfoIndex, int type) {
        // Log.d(TAG, "getTypedCameraParameter()");

        if ((cci == null) || (cci.camera == null)) {
            SystemTools.setSystemErrorCode(4);
            return null;
        }

        Camera.Parameters cameraParams = cci.camera.getParameters();

        if (cameraParams == null) {
            SystemTools.setSystemErrorCode(6);
            return null;
        }

        try {
            switch (type) {
                case AR_CAMERA_PARAMTYPE_TORCHMODE:
                    String flashMode = cameraParams.getFlashMode();
                    if ((flashMode.equals("torch")) || (flashMode.equals("on"))) {
                        return Integer.valueOf(AR_CAMERA_TORCHMODE_ON);
                    }
                    if (flashMode.equals("off")) {
                        return Integer.valueOf(AR_CAMERA_TORCHMODE_OFF);
                    }

                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_FOCUSMODE:
                    String focusMode = cameraParams.getFocusMode();
                    if (focusMode.equals("auto")) {
                        return Integer.valueOf(cci.isAutoFocusing ? AR_CAMERA_FOCUSMODE_AUTO : AR_CAMERA_FOCUSMODE_NORMAL);
                    }
                    if (((SystemTools.checkMinimumApiLevel(9)) && (focusMode.equals("continuous-video"))) || ((SystemTools.checkMinimumApiLevel(14)) && (focusMode.equals("continuous-picture")))) {
                        return Integer.valueOf(AR_CAMERA_FOCUSMODE_CONTINUOUSAUTO);
                    }
                    if (focusMode.equals("infinity")) {
                        return Integer.valueOf(AR_CAMERA_FOCUSMODE_INFINITY);
                    }
                    if (focusMode.equals("macro")) {
                        return Integer.valueOf(AR_CAMERA_FOCUSMODE_MACRO);
                    }
                    if (focusMode.equals("fixed")) {
                        return Integer.valueOf(AR_CAMERA_FOCUSMODE_FIXED);
                    }

                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_FOCUSVALUE:
                    if (SystemTools.checkMinimumApiLevel(8)) {
                        return Float.valueOf(cameraParams.getFocalLength());
                    }

                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_FOCUSRANGE:
                    if (SystemTools.checkMinimumApiLevel(9)) {
                        float[] focusDistances = new float[3];
                        cameraParams.getFocusDistances(focusDistances);

                        float[] focusValueRange = new float[2];
                        focusValueRange[0] = focusDistances[0];
                        focusValueRange[1] = focusDistances[2];

                        return focusValueRange;
                    }

                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_FOCUSREGION:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_EXPOSUREMODE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_EXPOSUREVALUE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_EXPOSURERANGE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_EXPOSURECOMPENSATIONVALUE:
                    if (SystemTools.checkMinimumApiLevel(8)) {
                        return Integer.valueOf(cameraParams.getExposureCompensation());
                    }

                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_EXPOSURECOMPENSATIONRANGE:
                    if (SystemTools.checkMinimumApiLevel(8)) {
                        int[] exposureCompRange = new int[2];
                        exposureCompRange[0] = cameraParams.getMinExposureCompensation();
                        exposureCompRange[1] = cameraParams.getMaxExposureCompensation();

                        return exposureCompRange;
                    }

                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_WHITEBALANCEMODE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_WHITEBALANCEVALUE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_WHITEBALANCERANGE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_ZOOMVALUE:
                    if ((SystemTools.checkMinimumApiLevel(8)) && (cameraParams.isZoomSupported())) {
                        return Integer.valueOf(cameraParams.getZoom());
                    }

                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_ZOOMRANGE:
                    if ((SystemTools.checkMinimumApiLevel(8)) && (cameraParams.isZoomSupported())) {
                        int[] zoomRange = new int[2];
                        zoomRange[0] = 0;
                        zoomRange[1] = cameraParams.getMaxZoom();
                        return zoomRange;
                    }

                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_BRIGHTNESSVALUE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_BRIGHTNESSRANGE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_CONTRASTVALUE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_CONTRASTRANGE:
                    SystemTools.setSystemErrorCode(6);
                    return null;

                case AR_CAMERA_PARAMTYPE_ROTATION:
                    SystemTools.setSystemErrorCode(6);
                    return null;
            }

            return null;

        } catch (Exception e) {
            SystemTools.setSystemErrorCode(6);
        }

        return null;
    }

    // (native)
    int getStatus(int cameraCacheInfoIndex) {
        // Log.d(TAG, "getStatus()");

        if (cci == null) {
            SystemTools.setSystemErrorCode(4);
            return AR_CAMERA_STATUS_UNKNOWN;
        }

        return cci.status;
    }

    public class CameraCacheInfo {
        int deviceID;
        Camera camera;
        CameraSurface surface;
        Object surfaceTexture;
        byte[][] buffer;
        int bufferWidth;
        int bufferHeight;
        int bufferFormatPL;
        int requestWidth;
        int requestHeight;
        int requestFormatAndroid;
        int overrideWidth;
        int overrideHeight;
        int overrideFormatAndroid;
        int[] caps;
        int status;
        boolean isAutoFocusing;

        public CameraCacheInfo() {}
    }
}
