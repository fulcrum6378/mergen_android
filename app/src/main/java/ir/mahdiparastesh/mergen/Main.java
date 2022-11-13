package ir.mahdiparastesh.mergen;

import android.Manifest;
import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;
import android.view.ViewGroup;
import android.widget.Toast;

import java.util.Arrays;

public class Main extends Activity implements TextureView.SurfaceTextureListener {
    private final int permCode = 372;
    private final String[] requiredPerms =
            new String[]{Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO};
    private DisplayMetrics dm;
    private long ndkCamera;
    private TextureView preview;
    private Surface surface = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        preview = findViewById(R.id.preview);
        dm = getResources().getDisplayMetrics();

        if (Arrays.stream(requiredPerms).anyMatch(s -> checkSelfPermission(s) < 0))
            requestPermissions(requiredPerms, permCode);
        else preview();

        String tested = test();
        if (tested != null) Toast.makeText(this, tested, Toast.LENGTH_LONG).show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == permCode) {
            if (Arrays.stream(grantResults).sum() == 0) preview();
            else onBackPressed();
        }
    }

    private void preview() {
        // Adjust the size of the TextureView and add click listeners
        int dim = Math.min(dm.widthPixels, dm.heightPixels);
        ViewGroup.LayoutParams previewLP = preview.getLayoutParams();
        previewLP.width = dim;
        previewLP.height = dim;
        preview.setLayoutParams(previewLP);
        preview.setOnClickListener(DoubleClickListener -> recording(!isRecording));

        preview.setSurfaceTextureListener(this); // don't make it in-line.
        if (preview.isAvailable()) onSurfaceTextureAvailable(
                preview.getSurfaceTexture(), preview.getWidth(), preview.getHeight());
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
        ndkCamera = createCamera(width, height);
        Size cameraPreviewSize = getMinimumCompatiblePreviewSize(ndkCamera);

        surfaceTexture.setDefaultBufferSize(
                cameraPreviewSize.getWidth(), cameraPreviewSize.getHeight());
        surface = new Surface(surfaceTexture);
        onPreviewSurfaceCreated(ndkCamera, surface);
        preview.setClickable(true);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        preview.setClickable(false);
        recording(false);
        onPreviewSurfaceDestroyed(ndkCamera, surface);
        deleteCamera(ndkCamera, surface);
        ndkCamera = 0;
        surface = null;
        return true;
    }

    private boolean isRecording = false;

    private void recording(boolean bb) {
        if (bb == isRecording) return;
        isRecording = bb;
        if (isRecording) startRecording();
        else stopRecording();
    }

    @Override
    public void onBackPressed() {
        if (isRecording) {
            recording(false);
            return;
        }
        super.onBackPressed();
    }

    @Override
    protected void onDestroy() {
        surface.release();
        super.onDestroy();
    }


    private native void startRecording();

    private native void stopRecording();

    private native String test();

    private native long createCamera(int width, int height);

    private native Size getMinimumCompatiblePreviewSize(long ndkCamera);

    @SuppressWarnings("unused")
    private native int getCameraSensorOrientation(long ndkCamera);

    private native void onPreviewSurfaceCreated(long ndkCamera, Surface surface);

    private native void onPreviewSurfaceDestroyed(long ndkCamera, Surface surface);

    private native void deleteCamera(long ndkCamera, Surface surface);

    static {
        System.loadLibrary("main");
    }
}
