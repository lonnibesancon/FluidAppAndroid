package fr.limsi.ARViewer;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.SeekBar;

public class VerticalSeekBar extends SeekBar {
    public VerticalSeekBar(Context context) {
        super(context);
    }

    public VerticalSeekBar(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public VerticalSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(h, w, oldh, oldw);
    }

    @Override
    protected synchronized void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(heightMeasureSpec, widthMeasureSpec);
        setMeasuredDimension(getMeasuredHeight(), getMeasuredWidth());
    }

    protected void onDraw(Canvas canvas) {
        canvas.rotate(-90);
        canvas.translate(-getHeight(), 0);
        super.onDraw(canvas);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (!isEnabled())
            return false;

        boolean result = false;

        if (event.getAction() != MotionEvent.ACTION_MOVE) {
            // Prevent duplicate haptic feedback events (due to
            // setProgress() below)
            boolean hf = isHapticFeedbackEnabled();
            setHapticFeedbackEnabled(false); {
                // This is required for OnSeekBarChangeListener.on{Start,Stop}TrackingTouch()
                result = super.onTouchEvent(event);
            } setHapticFeedbackEnabled(hf);
        }

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_MOVE:
                // Vertical scrolling
                setProgress(getMax() - (int)(getMax() * event.getY()/getHeight()));
                onSizeChanged(getWidth(), getHeight(), 0, 0);
                result = true;
                break;
        }

        return result;
    }

    public void customSetProgress(int progress) {
        setProgress(progress);
        onSizeChanged(getWidth(), getHeight(), 0, 0);
    }
}
