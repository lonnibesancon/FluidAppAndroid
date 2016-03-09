package com.qualcomm.ar.pl;

import android.content.Context;
import android.hardware.Camera;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import java.io.IOException;

class CameraSurface extends SurfaceView
    implements SurfaceHolder.Callback
{
    Camera camera;
    SurfaceHolder surfaceHolder;
    private static final String MODULENAME = "CameraSurface";

    public CameraSurface(Context context)
    {
        super(context);

        this.camera = null;

        this.surfaceHolder = getHolder();
        this.surfaceHolder.addCallback(this);
        this.surfaceHolder.setType(3);
    }

    public void setCamera(Camera cam)
    {
        this.camera = cam;
    }

    public void surfaceCreated(SurfaceHolder holder)
    {
        try
        {
            if (this.camera != null)
            {
                this.camera.setPreviewDisplay(holder);
            }

        }
        catch (IOException exception)
        {
            this.camera = null;
        }
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
    {
    }

    public void surfaceDestroyed(SurfaceHolder holder)
    {
        this.camera = null;
    }
}
