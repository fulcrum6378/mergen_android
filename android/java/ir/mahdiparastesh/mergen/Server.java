package ir.mahdiparastesh.mergen;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;

public class Server extends Thread {
    boolean active = true;

    @Override
    public void run() {
        try {
            ServerSocket server = new ServerSocket(3772);
            while (active) {
                Socket connection = server.accept(); // blocks until something is received...
                InputStream in = connection.getInputStream();
                OutputStream out = connection.getOutputStream();
                switch (in.read()) {
                    case 0:
                        break;
                }
            }
            server.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
