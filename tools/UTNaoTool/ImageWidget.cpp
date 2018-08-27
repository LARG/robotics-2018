#include <tool/ImageWidget.h>
#include <cstdlib>
#include <math/Common.h>

using namespace static_math;

ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent) {
  setMouseTracking(true);
  isDragging_ = false;
  isSelecting_ = false;
  isSelectionEnabled_ = false;
  selectionType_ = SelectionType::Rectangle;
  selectionColor_ = Qt::cyan;
  hoverColor_ = Qt::yellow;
}

void ImageWidget::setImageSize(int width, int height, QImage::Format format){
  if(width_ != width || height_ != height) {
    width_ = width;
    height_ = height;
    img_ = std::make_unique<QImage>(width, height, format);
    img_->fill(Qt::black);
  }
}

void ImageWidget::setImageSource(const cv::Mat& image, int format) {
  switch(format) {
    case CV_8UC1:
      setImageSize(image.cols, image.rows);
      for(int j = 0; j < image.rows; j++)
        for(int i = 0; i < image.cols; i++) {
          unsigned char value = image.at<unsigned char>(j, i);
          setPixel(i, j, qRgb(value, value, value));
        }
      break;
    case CV_8UC3:
      setImageSize(image.cols, image.rows);
      for(int j = 0; j < image.rows; j++)
        for(int i = 0; i < image.cols; i++) {
          cv::Vec3b value = image.at<cv::Vec3b>(j, i);
          setPixel(i, j, qRgb(value[0], value[1], value[2]));
        }
      break;
  }
  update();
}

void ImageWidget::setImageSource(const QImage* image) {
  if(image == nullptr) {
    setImageSize(1280,960);
    img_->fill(Qt::black);
  } else {
    setImageSize(image->width(), image->height());
    *img_ = *image;
  }
  update();
}

void ImageWidget::paintEvent(QPaintEvent *event) {
  if(img_ == nullptr) return;
  QPainter painter;
  painter.begin(this);

  if(zoomstack_.size() == 0)
    painter.drawImage(event->rect(), *img_);
  else{
    QRect viewbox = getViewBox();
    painter.drawImage(QRect(0,0,width(),height()), *img_, viewbox);
  }
  painter.end();
  if(isSelecting_)
    drawSelection();
  if(isSelectionEnabled_)
    drawStoredSelections();
}

SelectionMode ImageWidget::getMode(){
  int mods = QApplication::keyboardModifiers();
  if(mods == Qt::NoModifier)
    return None;
  else if(mods == (Qt::ControlModifier | Qt::AltModifier)) {
    return Zoom;
  }
  else if(isSelectionEnabled_ && mods == Qt::AltModifier) {
    return Subtract;
  }
  else if(isSelectionEnabled_) {
    return Add;
  }
  return None;
}

SelectionType ImageWidget::getType() {
  if(getMode() == Zoom)
    return SelectionType::Rectangle;
  else
    return selectionType_;
}

/** Mouse Events **/

void ImageWidget::mousePressEvent(QMouseEvent *event) {
  currentX_ = startX_ = event->x();
  currentY_ = startY_ = event->y();
  if(getMode() == None) {
    if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton) return;
    clearSelection();
    QPoint point = mapPoint(QPoint(event->x(),event->y()));
    emit clicked(point.x(), point.y(), event->button());
  }
  else if (getMode() == Zoom){
    if(event->button() == Qt::RightButton){
      zoomOut();
    }
  }
  else if (getMode() == Add && (getType() == SelectionType::Polygon || getType() == SelectionType::Manual)) {
    if (event->button() == Qt::LeftButton && !isSelecting_)
      beginSelection();
    else if (event->button() == Qt::LeftButton && isSelecting_)
      selectionVertices_.push_back(mapPoint(QPoint(startX_, startY_)));
    else if(event->button() == Qt::RightButton)
      endSelection();
  }
  else if (getMode() == Add && getType() == SelectionType::Ellipse){
    if (event->button() == Qt::LeftButton && !isSelecting_)
      beginSelection();
    else if (event->button() == Qt::LeftButton && isSelecting_ && selectionVertices_.size() < 3)
      selectionVertices_.push_back(mapPoint(QPoint(startX_, startY_)));
    if(selectionVertices_.size() == 3)
      endSelection();
  }
  repaint();
}

