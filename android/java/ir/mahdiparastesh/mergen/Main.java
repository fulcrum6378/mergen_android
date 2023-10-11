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
import android.widget.PopupMenu;
import android.widget.RelativeLayout;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Objects;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

@SuppressLint("ClickableViewAccessibility")
public class Main extends Activity implements TextureView.SurfaceTextureListener,
        View.OnLongClickListener {
    private long ndkCamera;
    private Surface surface = null;
    private boolean isRecording = false, isDebugging = false, isFinished = true;
    private static Handler handler;
    private Toast toast;
    private Shaker shaker;
    private ObjectAnimator captureAnimation;

    private RelativeLayout root;
    private View colouring;
    private TextureView preview;
    private View capture, popupAnchor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        root = findViewById(R.id.root);
        colouring = findViewById(R.id.colouring);
        preview = findViewById(R.id.preview);
        capture = findViewById(R.id.capture);
        popupAnchor = findViewById(R.id.popupAnchor);

        // ask for camera and microphone permissions
        String[] requiredPerms =
                new String[]{Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO};
        if (Arrays.stream(requiredPerms).anyMatch(s -> checkSelfPermission(s) < 0))
            requestPermissions(requiredPerms, 1);
        else permitted();

        // handler
        handler = new Handler(Looper.getMainLooper()) {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    // Called by vis/Camera whenever a new frame is captured for analysis or mere saving.
                    case 0:
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
                        break;
                    // Called by vis/Segmentation when it's done saving data.
                    case 1:
                        isFinished = true;
                        Toast.makeText(Main.this, "You can now close the app safely.",
                                Toast.LENGTH_SHORT).show();
                        break;
                }
            }
        };

        // miscellaneus jobs
        shaker = new Shaker();
        root.setOnLongClickListener(this);

        // toast any test string values
        String tested = test();
        if (tested != null) Toast.makeText(this, tested, Toast.LENGTH_LONG).show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (Arrays.stream(grantResults).sum() == 0) permitted();
        else onBackPressed();
    }


    private void permitted() {
        ndkCamera = create();
        Size size = getCameraDimensions(ndkCamera);
        /*Toast.makeText(this, size.getWidth() + " : " + size.getHeight(),
                Toast.LENGTH_SHORT).show();*/
        onRecordingStopped();
        ViewGroup.LayoutParams previewLP = preview.getLayoutParams();
        previewLP.width = size.getWidth();
        previewLP.height = size.getHeight();
        preview.setLayoutParams(previewLP);
        preview.setSurfaceTextureListener(this);
        if (preview.isAvailable()) //noinspection DataFlowIssue
            onSurfaceTextureAvailable(preview.getSurfaceTexture(), size.getWidth(), size.getHeight());
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
        surfaceTexture.setDefaultBufferSize(width, height);
        surface = new Surface(surfaceTexture);
        onSurfaceStatusChanged(ndkCamera, surface, true);
        root.setClickable(true);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        root.setClickable(false);
        recording(false);
        onSurfaceStatusChanged(ndkCamera, surface, false);
        ndkCamera = 0;
        surface = null;
        return true;
    }

    @Override
    public boolean onLongClick(View v) {
        PopupMenu popup = new PopupMenu(this, popupAnchor);
        popup.inflate(R.menu.main);
        popup.setOnMenuItemClickListener(item -> {
            if (item.getItemId() == R.id.debug) {
                isDebugging = !isDebugging;
                // TODO
                return true;
            }
            if (item.getItemId() == R.id.export) try {
                FileOutputStream fos = new FileOutputStream(new File(getCacheDir(), "memory.zip"));
                ZipOutputStream zos = new ZipOutputStream(fos);
                addDirToZip(zos, getFilesDir(), null);
                zos.flush();
                fos.flush();
                zos.close();
                fos.close();
                return true;
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
            return false;
        });
        popup.getMenu().findItem(R.id.debug).setChecked(isDebugging);
        popup.show();
        return true;
    }

    public static void addDirToZip(ZipOutputStream zos, File fileToZip, String parentDirName)
            throws IOException {
        if (fileToZip == null || !fileToZip.exists()) return;

        String zipEntryName = fileToZip.getName();
        if (parentDirName != null && !parentDirName.isEmpty())
            zipEntryName = parentDirName + "/" + fileToZip.getName();

        if (fileToZip.isDirectory()) {
            for (File file : Objects.requireNonNull(fileToZip.listFiles()))
                addDirToZip(zos, file, zipEntryName);
        } else {
            byte[] buffer = new byte[1024];
            FileInputStream fis = new FileInputStream(fileToZip);
            zos.putNextEntry(new ZipEntry(zipEntryName));
            int length;
            while ((length = fis.read(buffer)) > 0)
                zos.write(buffer, 0, length);
            zos.closeEntry();
            fis.close();
        }
    }

    private void recording(boolean bb) {
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
        shaker.amplitude = amplitude;
    }

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
        surface.release();
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

    private class Shaker extends Thread { // FIXME THIS SUCKS
        public int amplitude = 0;
        private boolean active = true;
        private final Vibrator vib = (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) ?
                ((VibratorManager) getSystemService(Context.VIBRATOR_MANAGER_SERVICE))
                        .getDefaultVibrator() : (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);

        public Shaker() {
            start();
        }

        @Override
        public void run() {
            long dur = 200;
            while (active) {
                if (amplitude > 0) vib.vibrate(VibrationEffect.createOneShot(dur, amplitude));
                try {
                    //noinspection BusyWait
                    Thread.sleep(dur);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }
        }

        @Override
        public void interrupt() {
            active = false;
            super.interrupt();
        }
    }


    /** Structs utilities required for recording. */
    private native long create();

    /** Starts recording. */
    private native byte start();

    /** Stops recording. */
    private native byte stop();

    /** Destructs utilities required for recording. */
    private native void destroy();

    /** Shows test string values coming from C++ */
    private native String test();

    /** Camera cannot record in any dimension you want! */
    private native Size getCameraDimensions(long cameraObj);

    /** Lets Camera be created or destroyed. */
    private native void onSurfaceStatusChanged(long cameraObj, Surface surface, boolean available);

    /** Sends specific touch events. */
    private native void onTouch(int dev, int act, int id, float x, float y, float size);

    static {
        System.loadLibrary("main");
    }
}
