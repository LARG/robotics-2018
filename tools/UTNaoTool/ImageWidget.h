#ifndef IMAGE_WIDGET_H
#define IMAGE_WIDGET_H

#include <QtGui>
#include <QWidget>
#include <QImage>

#include <vector>
#include <math.h>
#include <iostream>

#include <common/RobotInfo.h>

#include <common/annotations/SelectionType.h>
#include <common/annotations/SelectionMode.h>
#include <common/annotations/EllipseSelection.h>
#include <common/annotations/ManualSelection.h>
#include <common/annotations/PolygonSelection.h>
#include <common/annotations/RectangleSelection.h>
#include <common/annotations/Selection.h>

#include <opencv2/core/core.hpp>

class QPaintEvent;

class ImageWidget : public QWidget {
  Q_OBJECT
    Q_PROPERTY(bool selectionEnabled READ getSelectionEnabled WRITE setSelectionEnabled)

  public:
    using Polygon = PolygonSelection;
    using Rectangle = RectangleSelection;
    using Ellipse = EllipseSelection;
    using Manual = ManualSelection;
    ImageWidget(QWidget *parent);
    void setImageSize(int width, int height, QImage::Format format = QImage::Format_RGB32);
    inline void setPixel(int x, int y, QRgb value) {
      img_->setPixel(x, y, value);
    }
    inline QRgb getPixel(int x, int y) {
      return img_->pixel(x, y);
    }
    inline void fill(unsigned char value) {
      img_->fill(value);
    }
    inline void save(const QString& path, const char* fmt, int v) {
      img_->save(path, fmt, v);
    }
    inline QImage* getImage() { return img_.get(); }

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void setSelectionType(SelectionType);
    bool getSelectionEnabled();


    public slots:
      void selectionTypeChanged(SelectionType);
    void setSelectionEnabled(bool);
    void setCurrentSelections(std::vector<Selection*>);
    void setImageSource(const QImage* image);
    void setImageSource(const cv::Mat& image, int format = CV_8UC1);

signals:
    void clicked(int x, int y, Qt::MouseButton button);
    void dragged(int x, int y, Qt::MouseButton button);
    void moved(int x, int y);
    void hovered(int x, int y);
    void selected(Selection*);

  private:
    std::unique_ptr<QImage> img_;
    int width_, height_;
    bool isDragging_, isSelecting_, isSelectionEnabled_;
    SelectionType selectionType_;
    SelectionMode selectionMode_;

    void updateSelection(int,int);
    void beginSelection();
    void endSelection();
    void clearSelection();
    void drawSelection();
    void drawSelectionRectangle(bool);
    void drawSelectionPolygon();
    void drawSelectionEllipse();
    void drawStoredSelections();
    void drawPolygon(Polygon*);
    void drawEllipse(Ellipse*);
    void drawRectangle(Rectangle*,bool);

    void zoomIn(int,int,int,int);
    void zoomOut();
    QRect getViewBox();
    QPoint mapPoint(QPoint);
    QPoint unMapPoint(QPoint);

    SelectionMode getMode();
    SelectionType getType();
    int startX_, startY_, currentX_, currentY_;
    std::vector<QRect> zoomstack_;
    std::vector<QPoint> selectionVertices_;
    std::vector<Selection*> selections_;
    QColor selectionColor_, hoverColor_;
};

#endif
