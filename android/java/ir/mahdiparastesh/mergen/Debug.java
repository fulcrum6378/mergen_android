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
                //Log.println(Log.ASSERT, "ASAJJ", String.valueOf(in.read()));
                byte[] res;
                switch (in.read()) {
                    case 0:
                        res = new byte[]{0};
                        break;
                    case 1:
                        FileOutputStream fos = new FileOutputStream(
                                new File(c.getCacheDir(), "memory.zip"));
                        ZipOutputStream zos = new ZipOutputStream(fos);
                        addDirToZip(zos, c.getFilesDir(), null);
                        zos.flush();
                        fos.flush();
                        zos.close();
                        fos.close();

                        res = new byte[]{0};
                        break;
                    default:
                        res = new byte[]{1};
                }
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
