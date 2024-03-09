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

/**
 * A server-client approach for controlling and debugging from another computer.
 *
 * @noinspection ResultOfMethodCallIgnored, RedundantSuppression
 */
public class Bridge extends Thread {
    boolean active = true, recorded = true;
    final Main c;

    public Bridge(Main c) {
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
            if (mode > 10 && mode <= 20) { // those which require recording to be started
                out.write(0);
                recorded = false;
                Main.handler.obtainMessage(127).sendToTarget();
                try {
                    while (!recorded) //noinspection BusyWait
                        Thread.sleep(200);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }
            switch (mode) {
                case 1 -> { // START
                    if (!c.isRecording) {
                        Main.handler.obtainMessage(127).sendToTarget();
                        out.write(0);
                    } else out.write(1);
                }
                case 2 -> { // STOP
                    if (c.isRecording) {
                        Main.handler.obtainMessage(126).sendToTarget();
                        out.write(0);
                    } else out.write(1);
                }
                case 3 -> { // EXIT
                    if (c.isFinished) {
                        Main.handler.obtainMessage(125).sendToTarget();
                        out.write(0);
                    } else out.write(1);
                }
                case 10 -> { // wipe visual STM
                    truncateDir(new File(c.getFilesDir(), "vis/stm/shapes"));
                    truncateDir(new File(c.getFilesDir(), "vis/stm/y"));
                    truncateDir(new File(c.getFilesDir(), "vis/stm/u"));
                    truncateDir(new File(c.getFilesDir(), "vis/stm/v"));
                    truncateDir(new File(c.getFilesDir(), "vis/stm/r"));
                    new File(c.getFilesDir(), "vis/stm/frames").delete();
                    new File(c.getFilesDir(), "vis/stm/numbers").delete();
                    out.write(0);
                }
                case 11 -> { // send `arr` and `b_status`
                    Files.copy(new File(c.getCacheDir(), "arr").toPath(), out);
                    Files.copy(new File(c.getCacheDir(), "b_status").toPath(), out);
                }
                case 21 -> { // send `stm.zip`
                    if (!c.isFinished) {
                        out.write(1);
                        break;
                    }
                    out.write(0);
                    ZipOutputStream stm = new ZipOutputStream(out);
                    addFileToZip(stm, new File(c.getFilesDir(), "vis/stm/shapes"), null);
                    addFileToZip(stm, new File(c.getFilesDir(), "vis/stm/y"), null);
                    addFileToZip(stm, new File(c.getFilesDir(), "vis/stm/u"), null);
                    addFileToZip(stm, new File(c.getFilesDir(), "vis/stm/v"), null);
                    addFileToZip(stm, new File(c.getFilesDir(), "vis/stm/r"), null);
                    addFileToZip(stm, new File(c.getFilesDir(), "vis/stm/frames"), null);
                    addFileToZip(stm, new File(c.getFilesDir(), "vis/stm/numbers"), null);
                    stm.flush();
                    stm.close(); // both necessary!
                }
                case 22 -> { // send 'mem.zip'
                    out.write(0);
                    ZipOutputStream ltm = new ZipOutputStream(out);
                    addFileToZip(ltm, new File(c.getFilesDir(), "vis/mem/shapes"), null);
                    addFileToZip(ltm, new File(c.getFilesDir(), "vis/mem/y"), null);
                    addFileToZip(ltm, new File(c.getFilesDir(), "vis/mem/u"), null);
                    addFileToZip(ltm, new File(c.getFilesDir(), "vis/mem/v"), null);
                    addFileToZip(ltm, new File(c.getFilesDir(), "vis/mem/r"), null);
                    ltm.flush();
                    ltm.close();
                }
                default -> out.write(255);
            }
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

    /** Adds a File to a ZipOutputStream. */
    public void addFileToZip(ZipOutputStream zos, File fileToZip, String parentDirName)
            throws IOException {
        if (fileToZip == null || !fileToZip.exists()) return;

        String zipEntryName = fileToZip.getName();
        if (parentDirName != null && !parentDirName.isEmpty())
            zipEntryName = parentDirName + "/" + fileToZip.getName();

        if (fileToZip.isDirectory()) {
            for (File file : Objects.requireNonNull(fileToZip.listFiles()))
                addFileToZip(zos, file, zipEntryName);
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

    /** Removes all files in a directory (assuming there are no other sub-directories). */
    public void truncateDir(File folder) {
        if (!folder.exists()) return;
        File[] files = folder.listFiles(); // some JVMs return null for empty dirs
        if (files != null) for (File f : files) f.delete();
    }

    @Override
    public void interrupt() {
        active = false;
        super.interrupt();
    }
}
