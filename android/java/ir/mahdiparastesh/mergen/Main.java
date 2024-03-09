package ir.mahdiparastesh.mergen;

import android.Manifest;
import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ObjectAnimator;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.os.VibratorManager;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.Toast;

import java.util.Arrays;

/** The main and only activity of the application */
@SuppressLint("ClickableViewAccessibility")
public class Main extends Activity {
    private RelativeLayout root, previewBox, segMarkerPool;
    private View colouring, capture;
    private TextureView preview, analyses;

    static Handler handler;
    private Vibrator vib;
    private Bridge bridge;
    private SegmentMarkers segMarker;
    private Surface previewSurface = null, analysesSurface = null;
    boolean isRecording = false, isFinished = true;
    private Toast toast;
    private ObjectAnimator captureAnimation;
    private Thread shaker = null;
    private int shakeAmplitude = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        root = findViewById(R.id.root);
        colouring = findViewById(R.id.colouring);
        previewBox = findViewById(R.id.previewBox);
        preview = findViewById(R.id.preview);
        analyses = findViewById(R.id.analyses);
        segMarkerPool = findViewById(R.id.segMarkerPool);
        capture = findViewById(R.id.capture);

        // ask for camera and microphone permissions
        String[] requiredPerms =
                new String[]{Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO};
        if (Arrays.stream(requiredPerms).anyMatch(s -> checkSelfPermission(s) < 0))
            requestPermissions(requiredPerms, 1);
        else prepare();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (Arrays.stream(grantResults).sum() == 0) prepare();
        else onBackPressed();
    }

    private void prepare() {
        // JNI-related jobs
        handler = new Handler(Looper.getMainLooper()) {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    // C++ signals:
                    case 0 -> { // By vis/Camera.cpp whenever a new frame is captured.
                        capture.setAlpha(.9f);
                        captureAnimation = ObjectAnimator.ofFloat(capture, View.ALPHA, .9f, 0f)
                                .setDuration(200);
                        captureAnimation.addListener(new AnimatorListenerAdapter() {
                            @Override
                            public void onAnimationEnd(Animator animation) {
                                captureAnimation = null;
                            }
                        });
                        captureAnimation.start();
                    }
                    case 1 -> { // By vis/Segmentation.cpp when it's done saving data.
                        isFinished = true;
                        if (bridge.recorded) {
                            if (toast != null) toast.cancel();
                            toast = Toast.makeText(Main.this,
                                    "You can now close the app safely.", Toast.LENGTH_SHORT);
                            toast.show();
                        }
                    }
                    case 2 -> { // By vis/Segmentation.cpp to stop recording.
                        bridge.recorded = true;
                        recording(false);
                    }

                    // Java signals:
                    case 127 -> // By RemoteDebug.java to start recording.
                            recording(true);
                    case 126 -> // By RemoteDebug.java to stop recording.
                            recording(false);
                    case 125 -> // By RemoteDebug.java to close the app.
                            onBackPressed();
                    case 124 -> // For `updateSegMarkers()`
                            segMarker.update((long[]) msg.obj);
                }
            }
        };
        vib = (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) ?
                ((VibratorManager) getSystemService(Context.VIBRATOR_MANAGER_SERVICE))
                        .getDefaultVibrator() : (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);

        // initialise camera(s)
        create();
        Size size = getCameraDimensions();
        onRecordingStopped();
        ViewGroup.LayoutParams previewLP = previewBox.getLayoutParams();
        float sw = getResources().getDisplayMetrics().widthPixels,
                sh = getResources().getDisplayMetrics().heightPixels;
        if (sh > sw) { // portrait
            previewLP.width = (int) sw;
            previewLP.height = (int) ((sw / (float) size.getWidth()) * (float) size.getHeight());
        } else { // landscape or square
            previewLP.width = (int) ((sh / size.getHeight()) * (float) size.getWidth());
            previewLP.height = (int) sh;
        }
        previewBox.setLayoutParams(previewLP);
        preview.setSurfaceTextureListener(previewSurfaceListener);
        if (preview.isAvailable()) //noinspection DataFlowIssue
            previewSurfaceListener.onSurfaceTextureAvailable(
                    preview.getSurfaceTexture(), size.getWidth(), size.getHeight());

        // initialise a surface for visualising analyses
        analyses.setSurfaceTextureListener(analysesSurfaceListener);
        if (analyses.isAvailable()) //noinspection DataFlowIssue
            analysesSurfaceListener.onSurfaceTextureAvailable(
                    analyses.getSurfaceTexture(), size.getWidth(), size.getHeight());

        // initialise the Bridge
        bridge = new Bridge(this);
        bridge.start();

        // initialise the segment marker manager
        segMarker = new SegmentMarkers(this, segMarkerPool);
    }

    TextureView.SurfaceTextureListener previewSurfaceListener = new TextureView.SurfaceTextureListener() {
        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture st, int width, int height) {
            st.setDefaultBufferSize(width, height);
            previewSurface = new Surface(st);
            onPreviewSurfaceCreated(previewSurface);
            root.setClickable(true);
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture st, int width, int height) {
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture st) {
        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture st) {
            root.setClickable(false);
            recording(false);
            onPreviewSurfaceDestroyed();
            previewSurface = null;
            return true;
        }
    }, analysesSurfaceListener = new TextureView.SurfaceTextureListener() {
        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture st, int width, int height) {
            st.setDefaultBufferSize(width, height);
            analysesSurface = new Surface(st);
            onAnalysesSurfaceCreated(analysesSurface);
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture st, int width, int height) {
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture st) {
        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture st) {
            onAnalysesSurfaceDestroyed();
            analysesSurface = null;
            return true;
        }
    };

    /**
     * Starts/stops recording.
     * It must always be executed in the main thread.
     */
    void recording(boolean bb) {
        if (bb == isRecording) return;
        byte res;
        if (!isRecording) res = start();
        else res = stop();
        if (toast != null) toast.cancel();
        toast = Toast.makeText(this,
                (!isRecording ? "STARTED" : "STOPPED") + ": " + (int) res,
                Toast.LENGTH_SHORT);
        toast.show();
        if (res != 0) return;
        isRecording = bb;
        if (isRecording) onRecordingStarted();
        else onRecordingStopped();
    }

    /**
     * In my phone, `pressure` is always 1.0 and `orientation` always 0!
     * <p>
     * As long as the previous pointers (pointers with smaller indices) haven't got UP,
     * ANY ACTION_MOVE WILL BE COUNTED ON THE SMALLEST INDEX!
     *
     * @see <a href="https://stackoverflow.com/questions/28417492/android-multi-touch-pointers-with-
     * index-0-not-triggering-event-action-move">the problem</a>
     */
    @SuppressLint("ObsoleteSdkInt")
    @TargetApi(Build.VERSION_CODES.Q)
    private void onRecordingStarted() {
        root.setOnClickListener(null);
        root.setOnTouchListener((v, ev) -> {
            int mAct = ev.getActionMasked();
            if (mAct < 0 || mAct > 6) return false;
            int index = ev.getActionIndex();
            Main.this.onTouch(ev.getDeviceId(), mAct, ev.getPointerId(index),
                    ev.getRawX(index), ev.getRawY(index), ev.getSize(index));
            return true;
        });
        isFinished = false;
        root.setLongClickable(false);
    }

    private void onRecordingStopped() {
        root.setOnTouchListener(null);
        root.setOnClickListener(new DoubleClickListener() {
            @Override
            public void onDoubleClick() {
                recording(true);
            }
        });
        root.setLongClickable(true);
    }

    /** Transfers a signal from non-main threads of native codes to the main thread. */
    @SuppressWarnings("unused")
    void signal(byte id) {
        handler.obtainMessage(id).sendToTarget();
    }

    /** @param amplitude must be in range 1..255 */
    @SuppressWarnings("unused")
    void vibrate(int amplitude) {
        shakeAmplitude = amplitude;
        if (shaker != null) return;
        shaker = new Thread(() -> {
            while (shakeAmplitude != 0) {
                if (shakeAmplitude > 0)
                    vib.vibrate(VibrationEffect.createOneShot(200, shakeAmplitude));
                try { //noinspection BusyWait
                    Thread.sleep(200);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }
            shaker = null;
        });
        shaker.start();
    }

    /** Passes segments' data from C++ to SegmentMarkers. */
    @SuppressWarnings("unused")
    void updateSegMarkers(long[] data) {
        handler.obtainMessage(124, data).sendToTarget();
    }

    /** Effect for expressing simulated pleasure & pain. */
    @SuppressWarnings("unused")
    void colouring(int colour) {
        colouring.setBackgroundColor(colour);
    }

    @Override
    public void onBackPressed() {
        if (isRecording) {
            recording(false);
            return;
        }
        if (!isFinished) return;
        moveTaskToBack(true);
        android.os.Process.killProcess(android.os.Process.myPid());
        System.exit(0);
    }

    @Override
    protected void onDestroy() {
        if (shakeAmplitude != 0) shakeAmplitude = 0;
        bridge.interrupt();
        previewSurface.release();
        analysesSurface.release();
        destroy();
        super.onDestroy();
    }


    public abstract static class DoubleClickListener implements View.OnClickListener {
        private long lastClickAt = 0;

        @Override
        public void onClick(View v) {
            if ((SystemClock.elapsedRealtime() - lastClickAt) < 200) onDoubleClick();
            lastClickAt = SystemClock.elapsedRealtime();
        }

        public abstract void onDoubleClick();
    }


    /** Structs utilities required for recording. */
    private native void create();

    /** Starts recording. */
    private native byte start();

    /** Stops recording. */
    private native byte stop();

    /** Destructs utilities required for recording. */
    private native void destroy();


    /**
     * Finds the nearest possible dimensions to the ones requested in Segmentation.
     * Always call this after create().
     */
    private native Size getCameraDimensions();

    /** Lets ACaptureSession be created. */
    private native void onPreviewSurfaceCreated(Surface surface);

    /** Lets Camera be destroyed. */
    private native void onPreviewSurfaceDestroyed();

    /** Lets Analyses be created. */
    private native void onAnalysesSurfaceCreated(Surface surface);

    /** Lets Analyses be destroyed. */
    private native void onAnalysesSurfaceDestroyed();

    /** Sends specific touch events. */
    private native void onTouch(int dev, int act, int id, float x, float y, float size);

    static {
        System.loadLibrary("main");
    }
}
