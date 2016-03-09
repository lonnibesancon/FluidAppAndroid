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

import com.qualcomm.QCAR.QCAR;

public class CameraPreview extends SurfaceView
    implements SurfaceHolder.Callback
{
    private static final String TAG = Config.APP_TAG;
    private static final boolean DEBUG = Config.DEBUG;

    public interface SizeCallback {
        public void previewSizeChanged(int width, int height);
    }

    private Activity mActivity;
    private Camera.PreviewCallback mPreviewCallback;
    private SizeCallback mSizeCallback;
    private SurfaceHolder mHolder, mSurfaceHolder;
    private int mCameraID;
    private Camera mCamera;
    private int mVideoWidth, mVideoHeight;

    @SuppressWarnings("deprecation")
    public CameraPreview(Context context, AttributeSet attrs) {
        super(context, attrs);
        mActivity = (Activity)context;

        // Install a SurfaceHolder.Callback so we get notified when the
        // underlying surface is created and destroyed.
        mHolder = getHolder();
        mHolder.addCallback(this);
        // deprecated setting, but required on Android versions prior to 3.0
        mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
        int frontCameraID = -1, backCameraID = -1;
        for (int i = 0; i < Camera.getNumberOfCameras(); ++i) {
            Camera.getCameraInfo(i, cameraInfo);
            switch (cameraInfo.facing) {
                case Camera.CameraInfo.CAMERA_FACING_BACK:
                    backCameraID = i;
                    break;
                case Camera.CameraInfo.CAMERA_FACING_FRONT:
                    frontCameraID = i;
                    break;
            }
        }

        mCameraID = (attrs.getAttributeValue("http://example.org/arviewer", "camera") == "front"
                     ? frontCameraID
                     : backCameraID);

        mVideoWidth = attrs.getAttributeUnsignedIntValue("http://example.org/arviewer", "videoWidth", 640);
        mVideoHeight = attrs.getAttributeUnsignedIntValue("http://example.org/arviewer", "videoHeight", 480);

        mCamera = getCameraInstance(mCameraID);
    }

    // public void setCamera(Camera camera) {
    //     if (mCamera != null && mCamera != camera) {
    //         try {
    //             mCamera.stopPreview();
    //             mCamera.setPreviewCallback(null);
    //         } catch (Exception e){
    //             // ignore: tried to stop a non-existent preview
    //         }
    //     }

    //     mCamera = camera;
    // }

    public Camera getCamera() {
        return mCamera;
    }

    public void setPreviewCallback(Camera.PreviewCallback callback) {
        Log.d(TAG, "setPreviewCallback");
        mPreviewCallback = callback;
    }

    public void setSizeCallback(SizeCallback callback) {
        mSizeCallback = callback;
    }

    private Camera getCameraInstance(int id) {
        Camera c = null;

        try {
            // Log.d(TAG, "Opening camera ID: " + id);
            c = Camera.open(id);
        } catch (Exception e) {
            // Camera is not available (in use or does not exist)
            Log.d(TAG, "Error opening the camera: ", e);
            MessageBox.error(mActivity, "Unable to open the camera.");
        }

        return c; // returns null if camera is unavailable
    }

    public void pause() {
        if (mCamera != null) {
            try {
                mCamera.stopPreview();
                mCamera.setPreviewCallback(null);
            } catch (Exception e) {
                // ignore: tried to stop a non-existent preview
            }
            mCamera.release();
            mCamera = null;
        }
    }

    @SuppressWarnings("deprecation")
    public void resume() {
        if (mCamera == null) {
            mCamera = getCameraInstance(mCameraID);

            try {
                if (mSurfaceHolder != null)
                    mCamera.setPreviewDisplay(mSurfaceHolder);
                if (mPreviewCallback != null)
                    mCamera.setPreviewCallback(mPreviewCallback);
            } catch (IOException e) {
                Log.d(TAG, "Error setting camera preview: " + e.getMessage());
                MessageBox.error(mActivity, "Error starting camera preview.");
                return;
            }

            Camera.Parameters parameters = mCamera.getParameters();

            // List sizes = parameters.getSupportedPreviewSizes();
            // Camera.Size result = null;
            // for (int i = 0; i < sizes.size(); ++i) {
            //     result = (Camera.Size)sizes.get(i);
            //     Log.d(TAG, "supported size: " + result.width + "x" + result.height);
            // }

            parameters.setPreviewFrameRate(30);
            // parameters.setPreviewSize(1920, 1080);
            // parameters.setPreviewSize(1280, 720);
            // parameters.setPreviewSize(1280, 720);
            // parameters.setPreviewSize(768, 432);
            // parameters.setPreviewSize(640, 480);
            // parameters.setPreviewSize(480, 320);
            // Log.d(TAG, "videoWidth = " + mVideoWidth + ", videoHeight = " + mVideoHeight);
            parameters.setPreviewSize(mVideoWidth, mVideoHeight);
            // parameters.setPreviewSize(800, 480); // (*)
            // parameters.setPreviewSize(240, 160);

            // Request the NV21 format (needed for the optimized native
            // conversion code). The NV21 format is guaranteed to be
            // supported.
            parameters.setPreviewFormat(ImageFormat.NV21);

            try {
                mCamera.setParameters(parameters);
            } catch (RuntimeException e) {
                Log.d(TAG, "Error settings camera parameters: ", e);
                MessageBox.error(mActivity, "Error settings camera parameters.");
                return;
            }

            try {
                mCamera.startPreview();
            } catch (Exception e){
                Log.d(TAG, "Error starting camera preview: " + e.getMessage());
                MessageBox.error(mActivity, "Error starting camera preview.");
                return;
            }
        }
    }

    public void surfaceCreated(SurfaceHolder holder) {
        // The Surface has been created, now tell the camera where to draw the preview.
        try {
            if (mCamera != null) {
                mSurfaceHolder = holder;
                mCamera.setPreviewDisplay(mSurfaceHolder);
                if (mPreviewCallback != null) {
                    Log.d(TAG, "mCamera.setPreviewCallback");
                    mCamera.setPreviewCallback(mPreviewCallback);
                }
            }
        } catch (IOException e) {
            Log.d(TAG, "Error setting camera preview: " + e.getMessage());
            MessageBox.error(mActivity, "Error starting camera preview.");
        }
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        //
    }

    @SuppressWarnings("deprecation")
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        Log.d(TAG, "surfaceChanged");
        // If your preview can change or rotate, take care of those events here.
        // Make sure to stop the preview before resizing or reformatting it.

        if (mHolder.getSurface() == null) {
            // preview surface does not exist
            return;
        }

        if (mCamera == null) {
            return;
        }

        // stop preview before making changes
        try {
            mCamera.stopPreview();
        } catch (Exception e) {
            // ignore: tried to stop a non-existent preview
        }

        // GS4m??
        try {
            mCamera.setPreviewDisplay(holder);
            if (mPreviewCallback != null)
                mCamera.setPreviewCallback(mPreviewCallback);
        } catch (IOException e) {
            Log.d(TAG, "exception", e);
        }

        // set preview size and make any resize, rotate or
        // reformatting changes here

        Camera.Parameters parameters = mCamera.getParameters();

        // List sizes = parameters.getSupportedPictureSizes();
        // Camera.Size result = null;
        // for (int i = 0; i < sizes.size(); ++i) {
        //     result = (Camera.Size)sizes.get(i);
        //     Log.d(TAG, "supported size: " + result.width + "x" + result.height);
        // }

        parameters.setPreviewFrameRate(30);
        // parameters.setPreviewSize(1920, 1080);
        // parameters.setPreviewSize(1280, 720);
        // parameters.setPreviewSize(1280, 720);
        // parameters.setPreviewSize(768, 432);
        // parameters.setPreviewSize(640, 480);
        // parameters.setPreviewSize(480, 320);
        parameters.setPreviewSize(mVideoWidth, mVideoHeight);
        // parameters.setPreviewSize(800, 480); // (*)
        // parameters.setPreviewSize(240, 160);

        // Request the NV21 format (needed for the optimized native
        // conversion code). The NV21 format is guaranteed to be
        // supported.
        parameters.setPreviewFormat(ImageFormat.NV21);

        // for (Integer format : parameters.getSupportedPreviewFormats()) {
        //     Log.d(TAG, "supported format: " + format);
        // }

        // parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);

        parameters.setRecordingHint(true);
        // parameters.setExposureCompensation(0);

        try {
            mCamera.setParameters(parameters);
		} catch (RuntimeException e) {
            Log.d(TAG, "Error settings camera parameters: ", e);
            MessageBox.error(mActivity, "Error settings camera parameters.");
		}

        // NativeApp.initCamera(previewSize.width, previewSize.height);
        if (mSizeCallback != null) {
            Camera.Size previewSize = parameters.getPreviewSize();
            mSizeCallback.previewSizeChanged(previewSize.width, previewSize.height);
        }

        // start preview with new settings
        try {
            Log.d(TAG, "startPreview");
            mCamera.startPreview();
        } catch (Exception e){
            Log.d(TAG, "Error starting camera preview: " + e.getMessage());
            MessageBox.error(mActivity, "Error starting camera preview.");
        }

        if (QCAR.isInitialized() && com.qualcomm.ar.pl.CameraPreview.instance != null) {
            // CameraPreview.instance.setCamera(mCamera);
            com.qualcomm.ar.pl.CameraPreview.instance.setCamera(mCamera);
            FluidMechanics.initQCAR(); // XXX: FluidMechanics => NativeApp
        }
    }
}
