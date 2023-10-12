package ir.mahdiparastesh.mergen;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Objects;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

public class Debug extends Thread {
    boolean active = true;
    Main c;

    public Debug(Main c) {
        this.c = c;
    }

    @Override
    public void run() {
        try {
            ServerSocket server = new ServerSocket(3772);
            while (active) {
                Socket con = server.accept(); // blocks until something is received...
                InputStream in = con.getInputStream();
                OutputStream out = con.getOutputStream();

                byte mode = (byte) in.read();
                boolean process = true;
                if (mode <= 1) { // those which require recording to be started
                    if (c.isFinished) c.recording(true, mode);
                    else {
                        process = false;
                        continue;
                    }
                }
                byte[] res;
                FileOutputStream cache;
                if (process) switch (mode) {
                    case 1:
                        // TODO e
                        cache = new FileOutputStream(
                                new File(c.getCacheDir(), "arr.bin"));

                        cache.close();
                        res = new byte[]{0};
                        break;
                    case 2:
                        cache = new FileOutputStream(
                                new File(c.getCacheDir(), "memory.zip"));
                        ZipOutputStream zos = new ZipOutputStream(cache);
                        addDirToZip(zos, c.getFilesDir(), null);
                        zos.flush();
                        cache.flush();
                        zos.close();
                        cache.close();

                        res = new byte[]{0};
                        break;
                    default:
                        res = new byte[]{2};
                }
                else res = new byte[]{1};
                out.write(res);
                out.flush();
                con.close();
            }
            server.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
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

    @Override
    public void interrupt() {
        active = false;
        super.interrupt();
    }
}