std::vector<Point> convert(std::vector<QPoint> qpoints) {
  std::vector<Point> points;
  for(auto p : qpoints)
    points.push_back(Point(p.x(), p.y()));
  return points;
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event) {
  QPoint point = mapPoint(QPoint(event->x(),event->y()));
  emit moved(point.x(),point.y());
  if(getMode() == None) {
    if(QApplication::mouseButtons() == Qt::LeftButton) {
      emit dragged(point.x(), point.y(), event->button());
    }
    else
      emit hovered(point.x(), point.y());
  }
  else if(QApplication::mouseButtons() == Qt::LeftButton && (getMode() == Zoom || getMode() == Add) && (getType() == SelectionType::Rectangle || getType() == SelectionType::Square)) {
    isDragging_ = true;
    beginSelection();
    updateSelection(event->x(), event->y());
  }
  else if (getMode() == Add && getType() == SelectionType::Manual) {
    isDragging_ = true;
    QPoint p(event->x(), event->y());
    p = mapPoint(p);
    if(QApplication::mouseButtons() == Qt::LeftButton)
      selectionVertices_.push_back(p);
    updateSelection(event->x(), event->y());
  }
  else if(getMode() == Add && (getType() == SelectionType::Polygon || getType() == SelectionType::Ellipse))
    updateSelection(event->x(), event->y());
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *) {
  if(isDragging_){
    if(getType() != SelectionType::Manual)
      endSelection();
    isDragging_ = false;
    repaint();
    return;
  }
}

/** Selection **/

void ImageWidget::setCurrentSelections(std::vector<Selection*> selections) {
  selections_ = selections;
  repaint();
}

void ImageWidget::beginSelection(){
  isSelecting_ = true;
  switch(getType()) {
    case SelectionType::Ellipse:
    case SelectionType::Polygon:
    case SelectionType::Manual:
      selectionVertices_.clear();
      selectionVertices_.push_back(mapPoint(QPoint(startX_, startY_)));
      break;
    default:
      break;
  }
}

void ImageWidget::updateSelection(int x, int y){
  currentX_ = x;
  currentY_ = y;
  repaint();
}

void ImageWidget::endSelection(){
  isSelecting_ = false;
  if(getMode() == Zoom){
    zoomIn(startX_, startY_, currentX_, currentY_);
    repaint();
  }
  //TODO: Fix these memory leaks
  if(getMode() == Add) {
    switch(getType()) {
      case SelectionType::Rectangle: {
        QPoint sp(startX_, startY_), cp(currentX_, currentY_);
        sp = mapPoint(sp);
        cp = mapPoint(cp);
        Rectangle* rect = new Rectangle(Point(sp.x(), sp.y()), Point(cp.x(), cp.y()));
        emit selected(rect);
      } break;
      case SelectionType::Polygon: {
        Polygon* poly = new Polygon(convert(selectionVertices_));
        emit selected(poly);
      } break;
      case SelectionType::Manual: {
        Polygon* poly = new Manual(convert(selectionVertices_));
        emit selected(poly);
      } break;
      case SelectionType::Ellipse: {
        Ellipse* ellipse = new Ellipse(convert(selectionVertices_));
        ellipse->fixpoints();
        emit selected(ellipse);
      } break;
    }
  }
}

void ImageWidget::clearSelection(){
  isSelecting_ = false;
  isDragging_ = false;
  selectionVertices_.clear();
}

/** Drawing **/

void ImageWidget::drawSelection(){
  switch(getType()){
    case SelectionType::Rectangle:
      drawSelectionRectangle(getMode() != Zoom);
      break;
    case SelectionType::Manual:
    case SelectionType::Polygon:
      drawSelectionPolygon();
      break;
    case SelectionType::Ellipse:
      drawSelectionEllipse();
      break;
    default:
      break;
  }
}

void ImageWidget::drawSelectionRectangle(bool map){
  QPoint topleft(startX_, startY_), bottomright(currentX_, currentY_);
  if(map) {
    topleft = mapPoint(topleft);
    bottomright = mapPoint(bottomright);
  }
  Rectangle rect(Point(topleft.x(), topleft.y()), Point(bottomright.x(), bottomright.y()));
  drawRectangle(&rect, map);
}

void ImageWidget::drawSelectionEllipse(){
  Ellipse ellipse(convert(selectionVertices_));
  if(isSelecting_) {
    QPoint qp(currentX_, currentY_);
    qp = mapPoint(qp);
    Point p(qp.x(), qp.y());
    ellipse.addVertex(p);
  }
  ellipse.fixpoints();
  drawEllipse(&ellipse);
}

void ImageWidget::drawSelectionPolygon(){
  Polygon poly(convert(selectionVertices_));
  if(isSelecting_) {
    QPoint qp(currentX_, currentY_);
    qp = mapPoint(qp);
    Point p(qp.x(), qp.y());
    poly.addVertex(p);
  }
  drawPolygon(&poly);
}

