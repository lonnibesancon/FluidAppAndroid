package fr.limsi.ARViewer;

public class NativeApp {
    static {
        // All shared library dependencies must be loaded in
        // correct order
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("artoolkit");
        System.loadLibrary("native");
    }

    public static final int APP_TYPE_STUB    = 0;
    public static final int APP_TYPE_FLUID   = 1;

    // NOTE: must be called first
    public static native boolean init(
        int appType, // APP_* constant
        String baseDir/*,
        int videoWidth, int videoHeight*/
    );

    // (GL context)
    public static native void rebind();

    // (GL context)
    public static native void reshape(int width, int height);

    // // NOTE: blocks until the frame has been fully processed
    // public static native boolean processVideoFrame(byte[] in);

    // (GL context)
    public static native void render();

    static class Settings {
        float zoomFactor;
        boolean showCamera;
    }

    static class State {
        // ...
    }

    public static native void getSettings(NativeApp.Settings settings);
    public static native void setSettings(NativeApp.Settings settings);

    public static native void getState(NativeApp.State state);
}
