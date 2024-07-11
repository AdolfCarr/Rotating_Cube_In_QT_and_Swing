package com.mycompany.rotating3dcube;

import java.awt.Canvas;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

/**
 *
 * @author Adolfo Carrillo
 */
public class Rotating3DCube extends Frame{
    public static void main(String[] args) {
        Rotating3DCube rotating3DCube = new Rotating3DCube();
    }

    Rotating3DCube() {
        super("Rotating3DCube_Java");
        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                System.exit(0);
            }
        });
        add("Center", new CvCubRot());
        Dimension dim = getToolkit().getScreenSize();
        setSize(dim.width / 2, dim.height / 2);
        setLocation(dim.width / 4, dim.height / 4);
        setVisible(true);
    }
}

class CvCubRot extends Canvas implements Runnable {

    int centerX, centerY, w, h;
    Obj5 obj = new Obj5();
    Image image;
    Graphics gImage;
    double alpha = 0;
    Thread thr = new Thread(this);

    @Override
    public void run() {
        try {
            for (;;) {
                alpha += 0.01;
                repaint();
                Thread.sleep(20);
            }
        } catch (InterruptedException e) {
        }
    }

    CvCubRot() {
        thr.start();
    }

    @Override
    public void update(Graphics g) {
        paint(g);
    }

    int iX(float x) {
        return Math.round(centerX + x);
    }

    int iY(float y) {
        return Math.round(centerY - y);
    }

    void line(int i, int j) {
        Point2D P = obj.vScr[i], Q = obj.vScr[j];
        gImage.drawLine(iX(P.x), iY(P.y), iX(Q.x), iY(Q.y));
    }

    @Override
    public void paint(Graphics g) {
        Dimension dim = getSize();
        int maxX = dim.width - 1, maxY = dim.height - 1;
        centerX = maxX / 2;
        centerY = maxY / 2;
        int minMaxXY = Math.min(maxX, maxY);
        obj.d = obj.rho * minMaxXY / obj.objSize;
        obj.rotateCube(alpha);
        obj.eyeAndScreen();
        if (w != dim.width || h != dim.height) {
            w = dim.width;
            h = dim.height;
            image = createImage(w, h);
            gImage = image.getGraphics();
        }
        gImage.clearRect(0, 0, w, h);
// Horizontal edges at the bottom:
        line(0, 1);
        line(1, 2);
        line(2, 3);
        line(3, 0);
// Horizontal edges at the top:
        line(4, 5);
        line(5, 6);
        line(6, 7);
        line(7, 4);
// Vertical edges:
        line(0, 4);
        line(1, 5);
        line(2, 6);
        line(3, 7);

        g.drawImage(image, 0, 0, null);
    }
}

class Obj5 { // Contains 3D object data for a cube

    //float rho, theta = 0F, phi = 1.3F, d;
    float rho, theta = 0.3F, phi = 1.3F, d;
    Point3D[] s, w; // World coordinates
    Point2D[] vScr; // Screen coordinates
    float v11, v12, v13, v21, v22, v23,
            v32, v33, v43, // Elements of viewing matrix V.
            xe, ye, ze, objSize = 5;//8

    Obj5() {
        s = new Point3D[8]; // Start situation
        w = new Point3D[8]; // After rotation
        vScr = new Point2D[8];
// Bottom surface:
        s[0] = new Point3D(1, -1, -1);
        s[1] = new Point3D(1, 1, -1);
        s[2] = new Point3D(-1, 1, -1);
        s[3] = new Point3D(-1, -1, -1);
// Top surface:
        s[4] = new Point3D(1, -1, 1);
        s[5] = new Point3D(1, 1, 1);
        s[6] = new Point3D(-1, 1, 1);
        s[7] = new Point3D(-1, -1, 1);

        rho = 15; // For reasonable perspective effect
    }

    void rotateCube(double alpha) {
        Rota3D.initRotate(s[1], s[7], alpha);//Ideal rotation axis s[1] s[7], or s[0] s[4](rotation over vertical axis),s[0], s[1](rotation over horizontal axis downwards), or s[7], s[1](rotation over diagonal axis to left downward),or s[6] s[0](rotation over diagonal axis to left upward), Rotations about axis defined by the 3D points s[r], s[s].
        for (int i = 0; i < 8; i++) {
            w[i] = Rota3D.rotate(s[i]);
        }
    }

    void initPersp() {
        float costh = (float) Math.cos(theta),
                sinth = (float) Math.sin(theta),
                cosph = (float) Math.cos(phi),
                sinph = (float) Math.sin(phi);
        v11 = -sinth;
        v12 = -cosph * costh;
        v13 = sinph * costh;
        v21 = costh;
        v22 = -cosph * sinth;
        v23 = sinph * sinth;
        v32 = sinph;
        v33 = cosph;
        v43 = -rho;
    }

    void eyeAndScreen() {
        initPersp();
        for (int i = 0; i < 8; i++) {
            Point3D P = w[i];
            float x = v11 * P.x + v21 * P.y;
            float y = v12 * P.x + v22 * P.y + v32 * P.z;
            float z = v13 * P.x + v23 * P.y + v33 * P.z + v43;
            Point3D Pe = new Point3D(x, y, z);
            vScr[i] = new Point2D(-d * Pe.x / Pe.z, -d * Pe.y / Pe.z);
        }
    }
}
    