void ImageWidget::drawStoredSelections() {
  for(unsigned int i=0; i<selections_.size(); i++) {
    switch(selections_[i]->getSelectionType()){
      case SelectionType::Rectangle:
        drawRectangle(static_cast<Rectangle*>(selections_[i]), true);
        break;
      case SelectionType::Ellipse:
        drawEllipse(static_cast<Ellipse*>(selections_[i]));
        break;
      case SelectionType::Manual:
      case SelectionType::Polygon:
        drawPolygon(static_cast<Polygon*>(selections_[i]));
        break;
      default:
        break;
    }
  }
}

void ImageWidget::drawRectangle(Rectangle* rectangle, bool map){
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(rectangle->hovered() ? hoverColor_ : selectionColor_);
  painter.setOpacity(.7);
  QPoint topleft = QPoint(rectangle->getTopLeft().x, rectangle->getTopLeft().y);
  QPoint bottomright = QPoint(rectangle->getBottomRight().x, rectangle->getBottomRight().y);
  QRect rect(topleft, bottomright);
  if(map)
    rect = QRect(unMapPoint(topleft), unMapPoint(bottomright));
  else
    rect = QRect(topleft, bottomright);
  painter.drawRect(rect);
  painter.setBrush(rectangle->hovered() ? hoverColor_ : selectionColor_);
  painter.setOpacity(.2);
  painter.drawRect(rect);
  painter.end();
}

void ImageWidget::drawEllipse(Ellipse* ellipse){
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(ellipse->hovered() ? hoverColor_ : selectionColor_);
  QPoint topleft(ellipse->x(), ellipse->y());
  QPoint bottomright(ellipse->x() + ellipse->width(), ellipse->y() + ellipse->height());
  QRect rect(unMapPoint(topleft), unMapPoint(bottomright));
  painter.drawEllipse(rect);
  painter.setBrush(ellipse->hovered() ? hoverColor_ : selectionColor_);
  painter.setOpacity(.2);
  painter.drawEllipse(rect);
  painter.end();
}

void ImageWidget::drawPolygon(Polygon* poly) {
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(poly->hovered() ? hoverColor_ : selectionColor_);
  std::vector<Point> vertices = poly->getVertices();
  int vcount = vertices.size();
  std::vector<QPoint> points(vcount);
  for(int j=0; j<vcount; j++){
    Point p = vertices[j];
    QPoint qp = QPoint(p.x,p.y);
    points[j] = unMapPoint(qp);
  }
  painter.setOpacity(.7);
  painter.drawPolygon(points.data(), vcount);
  painter.setBrush(poly->hovered() ? hoverColor_ : selectionColor_);
  painter.setOpacity(.2);
  painter.drawPolygon(points.data(), vcount);
  painter.end();
}

/** Zooming **/

void ImageWidget::zoomIn(int startx, int starty, int endx, int endy){
  QRect zoom(std::min(startx,endx), std::min(starty,endy), std::abs(startx - endx), std::abs(starty - endy));
  zoomstack_.push_back(zoom);
}

void ImageWidget::zoomOut(){
  if(zoomstack_.size() > 0)
    zoomstack_.pop_back();
}

QRect ImageWidget::getViewBox(){
  QRect viewbox(0,0,width_,height_);
  for(unsigned int i=0; i<zoomstack_.size();i++) {
    QRect zoom = zoomstack_[i];
    float currentWidth = viewbox.width(), currentHeight = viewbox.height();
    viewbox.setLeft(viewbox.left() + zoom.left() * currentWidth / width());
    viewbox.setRight( viewbox.right() - (width() - zoom.right()) * currentWidth / width() );
    viewbox.setTop(viewbox.top() + zoom.top() * currentHeight / height());
    viewbox.setBottom(viewbox.bottom() - (height() - zoom.bottom()) * currentHeight / height());
  }
  return viewbox;
}

// Maps a point in the current frame to the global coordinates
QPoint ImageWidget::mapPoint(QPoint point) {
  QRect viewbox = getViewBox();
  float xpct = (float)point.x() / width(), ypct = (float)point.y() / height();
  point.setX(xpct * viewbox.width() + viewbox.left());
  point.setY(ypct * viewbox.height() + viewbox.top());
  return point;
}

// Maps a point in the global frame to the zoomed coordinates
QPoint ImageWidget::unMapPoint(QPoint point) {
  QRect viewbox = getViewBox();
  float xpct = ((float)point.x() - viewbox.left()) / viewbox.width(), ypct = ((float)point.y() - viewbox.top()) / viewbox.height();
  point.setX(xpct * width());
  point.setY(ypct * height());
  return point;
}

void ImageWidget::setSelectionEnabled(bool value){
  isSelectionEnabled_ = value;
  repaint();
}

void ImageWidget::selectionTypeChanged(SelectionType type) {
  selectionType_ = type;
}

bool ImageWidget::getSelectionEnabled() {
  return isSelectionEnabled_;
}
