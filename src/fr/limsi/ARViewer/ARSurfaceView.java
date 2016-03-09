package fr.limsi.ARViewer;

import java.util.ArrayList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

import com.qualcomm.QCAR.QCAR;

public class ARSurfaceView extends GLSurfaceView {
    private static final String TAG = Config.APP_TAG;
    private static final boolean DEBUG = Config.DEBUG;

    private Renderer mRenderer;

    public ARSurfaceView(Context context) {
        super(context);
        init();
    }

    public ARSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        // Try to preserve the GL context when the application is paused.
        // This is NOT a strong guarantee, since it may not be
        // supported on some devices.
        setPreserveEGLContextOnPause(true);

        // Request a GLES 2.0 context
        setEGLContextClientVersion(2);
        // setEGLContextClientVersion(3);

        mRenderer = new Renderer();
        mRenderer.parentView = this;
        setRenderer(mRenderer);
        setRenderMode(RENDERMODE_WHEN_DIRTY);
        // setRenderMode(RENDERMODE_CONTINUOUSLY);

        // getHolder().setFixedSize(800, 480);
        // getHolder().setFixedSize(800, 460);
    }

    // @Override
    // public void onPause() {
    //     queueEvent(new Runnable() {
    //         public void run() {
    //             setRenderMode(RENDERMODE_WHEN_DIRTY);
    //         }});
    //     super.onPause();
    // }

    @Override
    public void queueEvent(Runnable r) {
        if (mRenderer.isInitialized)
            super.queueEvent(r);
        else
            mRenderer.queuePostInitEvent(r);
    }

    class Renderer implements GLSurfaceView.Renderer {
        GLSurfaceView parentView;
        boolean isInitialized = false;
        private ArrayList<Runnable> mPostInitRunnables = new ArrayList<Runnable>();

        synchronized void queuePostInitEvent(Runnable runnable) {
            mPostInitRunnables.add(runnable);
        }

        // NOTE: this method is called when the GL context is created OR
        // *recreated*. This means that all existing GL objects have been
        // invalidated, and must be recreated inside the new context.
        public void onSurfaceCreated(GL10 unused, EGLConfig config) {
            NativeApp.rebind();
            // QCAR.onSurfaceCreated();
            isInitialized = true;

            while (mPostInitRunnables.size() > 0)
                mPostInitRunnables.remove(0).run();
        }

        // Called when the surface size is changed (or first created).
        public void onSurfaceChanged(GL10 unused, int width, int height) {
            NativeApp.reshape(width, height);
            // QCAR.onSurfaceChanged(width, height);
        }

        public void onDrawFrame(GL10 unused) {
            NativeApp.render();
        }
    }
}
