#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QPainter>
#include <cmath>

class Point2D {
public:
    float x, y;
    Point2D(float x, float y) : x(x), y(y) {}
};

class Point3D {
public:
    float x, y, z;
    Point3D(double x = 0, double y = 0, double z = 0) : x(static_cast<float>(x)), y(static_cast<float>(y)), z(static_cast<float>(z)) {}
};

class Rota3D {
public:
    static double r11, r12, r13, r21, r22, r23;
    static double r31, r32, r33, r41, r42, r43;

    static void initRotate(const Point3D& a, const Point3D& b, double alpha) {
        double v1 = b.x - a.x, v2 = b.y - a.y, v3 = b.z - a.z,
            theta = std::atan2(v2, v1),
            phi = std::atan2(std::sqrt(v1 * v1 + v2 * v2), v3);
        initRotate(a, theta, phi, alpha);
    }

    static void initRotate(const Point3D& a, double theta, double phi, double alpha) {
        double cosAlpha = std::cos(alpha), sinAlpha = std::sin(alpha),
            cosPhi = std::cos(phi), sinPhi = std::sin(phi),
            cosTheta = std::cos(theta), sinTheta = std::sin(theta),
            cosPhi2 = cosPhi * cosPhi, sinPhi2 = sinPhi * sinPhi,
            cosTheta2 = cosTheta * cosTheta, sinTheta2 = sinTheta * sinTheta,
            c = 1.0 - cosAlpha;

        r11 = cosTheta2 * (cosAlpha * cosPhi2 + sinPhi2) + cosAlpha * sinTheta2;
        r12 = sinAlpha * cosPhi + c * sinPhi2 * cosTheta * sinTheta;
        r13 = sinPhi * (cosPhi * cosTheta * c - sinAlpha * sinTheta);
        r21 = sinPhi2 * cosTheta * sinTheta * c - sinAlpha * cosPhi;
        r22 = sinTheta2 * (cosAlpha * cosPhi2 + sinPhi2) + cosAlpha * cosTheta2;
        r23 = sinPhi * (cosPhi * sinTheta * c + sinAlpha * cosTheta);
        r31 = sinPhi * (cosPhi * cosTheta * c + sinAlpha * sinTheta);
        r32 = sinPhi * (cosPhi * sinTheta * c - sinAlpha * cosTheta);
        r33 = cosAlpha * sinPhi2 + cosPhi2;
        r41 = a.x - a.x * r11 - a.y * r21 - a.z * r31;
        r42 = a.y - a.x * r12 - a.y * r22 - a.z * r32;
        r43 = a.z - a.x * r13 - a.y * r23 - a.z * r33;
    }

    static Point3D rotate(const Point3D& p) {
        return Point3D(
            p.x * r11 + p.y * r21 + p.z * r31 + r41,
            p.x * r12 + p.y * r22 + p.z * r32 + r42,
            p.x * r13 + p.y * r23 + p.z * r33 + r43
            );
    }
};

double Rota3D::r11, Rota3D::r12, Rota3D::r13, Rota3D::r21, Rota3D::r22, Rota3D::r23;
double Rota3D::r31, Rota3D::r32, Rota3D::r33, Rota3D::r41, Rota3D::r42, Rota3D::r43;

class Obj5 {
public:
    float rho, theta = 0.3F, phi = 1.3F, d;
    Point3D s[8], w[8];
    Point2D* vScr[8];
    float v11, v12, v13, v21, v22, v23, v32, v33, v43;
    float xe, ye, ze, objSize = 5;

    Obj5() {
        s[0] = Point3D(1, -1, -1);
        s[1] = Point3D(1, 1, -1);
        s[2] = Point3D(-1, 1, -1);
        s[3] = Point3D(-1, -1, -1);
        s[4] = Point3D(1, -1, 1);
        s[5] = Point3D(1, 1, 1);
        s[6] = Point3D(-1, 1, 1);
        s[7] = Point3D(-1, -1, 1);
        rho = 15;
    }

    void rotateCube(double alpha) {
        Rota3D::initRotate(s[1], s[7], alpha);
        for (int i = 0; i < 8; ++i) {
            w[i] = Rota3D::rotate(s[i]);
        }
    }

    void initPersp() {
        float costh = std::cos(theta), sinth = std::sin(theta),
            cosph = std::cos(phi), sinph = std::sin(phi);
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
        for (int i = 0; i < 8; ++i) {
            Point3D& P = w[i];
            float x = v11 * P.x + v21 * P.y;
            float y = v12 * P.x + v22 * P.y + v32 * P.z;
            float z = v13 * P.x + v23 * P.y + v33 * P.z + v43;
            Point3D Pe = Point3D(x, y, z);
            vScr[i] = new Point2D(-d * Pe.x / Pe.z, -d * Pe.y / Pe.z);
        }
    }
};

class CvCubRot : public QWidget {
    Q_OBJECT

public:
    CvCubRot(QWidget *parent = nullptr) : QWidget(parent), alpha(0) {
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &CvCubRot::rotateCube);
        timer->start(20);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.fillRect(rect(), Qt::white);

        QImage image(size(), QImage::Format_RGB32);
        QPainter imgPainter(&image);
        imgPainter.fillRect(image.rect(), Qt::white);

        QSize dim = size();
        int maxX = dim.width() - 1, maxY = dim.height() - 1;
        centerX = maxX / 2;
        centerY = maxY / 2;
        int minMaxXY = std::min(maxX, maxY);
        obj.d = obj.rho * minMaxXY / obj.objSize;
        obj.rotateCube(alpha);
        obj.eyeAndScreen();

        imgPainter.setPen(Qt::blue);

        for (int i = 0; i < 8; ++i) {
            for (int j = i + 1; j < 8; ++j) {
                line(imgPainter, i, j);
            }
        }

        painter.drawImage(0, 0, image);
    }

private slots:
    void rotateCube() {
        alpha += 0.01;
        repaint();
    }

private:
    int centerX, centerY;
    Obj5 obj;
    double alpha;

    void line(QPainter& painter, int i, int j) {
        const std::vector<std::pair<int, int>> edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Bottom face
            {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Top face
            {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Connecting edges
        };

        for (auto& edge : edges) {
            if ((i == edge.first && j == edge.second) || (i == edge.second && j == edge.first)) {
                Point2D* P = obj.vScr[i];
                Point2D* Q = obj.vScr[j];
                painter.drawLine(iX(P->x), iY(P->y), iX(Q->x), iY(Q->y));
            }
        }
    }

    int iX(float x) {
        return std::lround(centerX + x);
    }

    int iY(float y) {
        return std::lround(centerY - y);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), cvCubRot(new CvCubRot(this))
{
    ui->setupUi(this);
    setCentralWidget(cvCubRot);

    // Set the window title
    setWindowTitle("RotatingCube_QT");

    // Set the window icon
    QIcon icon(":/cplusplus.png");
    setWindowIcon(icon);

    // Get screen size and set window size to half
    QSize screenSize = QGuiApplication::primaryScreen()->size();
    int width = screenSize.width() / 2;
    int height = screenSize.height() / 2;

    // Set the size and position of the window
    setGeometry((screenSize.width() - width) / 4, (screenSize.height() - height) / 4, width, height);

    // Set the initial size of the window
    resize(width, height);
}

MainWindow::~MainWindow() {
    delete ui;
}



#include "mainwindow.moc"//This library is necesary to execute!!!!
