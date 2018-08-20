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
#include <common/annotations/PolygonSelection.h>
#include <common/annotations/RectangleSelection.h>
#include <common/annotations/Selection.h>

#define UTPolygon PolygonSelection
#define UTEllipse EllipseSelection
#define UTRectangle RectangleSelection

class QPaintEvent;

class ImageWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(bool selectionEnabled READ getSelectionEnabled WRITE setSelectionEnabled)

  public:
   ImageWidget(QWidget *parent);
   void setImageSize(int width, int height, QImage::Format format = QImage::Format_RGB32);
   void setImageSource(QImage* image, int width, int height);
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

  signals:
    void clicked(int x, int y, int button);
    void mouseXY(int x, int y);
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
    void drawPolygon(UTPolygon*);
    void drawEllipse(UTEllipse*);
    void drawRectangle(UTRectangle*,bool);

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
    QColor selectionColor_;
};

#endif
