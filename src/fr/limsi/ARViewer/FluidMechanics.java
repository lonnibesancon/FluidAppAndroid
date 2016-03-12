package fr.limsi.ARViewer;

public class FluidMechanics {
    public static final int SLICE_CAMERA = 0;
    public static final int SLICE_AXIS   = 1;
    public static final int SLICE_STYLUS = 2;

    static class Settings {
        boolean showVolume, showSurface, showStylus, showSlice, showCrossingLines, showOutline;
        int sliceType; // SLICE_* constant
        float clipDist; // if clipDist == 0, the clip plane is disabled
        double surfacePercentage;
        boolean surfacePreview;
        boolean surfacePreviewAtPoint;
        float precision ;
        int considerX ;
        int considerY ;
        int considerZ ;
        int considerRotation ;
        int considerTranslation ;
    }

    static class State {
        boolean tangibleVisible;
        boolean stylusVisible;
        float computedZoomFactor;
    }

    public static native boolean loadDataset(String filename); // VTK format
    public static native boolean loadVelocityDataset(String filename); // VTK format

    public static native void initQCAR();

    public static native void releaseParticles();

    public static native void buttonPressed();
    public static native float buttonReleased();

    public static native void getSettings(FluidMechanics.Settings settings);
    public static native void setSettings(FluidMechanics.Settings settings);
    public static native void setTangoValues(double tx, double ty, double tz, double rx, double ry, double rz, double q);
    public static native void setGyroValues(double rx, double ry, double rz, double q);
    public static native String getData();
    public static native void setInteractionMode(int mode);
    public static native void updateFingerPositions(float x, float y, int fingerID);
    public static native void addFinger(float x, float y, int fingerID);
    public static native void removeFinger(int fingerID);
    public static native void getState(FluidMechanics.State state);
}
