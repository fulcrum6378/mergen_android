package ir.mahdiparastesh.mergen;

import android.Manifest;
import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;

import java.util.Arrays;

public class Main extends Activity implements TextureView.SurfaceTextureListener {
    private final int permCode = 372;
    private final String[] requiredPerms =
            new String[]{Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO};
    private DisplayMetrics dm;
    private long ndkCamera;
    private TextureView textureView;
    private Surface surface = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        textureView = findViewById(R.id.texturePreview);
        dm = getResources().getDisplayMetrics();

        if (Arrays.stream(requiredPerms).anyMatch(s -> checkSelfPermission(s) < 0))
            requestPermissions(requiredPerms, permCode);
        else preview();
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
        textureView.setSurfaceTextureListener(this); // don't make it in-line.
        if (textureView.isAvailable())
            onSurfaceTextureAvailable(textureView.getSurfaceTexture(),
                    textureView.getWidth(), textureView.getHeight());
        createCamera(dm.widthPixels, dm.heightPixels);
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
        ndkCamera = createCamera(dm.widthPixels, dm.heightPixels);
        Size cameraPreviewSize = getMinimumCompatiblePreviewSize(ndkCamera);

        surfaceTexture.setDefaultBufferSize(
                cameraPreviewSize.getWidth(), cameraPreviewSize.getHeight());
        surface = new Surface(surfaceTexture);
        onPreviewSurfaceCreated(ndkCamera, surface);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        onPreviewSurfaceDestroyed(ndkCamera, surface);
        deleteCamera(ndkCamera, surface);
        ndkCamera = 0;
        surface = null;
        return true;
    }

    @Override
    protected void onDestroy() {
        surface.release();
        super.onDestroy();
    }

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
