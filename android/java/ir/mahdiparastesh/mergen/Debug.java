package ir.mahdiparastesh.mergen;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.file.Files;
import java.util.Objects;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

public class Debug extends Thread {
    boolean active = true, recorded = true;
    Main c;

    public Debug(Main c) {
        this.c = c;
    }

    @Override
    public void run() {
        ServerSocket server;
        try {
            server = new ServerSocket(3772);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        while (active) try {
            Socket con = server.accept(); // blocks until something is received...
            InputStream in = con.getInputStream();
            OutputStream out = con.getOutputStream();

            byte mode = (byte) in.read();
            boolean process = true;
            if (mode <= 1) { // those which require recording to be started
                if (c.isFinished) {
                    recorded = false;
                    Main.handler.obtainMessage(127, mode).sendToTarget();
                } else {
                    process = false;
                    continue;
                }
            }
            if (process) switch (mode) {
                case 1:
                    out.write(0);
                    while (!recorded) try { //noinspection BusyWait
                        Thread.sleep(200);
                    } catch (InterruptedException e) {
                        throw new RuntimeException(e);
                    }
                    Files.copy(new File(c.getCacheDir(), "arr.bin").toPath(), out);
                    break;
                case 2:
                    out.write(0);
                    ZipOutputStream zos = new ZipOutputStream(out);
                    addDirToZip(zos, c.getFilesDir(), null);
                    break;
                default:
                    out.write(2);
            } else out.write(1);
            out.flush();
            con.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        try {
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
